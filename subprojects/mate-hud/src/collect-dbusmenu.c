#include "collect-dbusmenu.h"
#include "mnemonic.h"
#include <gio/gio.h>
#include <glib.h>
#include <string.h>

#define DBUSMENU_IFACE   "com.canonical.dbusmenu"
#define CALL_TIMEOUT_MS  3000
#define CALL_TIMEOUT_CHROMIUM_MS 1000
/* Limits on what an application can make us walk: a layout may loop back on
 * itself or keep inventing new submenu ids. */
#define MAX_MENU_DEPTH   16
#define MAX_SUBMENUS     1024

/* Forward */
static void traverse(GDBusProxy *proxy, int item_id,
                     GPtrArray *path_segs, GPtrArray *entries,
                     const char *service, const char *object_path,
                     int call_timeout_ms, GList **level1_ids,
                     GHashTable *visited);

/* Format a DBusMenu shortcut array-of-arrays into "Ctrl+Shift+S" style. */
static char *format_shortcut(GVariant *shortcut_var)
{
    if (!shortcut_var) return NULL;
    /* type is "aas" — array of array of strings */
    if (!g_variant_is_of_type(shortcut_var, G_VARIANT_TYPE("aas"))) return NULL;

    GVariantIter outer;
    g_variant_iter_init(&outer, shortcut_var);
    GVariant *combo;
    GString *result = g_string_new(NULL);

    while ((combo = g_variant_iter_next_value(&outer))) {
        GVariantIter inner;
        g_variant_iter_init(&inner, combo);
        const char *part;
        gboolean first = TRUE;
        while (g_variant_iter_next(&inner, "&s", &part)) {
            if (!first) g_string_append_c(result, '+');
            /* Translate modifier names to common display form */
            if (g_strcmp0(part, "Control") == 0)     g_string_append(result, "Ctrl");
            else if (g_strcmp0(part, "Shift") == 0)  g_string_append(result, "Shift");
            else if (g_strcmp0(part, "Alt") == 0)    g_string_append(result, "Alt");
            else if (g_strcmp0(part, "Super") == 0)  g_string_append(result, "Super");
            else                                     g_string_append(result, part);
            first = FALSE;
        }
        g_variant_unref(combo);
        break; /* only first combo */
    }

    if (result->len == 0) {
        g_string_free(result, TRUE);
        return NULL;
    }
    return g_string_free(result, FALSE);
}

/* Expand a node that has "children-display" by calling AboutToShow + opened. */
static void expand_node(GDBusProxy *proxy, int item_id, int timeout_ms)
{
    guint32 ts = (guint32)(g_get_monotonic_time() / 1000);
    g_autoptr(GError) err = NULL;

    g_dbus_proxy_call_sync(proxy, "AboutToShow",
        g_variant_new("(i)", item_id),
        G_DBUS_CALL_FLAGS_NONE, timeout_ms, NULL, &err);
    g_clear_error(&err);

    g_dbus_proxy_call_sync(proxy, "Event",
        g_variant_new("(isvu)", item_id, "opened",
                      g_variant_new_int32(0), ts),
        G_DBUS_CALL_FLAGS_NONE, timeout_ms, NULL, &err);
    g_clear_error(&err);
}

/* Recursive traversal of one node. path_segs holds label strings for ancestors.
 * visited holds the ids of every submenu already expanded. */
static void traverse(GDBusProxy *proxy, int item_id,
                     GPtrArray *path_segs, GPtrArray *entries,
                     const char *service, const char *object_path,
                     int timeout_ms, GList **level1_ids,
                     GHashTable *visited)
{
    g_autoptr(GError) err = NULL;

    /* GetLayout(item_id, depth=1, props=[...]) */
    static const char * const props[] = {
        "label","children-display","enabled","visible","type","shortcut", NULL
    };
    GVariant *layout_reply = g_dbus_proxy_call_sync(proxy, "GetLayout",
        g_variant_new("(ii^as)", item_id, 1, props),
        G_DBUS_CALL_FLAGS_NONE, timeout_ms, NULL, &err);

    if (!layout_reply) return;
    /* Replies come from the application; never destructure an unchecked type. */
    if (!g_variant_is_of_type(layout_reply, G_VARIANT_TYPE("(u(ia{sv}av))"))) {
        g_variant_unref(layout_reply);
        return;
    }

    guint32 revision;
    GVariant *layout;
    g_variant_get(layout_reply, "(u@(ia{sv}av))", &revision, &layout);
    g_variant_unref(layout_reply);
    if (!layout) return;

    gint32 node_id;
    GVariant *props_var;
    GVariant *children_var;
    g_variant_get(layout, "(i@a{sv}@av)", &node_id, &props_var, &children_var);

    GVariantIter child_iter;
    g_variant_iter_init(&child_iter, children_var);
    GVariant *child_v;

    while ((child_v = g_variant_iter_next_value(&child_iter))) {
        GVariant *child = g_variant_get_variant(child_v);
        g_variant_unref(child_v);
        if (!g_variant_is_of_type(child, G_VARIANT_TYPE("(ia{sv}av)"))) {
            g_variant_unref(child);
            continue;
        }

        gint32 child_id;
        GVariant *child_props;
        GVariant *child_children;
        g_variant_get(child, "(i@a{sv}@av)", &child_id, &child_props, &child_children);

        /* Track level-1 item IDs for the Firefox/VimFx "closed" workaround */
        if (item_id == 0 && level1_ids)
            *level1_ids = g_list_prepend(*level1_ids, GINT_TO_POINTER(child_id));

        /* Skip invisible items */
        GVariant *vis_v = g_variant_lookup_value(child_props, "visible", G_VARIANT_TYPE_BOOLEAN);
        gboolean visible = vis_v ? g_variant_get_boolean(vis_v) : TRUE;
        if (vis_v) g_variant_unref(vis_v);
        if (!visible) {
            g_variant_unref(child_props);
            g_variant_unref(child_children);
            g_variant_unref(child);
            continue;
        }

        /* Skip separators (no label, or type=="separator") */
        GVariant *type_v = g_variant_lookup_value(child_props, "type", G_VARIANT_TYPE_STRING);
        const char *type_str = type_v ? g_variant_get_string(type_v, NULL) : NULL;
        gboolean is_sep = (type_str && g_strcmp0(type_str, "separator") == 0);
        if (type_v) g_variant_unref(type_v);

        GVariant *label_v = g_variant_lookup_value(child_props, "label", G_VARIANT_TYPE_STRING);
        const char *label = label_v ? g_variant_get_string(label_v, NULL) : NULL;
        if (!label || is_sep) {
            if (label_v) g_variant_unref(label_v);
            g_variant_unref(child_props);
            g_variant_unref(child_children);
            g_variant_unref(child);
            continue;
        }

        /* Check enabled state */
        GVariant *ena_v = g_variant_lookup_value(child_props, "enabled", G_VARIANT_TYPE_BOOLEAN);
        gboolean enabled = ena_v ? g_variant_get_boolean(ena_v) : TRUE;
        if (ena_v) g_variant_unref(ena_v);

        /* Does it have children to expand? */
        GVariant *cd_v = g_variant_lookup_value(child_props, "children-display", G_VARIANT_TYPE_STRING);
        gboolean has_children = (cd_v != NULL);
        if (cd_v) g_variant_unref(cd_v);

        /* Add label to path */
        g_ptr_array_add(path_segs, (char *)label);

        if (has_children) {
            /* Expand and recurse, once per submenu and within the limits */
            if (path_segs->len <= MAX_MENU_DEPTH &&
                g_hash_table_size(visited) < MAX_SUBMENUS &&
                g_hash_table_add(visited, GINT_TO_POINTER(child_id))) {
                expand_node(proxy, child_id, timeout_ms);
                traverse(proxy, child_id, path_segs, entries,
                         service, object_path, timeout_ms, NULL, visited);
            }
        } else if (enabled) {
            /* Leaf item: build MenuEntry */
            GVariant *shortcut_v = g_variant_lookup_value(child_props, "shortcut",
                                                          G_VARIANT_TYPE("aas"));
            char *shortcut = format_shortcut(shortcut_v);
            if (shortcut_v) g_variant_unref(shortcut_v);

            /* Build path strings from path_segs */
            int n = (int)path_segs->len;
            const char **segs = (const char **)path_segs->pdata;
            char *markup = mnemonic_build_path_markup(segs, n);
            char *plain  = mnemonic_build_path_plain(segs, n);

            MenuEntry *e = g_new0(MenuEntry, 1);
            e->path_markup    = markup;
            e->path_plain     = plain;
            e->shortcut       = shortcut;
            e->backend        = BACKEND_DBUSMENU;
            e->dbus_service   = g_strdup(service);
            e->dbus_object_path = g_strdup(object_path);
            e->dbus_item_id   = child_id;
            e->enabled        = TRUE;
            e->use_count      = 0;
            g_ptr_array_add(entries, e);
        }

        /* Pop this label */
        g_ptr_array_remove_index(path_segs, path_segs->len - 1);

        if (label_v) g_variant_unref(label_v);
        g_variant_unref(child_props);
        g_variant_unref(child_children);
        g_variant_unref(child);
    }

    g_variant_unref(props_var);
    g_variant_unref(children_var);
    g_variant_unref(layout);
}

GPtrArray *collect_dbusmenu(const char *service, const char *object_path)
{
    g_autoptr(GError) err = NULL;
    g_autoptr(GDBusConnection) conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &err);
    if (!conn) return NULL;

    /* Detect Chromium-like services that need shorter timeouts */
    int timeout_ms = CALL_TIMEOUT_MS;
    const char *chromium_tokens[] = {"chrom", "brave", "vivaldi",
                                     "ungoogled", "microsoft-edge", "msedge", NULL};
    for (int i = 0; chromium_tokens[i]; i++) {
        if (g_strstr_len(service, -1, chromium_tokens[i])) {
            timeout_ms = CALL_TIMEOUT_CHROMIUM_MS;
            break;
        }
    }

    GDBusProxy *proxy = g_dbus_proxy_new_sync(conn,
        G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
        G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
        NULL, service, object_path, DBUSMENU_IFACE,
        NULL, &err);
    if (!proxy) return NULL;

    GPtrArray *entries   = g_ptr_array_new_with_free_func((GDestroyNotify)menu_entry_free);
    GPtrArray *path_segs = g_ptr_array_new();
    GList     *level1_ids = NULL;
    GHashTable *visited  = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_hash_table_add(visited, GINT_TO_POINTER(0));

    traverse(proxy, 0, path_segs, entries, service, object_path,
             timeout_ms, &level1_ids, visited);
    g_hash_table_destroy(visited);

    /* Firefox/VimFx workaround: send "closed" to all level-1 items */
    guint32 ts = (guint32)(g_get_monotonic_time() / 1000);
    for (GList *l = level1_ids; l; l = l->next) {
        int id = GPOINTER_TO_INT(l->data);
        g_dbus_proxy_call_sync(proxy, "Event",
            g_variant_new("(isvu)", id, "closed",
                          g_variant_new_int32(0), ts),
            G_DBUS_CALL_FLAGS_NONE, timeout_ms, NULL, NULL);
    }
    g_list_free(level1_ids);
    g_ptr_array_free(path_segs, TRUE);
    g_object_unref(proxy);

    return entries;
}
