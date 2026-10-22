#include "collect-menumodel.h"
#include "mnemonic.h"
#include <gio/gio.h>
#include <glib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>

#define CALL_TIMEOUT_MS 3000

/* Read a UTF-8 string X11 property from a window. Returns owned string or NULL. */
static char *read_utf8_prop(Display *dpy, Window xid, const char *atom_name)
{
    Atom prop = XInternAtom(dpy, atom_name, True);
    if (prop == None) return NULL;

    Atom actual_type;
    int  actual_fmt;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;

    if (XGetWindowProperty(dpy, xid, prop, 0, 4096, False,
                           AnyPropertyType, &actual_type, &actual_fmt,
                           &nitems, &bytes_after, &data) != Success)
        return NULL;

    if (!data || nitems == 0) {
        if (data) XFree(data);
        return NULL;
    }

    char *result = g_strndup((const char *)data, (gsize)nitems);
    XFree(data);
    return result;
}

/*
 * Format a GMenuModel "accel" attribute string (e.g. "<Primary><Shift>s") into
 * display form "Ctrl+Shift+S".  Returns owned string or NULL.
 */
static char *format_accel(const char *accel)
{
    if (!accel || *accel == '\0') return NULL;

    guint key;
    GdkModifierType mods;
    gtk_accelerator_parse(accel, &key, &mods);
    if (!key && !mods) return NULL;

    char *label = gtk_accelerator_get_label(key, mods);
    return label; /* already owned */
}

/* Context carried through recursive menu traversal */
typedef struct {
    GDBusConnection *conn;
    const char      *bus_name;
    const char      *menubar_path;
    const char      *app_action_path;
    const char      *win_action_path;
    const char      *unity_action_path;
    GPtrArray       *entries;
} TraverseCtx;

/* Get the action group D-Bus path for a given action name prefix. */
static const char *action_path_for_prefix(TraverseCtx *ctx, const char *action_name)
{
    if (!action_name) return NULL;
    if (g_str_has_prefix(action_name, "app.") && ctx->app_action_path)
        return ctx->app_action_path;
    if (g_str_has_prefix(action_name, "win.") && ctx->win_action_path)
        return ctx->win_action_path;
    if (g_str_has_prefix(action_name, "unity.") && ctx->unity_action_path)
        return ctx->unity_action_path;
    /* Default: try app path */
    return ctx->app_action_path;
}

/* Strip the "app.", "win.", "unity." prefix from an action name. */
static const char *strip_action_prefix(const char *action_name)
{
    if (!action_name) return action_name;
    const char *dot = strchr(action_name, '.');
    return dot ? dot + 1 : action_name;
}

/* Forward declaration */
static void traverse_section(TraverseCtx *ctx,
                              GVariant *section_ref,
                              GPtrArray *path_segs,
                              GHashTable *started_subs);

static void traverse_menu(TraverseCtx *ctx,
                           const char *menu_path,
                           guint menu_id,
                           GPtrArray *path_segs,
                           GHashTable *started_subs);

static void traverse_menu(TraverseCtx *ctx,
                           const char *menu_path,
                           guint menu_id,
                           GPtrArray *path_segs,
                           GHashTable *started_subs)
{
    /* Avoid infinite recursion */
    char *key = g_strdup_printf("%s:%u", menu_path, menu_id);
    if (g_hash_table_contains(started_subs, key)) {
        g_free(key);
        return;
    }
    g_hash_table_insert(started_subs, key, GUINT_TO_POINTER(1));

    g_autoptr(GError) err = NULL;
    GDBusProxy *proxy = g_dbus_proxy_new_sync(ctx->conn,
        G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
        G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
        NULL, ctx->bus_name, menu_path, "org.gtk.Menus",
        NULL, &err);
    if (!proxy) return;

    GVariantBuilder start_b;
    g_variant_builder_init(&start_b, G_VARIANT_TYPE("au"));
    g_variant_builder_add(&start_b, "u", menu_id);
    GVariant *start_reply = g_dbus_proxy_call_sync(proxy, "Start",
        g_variant_new("(au)", &start_b),
        G_DBUS_CALL_FLAGS_NONE, CALL_TIMEOUT_MS, NULL, &err);
    if (!start_reply) {
        g_object_unref(proxy);
        return;
    }
    if (!g_variant_is_of_type(start_reply, G_VARIANT_TYPE("(a(uuaa{sv}))"))) {
        g_variant_unref(start_reply);
        g_object_unref(proxy);
        return;
    }

    GVariant *content;
    g_variant_get(start_reply, "(@a(uuaa{sv}))", &content);
    g_variant_unref(start_reply);

    GVariantIter outer;
    g_variant_iter_init(&outer, content);
    guint32 sub_id, section_id;
    GVariant *items;

    while (g_variant_iter_next(&outer, "(uu@aa{sv})", &sub_id, &section_id, &items)) {
        if (sub_id != menu_id) {
            g_variant_unref(items);
            continue;
        }

        GVariantIter item_iter;
        g_variant_iter_init(&item_iter, items);
        GVariant *item;

        while ((item = g_variant_iter_next_value(&item_iter))) {
            /* Label: present → this is a named entry (leaf or submenu) */
            GVariant *label_v = g_variant_lookup_value(item, "label", G_VARIANT_TYPE_STRING);
            GVariant *action_v = g_variant_lookup_value(item, "action", G_VARIANT_TYPE_STRING);
            GVariant *submenu_v = g_variant_lookup_value(item, ":submenu", G_VARIANT_TYPE("(uu)"));
            GVariant *section_v = g_variant_lookup_value(item, ":section", G_VARIANT_TYPE("(uu)"));
            GVariant *accel_v   = g_variant_lookup_value(item, "accel", G_VARIANT_TYPE_STRING);
            GVariant *target_v  = g_variant_lookup_value(item, "target", NULL);

            const char *label  = label_v  ? g_variant_get_string(label_v, NULL)  : NULL;
            const char *action = action_v ? g_variant_get_string(action_v, NULL) : NULL;

            if (section_v) {
                traverse_section(ctx, section_v, path_segs, started_subs);
            } else if (submenu_v && label) {
                guint32     sub_menu_id;
                guint32     unused_id;
                g_variant_get(submenu_v, "(uu)", &sub_menu_id, &unused_id);

                g_ptr_array_add(path_segs, (char *)label);
                traverse_menu(ctx, ctx->menubar_path, sub_menu_id, path_segs, started_subs);
                g_ptr_array_remove_index(path_segs, path_segs->len - 1);
            } else if (action && label) {
                /* Leaf item */
                char *shortcut = accel_v
                    ? format_accel(g_variant_get_string(accel_v, NULL))
                    : NULL;

                g_ptr_array_add(path_segs, (char *)label);
                int n = (int)path_segs->len;
                char *markup = mnemonic_build_path_markup((const char **)path_segs->pdata, n);
                char *plain  = mnemonic_build_path_plain((const char **)path_segs->pdata, n);
                g_ptr_array_remove_index(path_segs, path_segs->len - 1);

                const char *apath = action_path_for_prefix(ctx, action);
                const char *bare_action = strip_action_prefix(action);

                MenuEntry *e = g_new0(MenuEntry, 1);
                e->path_markup      = markup;
                e->path_plain       = plain;
                e->shortcut         = shortcut;
                e->backend          = BACKEND_MENUMODEL;
                e->action_bus_name  = g_strdup(ctx->bus_name);
                e->action_group_path = g_strdup(apath ? apath : "");
                e->action_name      = g_strdup(bare_action);
                e->action_target    = target_v;
                target_v            = NULL;
                e->enabled          = TRUE;
                e->use_count        = 0;
                g_ptr_array_add(ctx->entries, e);
            }

            if (label_v)   g_variant_unref(label_v);
            if (action_v)  g_variant_unref(action_v);
            if (submenu_v) g_variant_unref(submenu_v);
            if (section_v) g_variant_unref(section_v);
            if (accel_v)   g_variant_unref(accel_v);
            if (target_v)  g_variant_unref(target_v);
            g_variant_unref(item);
        }

        g_variant_unref(items);
    }

    g_variant_unref(content);

    /* End subscription */
    g_dbus_proxy_call_sync(proxy, "End",
        g_variant_new("(u)", menu_id),
        G_DBUS_CALL_FLAGS_NONE, CALL_TIMEOUT_MS, NULL, NULL);

    g_object_unref(proxy);
}

static void traverse_section(TraverseCtx *ctx,
                              GVariant *section_ref,
                              GPtrArray *path_segs,
                              GHashTable *started_subs)
{
    guint32     menu_id;
    guint32     unused_id;
    g_variant_get(section_ref, "(uu)", &menu_id, &unused_id);
    traverse_menu(ctx, ctx->menubar_path, menu_id, path_segs, started_subs);
}

GPtrArray *collect_menumodel(gulong xid)
{
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return NULL;

    char *bus_name    = read_utf8_prop(dpy, (Window)xid, "_GTK_UNIQUE_BUS_NAME");
    char *menubar_path = read_utf8_prop(dpy, (Window)xid, "_GTK_MENUBAR_OBJECT_PATH");
    char *app_path    = read_utf8_prop(dpy, (Window)xid, "_GTK_APPLICATION_OBJECT_PATH");
    char *win_path    = read_utf8_prop(dpy, (Window)xid, "_GTK_WINDOW_OBJECT_PATH");
    char *unity_path  = read_utf8_prop(dpy, (Window)xid, "_UNITY_OBJECT_PATH");
    XCloseDisplay(dpy);

    if (!bus_name || !menubar_path) {
        g_free(bus_name); g_free(menubar_path);
        g_free(app_path); g_free(win_path); g_free(unity_path);
        return NULL;
    }

    g_autoptr(GError) err = NULL;
    GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &err);
    if (!conn) {
        g_free(bus_name); g_free(menubar_path);
        g_free(app_path); g_free(win_path); g_free(unity_path);
        return NULL;
    }

    GPtrArray *entries = g_ptr_array_new_with_free_func((GDestroyNotify)menu_entry_free);
    GPtrArray *path_segs = g_ptr_array_new();
    GHashTable *started_subs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    TraverseCtx ctx = {
        .conn              = conn,
        .bus_name          = bus_name,
        .menubar_path      = menubar_path,
        .app_action_path   = app_path,
        .win_action_path   = win_path,
        .unity_action_path = unity_path,
        .entries           = entries,
    };

    traverse_menu(&ctx, menubar_path, 0, path_segs, started_subs);

    g_hash_table_destroy(started_subs);
    g_ptr_array_free(path_segs, TRUE);
    g_object_unref(conn);
    g_free(bus_name); g_free(menubar_path);
    g_free(app_path); g_free(win_path); g_free(unity_path);

    return entries;
}
