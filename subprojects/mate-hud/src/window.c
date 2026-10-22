#include "window.h"
#include "model.h"
#include "history.h"
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <glib/gi18n.h>
#include <string.h>

typedef enum {
    HUD_ANCHOR_START,
    HUD_ANCHOR_CENTER,
    HUD_ANCHOR_END
} HudAnchor;

static GdkRectangle applet_geometry = {0};

struct HudWindow {
    GtkWidget          *window;
    GtkWidget          *search_entry;
    GtkWidget          *tree_view;
    GtkListStore       *store;
    GtkTreeModelFilter *filter;
    GPtrArray          *entries;
    char               *wm_class;
    char               *attach_to;
    char               *location;
    int                 history_max;
    gulong              target_xid;
    guint32             dismiss_time;
    int                 placed_width;
    int                 placed_height;
    gboolean            alt_tap;
    gboolean            has_focus;
    guint               focus_watchdog_id;
};

/* Every way of closing the HUD needs keyboard focus, so a HUD the window
 * manager never focuses (e.g. focus-stealing prevention) would stay forever. */
#define FOCUS_WATCHDOG_MS 1000

/* ── Placement ───────────────────────────────────────────────────────────── */

static gboolean location_is_valid(const char *location)
{
    static const char *valid[] = {
        "northwest", "north", "northeast",
        "west", "center", "east",
        "southwest", "south", "southeast",
        NULL
    };

    for (guint i = 0; valid[i]; i++) {
        if (g_strcmp0(location, valid[i]) == 0)
            return TRUE;
    }
    return FALSE;
}

static const char *normalized_attach_to(const char *attach_to)
{
    if (g_strcmp0(attach_to, "monitor") == 0)
        return "monitor";
    if (g_strcmp0(attach_to, "applet") == 0)
        return "applet";
    return "window";
}

static const char *normalized_location(const char *location)
{
    if (location_is_valid(location))
        return location;
    return "northwest";
}

static void set_placement(HudWindow *w, const char *attach_to, const char *location)
{
    g_free(w->attach_to);
    g_free(w->location);
    w->attach_to = g_strdup(normalized_attach_to(attach_to));
    w->location = g_strdup(normalized_location(location));
}

static void parse_location(const char *location, HudAnchor *x_anchor, HudAnchor *y_anchor)
{
    const char *loc = normalized_location(location);

    if (strstr(loc, "west"))
        *x_anchor = HUD_ANCHOR_START;
    else if (strstr(loc, "east"))
        *x_anchor = HUD_ANCHOR_END;
    else
        *x_anchor = HUD_ANCHOR_CENTER;

    if (strstr(loc, "north"))
        *y_anchor = HUD_ANCHOR_START;
    else if (strstr(loc, "south"))
        *y_anchor = HUD_ANCHOR_END;
    else
        *y_anchor = HUD_ANCHOR_CENTER;
}

static gboolean get_target_rect(gulong target_xid, GdkRectangle *rect)
{
    if (!target_xid)
        return FALSE;

    GdkDisplay *display = gdk_display_get_default();
    if (!GDK_IS_X11_DISPLAY(display))
        return FALSE;

    GdkWindow *foreign =
        gdk_x11_window_foreign_new_for_display(display, (Window)target_xid);
    if (!foreign)
        return FALSE;

    gdk_window_get_frame_extents(foreign, rect);
    g_object_unref(foreign);

    return rect->width > 0 && rect->height > 0;
}

static gboolean get_monitor_workarea_for_rect(const GdkRectangle *target,
                                               GdkRectangle       *workarea)
{
    GdkDisplay *display = gdk_display_get_default();
    if (!display)
        return FALSE;

    GdkMonitor *monitor = NULL;
    if (target && target->width > 0 && target->height > 0) {
        int x = target->x + target->width / 2;
        int y = target->y + target->height / 2;
        monitor = gdk_display_get_monitor_at_point(display, x, y);
    }

    if (!monitor)
        monitor = gdk_display_get_primary_monitor(display);
    if (!monitor && gdk_display_get_n_monitors(display) > 0)
        monitor = gdk_display_get_monitor(display, 0);
    if (!monitor)
        return FALSE;

    gdk_monitor_get_workarea(monitor, workarea);
    return workarea->width > 0 && workarea->height > 0;
}

static int anchored_coord(int origin, int span, int window_span, HudAnchor anchor)
{
    const int margin = 12;

    switch (anchor) {
    case HUD_ANCHOR_START:
        return origin + margin;
    case HUD_ANCHOR_END:
        return origin + span - window_span - margin;
    case HUD_ANCHOR_CENTER:
    default:
        return origin + (span - window_span) / 2;
    }
}

static int clamp_dropdown_coord(int value, int origin, int span, int window_span)
{
    if (window_span >= span)
        return origin;
    return CLAMP(value, origin, origin + span - window_span);
}

static gboolean place_at_applet(HudWindow *w, int win_w, int win_h)
{
    if (applet_geometry.width <= 0 || applet_geometry.height <= 0)
        return FALSE;

    GdkRectangle workarea = {0};
    if (!get_monitor_workarea_for_rect(&applet_geometry, &workarea))
        return FALSE;

    int top_distance = ABS(applet_geometry.y + applet_geometry.height - workarea.y);
    int bottom_distance = ABS(applet_geometry.y - (workarea.y + workarea.height));
    int left_distance = ABS(applet_geometry.x + applet_geometry.width - workarea.x);
    int right_distance = ABS(applet_geometry.x - (workarea.x + workarea.width));
    int edge_distance = MIN(MIN(top_distance, bottom_distance),
                            MIN(left_distance, right_distance));
    int x;
    int y;

    if (edge_distance == top_distance) {
        x = clamp_dropdown_coord(applet_geometry.x, workarea.x,
                                 workarea.width, win_w);
        y = applet_geometry.y + applet_geometry.height;
    } else if (edge_distance == bottom_distance) {
        x = clamp_dropdown_coord(applet_geometry.x, workarea.x,
                                 workarea.width, win_w);
        y = applet_geometry.y - win_h;
    } else if (edge_distance == left_distance) {
        x = applet_geometry.x + applet_geometry.width;
        y = clamp_dropdown_coord(applet_geometry.y, workarea.y,
                                 workarea.height, win_h);
    } else {
        x = applet_geometry.x - win_w;
        y = clamp_dropdown_coord(applet_geometry.y, workarea.y,
                                 workarea.height, win_h);
    }

    gtk_window_move(GTK_WINDOW(w->window), x, y);
    return TRUE;
}

static void place_window(HudWindow *w, gulong target_xid)
{
    if (!w->window)
        return;

    GdkRectangle target = {0};
    GdkRectangle area = {0};
    gboolean have_target = get_target_rect(target_xid, &target);

    if (g_strcmp0(normalized_attach_to(w->attach_to), "window") == 0 && have_target) {
        area = target;
    } else if (!get_monitor_workarea_for_rect(have_target ? &target : NULL, &area)) {
        return;
    }

    int win_w = 0;
    int win_h = 0;
    gtk_window_get_size(GTK_WINDOW(w->window), &win_w, &win_h);
    if (win_w <= 0 || win_h <= 0)
        return;

    if (g_strcmp0(normalized_attach_to(w->attach_to), "applet") == 0 &&
        place_at_applet(w, win_w, win_h))
        return;

    HudAnchor x_anchor;
    HudAnchor y_anchor;
    parse_location(w->location, &x_anchor, &y_anchor);

    int x = anchored_coord(area.x, area.width, win_w, x_anchor);
    int y = anchored_coord(area.y, area.height, win_h, y_anchor);
    gtk_window_move(GTK_WINDOW(w->window), x, y);
}

void hud_window_set_applet_geometry(int x, int y, int width, int height)
{
    applet_geometry.x = x;
    applet_geometry.y = y;
    applet_geometry.width = width;
    applet_geometry.height = height;
}

static void on_size_allocate(GtkWidget *widget G_GNUC_UNUSED,
                             GtkAllocation *allocation, gpointer user_data)
{
    HudWindow *w = user_data;
    if (!w->target_xid ||
        (allocation->width == w->placed_width &&
         allocation->height == w->placed_height))
        return;
    w->placed_width = allocation->width;
    w->placed_height = allocation->height;
    place_window(w, w->target_xid);
}

/* ── Activation ──────────────────────────────────────────────────────────── */

static void activate_entry(HudWindow *w, MenuEntry *e)
{
    if (!e) return;

    history_record(w->wm_class, e->path_plain, w->history_max);

    g_autoptr(GError) err = NULL;
    g_autoptr(GDBusConnection) conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &err);
    if (!conn) return;

    if (e->backend == BACKEND_DBUSMENU) {
        guint32 ts = (guint32)(g_get_monotonic_time() / 1000);
        GDBusProxy *proxy = g_dbus_proxy_new_sync(conn,
            G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
            G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
            NULL,
            e->dbus_service, e->dbus_object_path,
            "com.canonical.dbusmenu",
            NULL, &err);
        if (proxy) {
            g_dbus_proxy_call_sync(proxy, "Event",
                g_variant_new("(isvu)", (gint32)e->dbus_item_id, "clicked",
                              g_variant_new_int32(0), ts),
                G_DBUS_CALL_FLAGS_NONE, 2000, NULL, NULL);
            g_object_unref(proxy);
        }
    } else {
        /* GMenuModel: org.gtk.Actions.Activate */
        if (!e->action_group_path || !*e->action_group_path) {
            g_warning("mate-hud: missing action group path for '%s'",
                      e->path_plain ? e->path_plain : "(unknown)");
            return;
        }
        GDBusProxy *proxy = g_dbus_proxy_new_sync(conn,
            G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
            G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
            NULL,
            e->action_bus_name, e->action_group_path,
            "org.gtk.Actions",
            NULL, &err);
        if (proxy) {
            GVariantBuilder target_builder;
            g_variant_builder_init(&target_builder, G_VARIANT_TYPE("av"));
            if (e->action_target)
                g_variant_builder_add(&target_builder, "v", e->action_target);

            GVariantBuilder platform_builder;
            g_variant_builder_init(&platform_builder, G_VARIANT_TYPE("a{sv}"));

            g_dbus_proxy_call_sync(proxy, "Activate",
                g_variant_new("(sava{sv})",
                              e->action_name ? e->action_name : "",
                              &target_builder,
                              &platform_builder),
                G_DBUS_CALL_FLAGS_NONE, 2000, NULL, NULL);
            g_object_unref(proxy);
        }
    }
}

static void do_activate_selected(HudWindow *w)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(w->tree_view));
    GtkTreeModel *model;
    GtkTreeIter filter_iter;
    if (!gtk_tree_selection_get_selected(sel, &model, &filter_iter)) return;

    /* Map filter iter back to store iter */
    GtkTreeIter store_iter;
    gtk_tree_model_filter_convert_iter_to_child_iter(w->filter, &store_iter, &filter_iter);

    guint idx;
    gtk_tree_model_get(GTK_TREE_MODEL(w->store), &store_iter, COL_ENTRY_IDX, &idx, -1);
    if (idx >= w->entries->len) return;

    MenuEntry *e = g_ptr_array_index(w->entries, idx);
    activate_entry(w, e);
    gtk_widget_destroy(w->window);
}

/* ── Navigation helpers ───────────────────────────────────────────────────── */

static void move_selection(HudWindow *w, int delta)
{
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(w->tree_view));
    GtkTreeModel *filter_model = GTK_TREE_MODEL(w->filter);
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(sel, NULL, &iter)) {
        /* Nothing selected: select first or last */
        if (delta > 0) {
            if (gtk_tree_model_get_iter_first(filter_model, &iter))
                gtk_tree_selection_select_iter(sel, &iter);
        }
        return;
    }

    if (delta > 0) {
        GtkTreeIter next = iter;
        if (gtk_tree_model_iter_next(filter_model, &next))
            gtk_tree_selection_select_iter(sel, &next);
    } else {
        GtkTreePath *path = gtk_tree_model_get_path(filter_model, &iter);
        if (gtk_tree_path_prev(path)) {
            gtk_tree_model_get_iter(filter_model, &iter, path);
            gtk_tree_selection_select_iter(sel, &iter);
        }
        gtk_tree_path_free(path);
    }

    /* Scroll selected row into view */
    GtkTreeIter cur;
    if (gtk_tree_selection_get_selected(sel, NULL, &cur)) {
        GtkTreePath *p = gtk_tree_model_get_path(filter_model, &cur);
        gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(w->tree_view), p, NULL, FALSE, 0, 0);
        gtk_tree_path_free(p);
    }
}

static void select_by_alt_index(HudWindow *w, int n)
{
    /* n is 0-based (Alt+1 → 0, Alt+0 → 9) */
    GtkTreeModel *filter_model = GTK_TREE_MODEL(w->filter);
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter_first(filter_model, &iter)) return;

    for (int i = 0; i < n; i++) {
        if (!gtk_tree_model_iter_next(filter_model, &iter)) return;
    }

    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(w->tree_view));
    gtk_tree_selection_select_iter(sel, &iter);
    GtkTreePath *p = gtk_tree_model_get_path(filter_model, &iter);
    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(w->tree_view), p, NULL, FALSE, 0, 0);
    gtk_tree_path_free(p);
}

/* ── Signal handlers ─────────────────────────────────────────────────────── */

static void on_search_changed(GtkSearchEntry *entry, gpointer user_data)
{
    HudWindow *w = user_data;
    const char *query = gtk_entry_get_text(GTK_ENTRY(entry));
    model_refilter(w->store, w->filter, w->entries, query);

    /* Auto-select first result */
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(w->filter), &iter)) {
        GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(w->tree_view));
        gtk_tree_selection_select_iter(sel, &iter);
    }
}

static void on_row_activated(GtkTreeView *tv G_GNUC_UNUSED,
                              GtkTreePath *path G_GNUC_UNUSED,
                              GtkTreeViewColumn *col G_GNUC_UNUSED,
                              gpointer user_data)
{
    do_activate_selected(user_data);
}

static gboolean on_key_press(GtkWidget *widget G_GNUC_UNUSED,
                              GdkEventKey *event,
                              gpointer user_data)
{
    HudWindow *w = user_data;
    guint kv = event->keyval;
    GdkModifierType state = event->state & gtk_accelerator_get_default_mod_mask();

    if (kv == GDK_KEY_Alt_L || kv == GDK_KEY_Alt_R) {
        w->alt_tap = TRUE;
        return TRUE;
    }
    w->alt_tap = FALSE;

    if (kv == GDK_KEY_Escape) {
        gtk_widget_destroy(w->window);
        return TRUE;
    }
    if (kv == GDK_KEY_Return || kv == GDK_KEY_KP_Enter) {
        do_activate_selected(w);
        return TRUE;
    }
    if (kv == GDK_KEY_Up) {
        move_selection(w, -1);
        return TRUE;
    }
    if (kv == GDK_KEY_Down) {
        move_selection(w, +1);
        return TRUE;
    }

    /* Alt+1..9 → select 0-indexed row 0..8; Alt+0 → row 9 */
    if (state == GDK_MOD1_MASK) {
        if (kv >= GDK_KEY_1 && kv <= GDK_KEY_9) {
            select_by_alt_index(w, (int)(kv - GDK_KEY_1));
            return TRUE;
        }
        if (kv == GDK_KEY_0) {
            select_by_alt_index(w, 9);
            return TRUE;
        }
    }

    /* Forward other printable key presses to search entry */
    if (!gtk_widget_has_focus(w->search_entry)) {
        gtk_widget_grab_focus(w->search_entry);
        /* Let the event propagate so the character lands in the entry */
    }
    return FALSE;
}

static gboolean on_key_release(GtkWidget *widget G_GNUC_UNUSED,
                                GdkEventKey *event,
                                gpointer user_data)
{
    HudWindow *w = user_data;

    if ((event->keyval == GDK_KEY_Alt_L || event->keyval == GDK_KEY_Alt_R) &&
        w->alt_tap) {
        w->alt_tap = FALSE;
        gtk_widget_destroy(w->window);
        return TRUE;
    }

    return FALSE;
}

static gboolean on_window_focus_out(GtkWidget *widget,
                                     GdkEventFocus *event G_GNUC_UNUSED,
                                     gpointer user_data)
{
    HudWindow *w = user_data;
    /* Focus events carry no timestamp; record server time so a shortcut
     * tap that caused this dismissal can be told apart from a new one. */
    GdkWindow *gdk_win = gtk_widget_get_window(widget);
    if (GDK_IS_X11_WINDOW(gdk_win))
        w->dismiss_time = gdk_x11_get_server_time(gdk_win);
    /* Watch the toplevel, not the search entry or result list. */
    gtk_widget_destroy(widget);
    return FALSE;
}

static void stop_focus_watchdog(HudWindow *w)
{
    if (w->focus_watchdog_id != 0) {
        g_source_remove(w->focus_watchdog_id);
        w->focus_watchdog_id = 0;
    }
}

static gboolean on_focus_watchdog(gpointer user_data)
{
    HudWindow *w = user_data;
    w->focus_watchdog_id = 0;
    if (w->window && !w->has_focus) {
        g_message("mate-hud: closing HUD that never received keyboard focus");
        gtk_widget_destroy(w->window);
    }
    return G_SOURCE_REMOVE;
}

static gboolean on_window_focus_in(GtkWidget *widget G_GNUC_UNUSED,
                                    GdkEventFocus *event G_GNUC_UNUSED,
                                    gpointer user_data)
{
    HudWindow *w = user_data;
    w->has_focus = TRUE;
    stop_focus_watchdog(w);
    return FALSE;
}

static void on_window_destroy(GtkWidget *widget G_GNUC_UNUSED, gpointer user_data)
{
    HudWindow *w = user_data;
    stop_focus_watchdog(w);
    /* Already being destroyed, children included. The HudWindow outlives them
     * until the nested gtk_main() returns, so drop every widget pointer. */
    w->window = NULL;
    w->search_entry = NULL;
    w->tree_view = NULL;
    gtk_main_quit();
}

static void select_first_row(HudWindow *w)
{
    GtkTreeIter first;
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(w->filter), &first))
        gtk_tree_selection_select_iter(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(w->tree_view)), &first);
}

/* ── Construction ────────────────────────────────────────────────────────── */

HudWindow *hud_window_new(GPtrArray *entries,
                          const char *wm_class,
                          int history_max,
                          const char *attach_to,
                          const char *location)
{
    HudWindow *w = g_new0(HudWindow, 1);
    w->entries     = entries;
    w->wm_class    = g_strdup(wm_class);
    w->history_max = history_max;
    set_placement(w, attach_to, location);

    /* Top-level window styled like a dialog */
    w->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(w->window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_position(GTK_WINDOW(w->window), GTK_WIN_POS_NONE);
    gtk_window_set_title(GTK_WINDOW(w->window), _("Search for menus"));
    gtk_window_set_icon_name(GTK_WINDOW(w->window), "system-search");
    gtk_window_set_decorated(GTK_WINDOW(w->window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(w->window), FALSE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(w->window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(w->window), 6);
    gtk_widget_set_size_request(w->window, 600, -1);

    /* Vertical box */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(w->window), vbox);

    /* Search entry */
    w->search_entry = gtk_search_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(w->search_entry),
                                   _("Search application menus"));
    gtk_box_pack_start(GTK_BOX(vbox), w->search_entry, FALSE, FALSE, 0);

    /* Separator */
    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 0);

    /* Scrolled window + tree view */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 280);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    /* Model */
    w->store  = model_new_store();
    w->filter = GTK_TREE_MODEL_FILTER(
        gtk_tree_model_filter_new(GTK_TREE_MODEL(w->store), NULL));
    gtk_tree_model_filter_set_visible_column(w->filter, COL_VISIBLE);

    model_populate(w->store, entries);
    model_refilter(w->store, w->filter, entries, "");

    w->tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(w->filter));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(w->tree_view), FALSE);
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(w->tree_view), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll), w->tree_view);

    /* Column 0: menu path (Pango markup, expands) */
    GtkCellRenderer *r0 = gtk_cell_renderer_text_new();
    g_object_set(r0, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    GtkTreeViewColumn *c0 = gtk_tree_view_column_new_with_attributes(
        _("Path"), r0, "markup", COL_PATH_MARKUP, NULL);
    gtk_tree_view_column_set_expand(c0, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(w->tree_view), c0);

    /* Column 1: Alt+N index (fixed width, right-aligned, dimmed) */
    GtkCellRenderer *r1 = gtk_cell_renderer_text_new();
    g_object_set(r1, "xalign", 1.0f,
                 "foreground", "#888888",
                 "scale", 0.85,
                 NULL);
    GtkTreeViewColumn *c1 = gtk_tree_view_column_new_with_attributes(
        _("Index"), r1, "text", COL_ALT_INDEX, NULL);
    gtk_tree_view_column_set_sizing(c1, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(c1, 60);
    gtk_tree_view_append_column(GTK_TREE_VIEW(w->tree_view), c1);

    /* Column 2: keyboard shortcut (fixed width, right-aligned, monospace) */
    GtkCellRenderer *r2 = gtk_cell_renderer_text_new();
    g_object_set(r2, "xalign", 1.0f,
                 "family", "monospace",
                 "scale", 0.85,
                 NULL);
    GtkTreeViewColumn *c2 = gtk_tree_view_column_new_with_attributes(
        _("Shortcut"), r2, "text", COL_SHORTCUT, NULL);
    gtk_tree_view_column_set_sizing(c2, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(c2, 100);
    gtk_tree_view_append_column(GTK_TREE_VIEW(w->tree_view), c2);

    /* Auto-select first row */
    select_first_row(w);

    /* Signals */
    g_signal_connect(w->window, "focus-in-event",
                     G_CALLBACK(on_window_focus_in), w);
    g_signal_connect(w->window, "focus-out-event",
                     G_CALLBACK(on_window_focus_out), w);
    g_signal_connect(w->search_entry, "search-changed",
                     G_CALLBACK(on_search_changed), w);
    g_signal_connect(w->tree_view, "row-activated",
                     G_CALLBACK(on_row_activated), w);
    g_signal_connect(w->window, "key-press-event",
                     G_CALLBACK(on_key_press), w);
    g_signal_connect(w->window, "key-release-event",
                     G_CALLBACK(on_key_release), w);
    g_signal_connect(w->window, "destroy",
                     G_CALLBACK(on_window_destroy), w);
    g_signal_connect(w->window, "size-allocate",
                     G_CALLBACK(on_size_allocate), w);

    gtk_widget_show_all(vbox);
    return w;
}

void hud_window_update(HudWindow *w,
                       GPtrArray *entries,
                       const char *wm_class,
                       int history_max,
                       const char *attach_to,
                       const char *location)
{
    if (!w) return;

    w->entries = entries;
    g_free(w->wm_class);
    w->wm_class = g_strdup(wm_class);
    w->history_max = history_max;
    set_placement(w, attach_to, location);
    if (!w->window)
        return;

    gtk_entry_set_text(GTK_ENTRY(w->search_entry), "");
    model_populate(w->store, entries);
    model_refilter(w->store, w->filter, entries, "");
    select_first_row(w);
    gtk_widget_grab_focus(w->search_entry);
}

void hud_window_set_placement(HudWindow *w,
                              const char *attach_to,
                              const char *location)
{
    if (!w) return;
    set_placement(w, attach_to, location);
    if (w->window && w->target_xid)
        place_window(w, w->target_xid);
}

void hud_window_present(HudWindow *w, guint32 timestamp, gulong target_xid)
{
    if (!w || !w->window)
        return;
    w->target_xid = target_xid;
    /* Grab focus before realize so GTK knows which widget to focus */
    gtk_widget_grab_focus(w->search_entry);
    gtk_widget_realize(w->window);

    /* Set X11 user time so the WM grants focus authority */
    guint32 present_ts = timestamp;
    GdkWindow *gdk_win = gtk_widget_get_window(w->window);
    if (GDK_IS_X11_WINDOW(gdk_win)) {
        /* The parent belongs to another process, so GtkWindow's transient
         * setter cannot be used. Publish WM_TRANSIENT_FOR through GDK before
         * mapping, allowing the WM to restore the application's focus when
         * the HUD closes without stealing focus on click-away dismissal. */
        GdkWindow *parent = target_xid
            ? gdk_x11_window_foreign_new_for_display(
                gdk_window_get_display(gdk_win), (Window)target_xid)
            : NULL;
        if (parent) {
            gdk_window_set_transient_for(gdk_win, parent);
            g_object_unref(parent);
        }
        present_ts = timestamp ? timestamp : gdk_x11_get_server_time(gdk_win);
        gdk_x11_window_set_user_time(gdk_win, present_ts);
    }

    /* Give the WM the requested position before the first map. */
    place_window(w, target_xid);
    gtk_widget_show(w->window);
    place_window(w, target_xid);
    gtk_window_present_with_time(GTK_WINDOW(w->window),
                                 present_ts ? present_ts : GDK_CURRENT_TIME);
    place_window(w, target_xid);
    gtk_widget_grab_focus(w->search_entry);

    /* Each (re-)present gives the WM a fresh chance to focus the HUD. */
    if (!w->has_focus) {
        stop_focus_watchdog(w);
        w->focus_watchdog_id = g_timeout_add(FOCUS_WATCHDOG_MS, on_focus_watchdog, w);
    }
}

void hud_window_destroy(HudWindow *w)
{
    if (!w) return;
    stop_focus_watchdog(w);
    if (w->window && GTK_IS_WIDGET(w->window))
        gtk_widget_destroy(w->window);
    g_object_unref(w->store);
    g_object_unref(w->filter);
    g_free(w->wm_class);
    g_free(w->attach_to);
    g_free(w->location);
    g_free(w);
}

GtkWidget *hud_window_widget(HudWindow *w)
{
    return w ? w->window : NULL;
}

gulong hud_window_xid(HudWindow *w)
{
    if (!w || !w->window || !gtk_widget_get_realized(w->window))
        return 0;

    GdkWindow *gdk_win = gtk_widget_get_window(w->window);
    if (!GDK_IS_X11_WINDOW(gdk_win))
        return 0;

    return (gulong)gdk_x11_window_get_xid(gdk_win);
}

guint32 hud_window_dismiss_time(HudWindow *w)
{
    return w ? w->dismiss_time : 0;
}

gulong hud_active_window_xid(void)
{
    GdkDisplay *display = gdk_display_get_default();
    if (!GDK_IS_X11_DISPLAY(display))
        return 0;

    Display *dpy = GDK_DISPLAY_XDISPLAY(display);
    Atom prop = gdk_x11_get_xatom_by_name_for_display(display, "_NET_ACTIVE_WINDOW");
    Atom type;
    int fmt;
    unsigned long n, after;
    unsigned char *data = NULL;
    gulong xid = 0;

    gdk_x11_display_error_trap_push(display);
    if (XGetWindowProperty(dpy, DefaultRootWindow(dpy), prop, 0, 1, False, XA_WINDOW,
                           &type, &fmt, &n, &after, &data) == Success && data) {
        /* A zero-length property still returns a (1-byte) buffer. */
        if (n >= 1 && fmt == 32)
            xid = *(Window *)data;
        XFree(data);
    }
    gdk_x11_display_error_trap_pop_ignored(display);
    return xid;
}
