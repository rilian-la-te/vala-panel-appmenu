#include "history.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>
#include <string.h>

/* Bounds on history.json so it cannot grow without limit: items kept per
 * application (matches the hud-history-max schema range) and applications
 * kept overall. */
#define HISTORY_MAX_PER_APP 500
#define HISTORY_MAX_APPS    100

static char *history_path(void)
{
    return g_build_filename(g_get_user_config_dir(), "mate-hud", "history.json", NULL);
}

/* Load root JSON object from file; returns NULL on error. Caller unrefs. */
static JsonObject *load_root(void)
{
    char *path = history_path();
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
        g_free(path);
        return NULL;
    }

    g_autoptr(GError) err = NULL;
    JsonParser *parser = json_parser_new();
    if (!json_parser_load_from_file(parser, path, &err)) {
        g_object_unref(parser);
        g_free(path);
        return NULL;
    }
    g_free(path);

    JsonNode *root = json_parser_get_root(parser);
    if (!root || !JSON_NODE_HOLDS_OBJECT(root)) {
        g_object_unref(parser);
        return NULL;
    }

    JsonObject *obj = json_node_dup_object(root);
    g_object_unref(parser);
    return obj;
}

/* Save root JSON object to file atomically. */
static void save_root(JsonObject *root_obj)
{
    char *dir  = g_build_filename(g_get_user_config_dir(), "mate-hud", NULL);
    char *path = history_path();
    char *tmp  = g_strdup_printf("%s.tmp", path);

    g_mkdir_with_parents(dir, 0700);

    JsonNode *root = json_node_new(JSON_NODE_OBJECT);
    json_node_set_object(root, root_obj);

    JsonGenerator *gen = json_generator_new();
    json_generator_set_root(gen, root);
    json_generator_set_pretty(gen, FALSE);

    g_autoptr(GError) err = NULL;
    json_generator_to_file(gen, tmp, &err);
    if (!err)
        g_rename(tmp, path);

    json_node_free(root);
    g_object_unref(gen);
    g_free(tmp);
    g_free(path);
    g_free(dir);
}

void history_load(GPtrArray *entries, const char *wm_class)
{
    if (!wm_class || !entries) return;

    JsonObject *root = load_root();
    if (!root) return;

    if (!json_object_has_member(root, wm_class)) {
        json_object_unref(root);
        return;
    }

    JsonNode *app_node = json_object_get_member(root, wm_class);
    if (!app_node || !JSON_NODE_HOLDS_OBJECT(app_node)) {
        json_object_unref(root);
        return;
    }
    JsonObject *app_obj = json_node_get_object(app_node);

    for (guint i = 0; i < entries->len; i++) {
        MenuEntry *e = g_ptr_array_index(entries, i);
        if (!e->path_plain) continue;
        if (json_object_has_member(app_obj, e->path_plain)) {
            gint64 count = json_object_get_int_member(app_obj, e->path_plain);
            e->use_count = (int)count;
        }
    }

    json_object_unref(root);
}

/* Trim an application's items to max_entries by removing the least-used ones. */
static void trim_app(JsonObject *app_obj, guint max_entries)
{
    while (json_object_get_size(app_obj) > max_entries) {
        GList *members = json_object_get_members(app_obj);
        /* Find the member with the lowest count */
        gint64 min_count = G_MAXINT64;
        const char *min_key = NULL;
        for (GList *l = members; l; l = l->next) {
            const char *k = l->data;
            gint64 c = json_object_get_int_member(app_obj, k);
            if (c < min_count) { min_count = c; min_key = k; }
        }
        if (min_key) json_object_remove_member(app_obj, min_key);
        g_list_free(members);
    }
}

void history_record(const char *wm_class, const char *path_plain, int max_entries)
{
    if (!wm_class || !path_plain) return;

    guint max_items = (guint)CLAMP(max_entries, 1, HISTORY_MAX_PER_APP);

    JsonObject *root = load_root();
    if (!root) root = json_object_new();

    /* Get or create the app object */
    JsonObject *app_obj;
    if (json_object_has_member(root, wm_class)) {
        JsonNode *n = json_object_get_member(root, wm_class);
        app_obj = JSON_NODE_HOLDS_OBJECT(n) ? json_node_dup_object(n) : json_object_new();
        json_object_remove_member(root, wm_class);
    } else {
        app_obj = json_object_new();
    }

    /* Increment count */
    gint64 count = 0;
    if (json_object_has_member(app_obj, path_plain))
        count = json_object_get_int_member(app_obj, path_plain);
    json_object_set_int_member(app_obj, path_plain, count + 1);

    /* Re-added last, so members stay ordered from least to most recently used. */
    json_object_set_object_member(root, wm_class, app_obj);

    /* Bound the whole file: drop the least recently used applications (and any
     * malformed ones) beyond the cap, and trim every remaining application to
     * the current limit. The app just recorded is last, so it is always kept. */
    GList *apps = json_object_get_members(root);
    guint n_apps = g_list_length(apps);
    for (GList *l = apps; l; l = l->next) {
        const char *k = l->data;
        JsonNode *n = json_object_get_member(root, k);
        if (n_apps > HISTORY_MAX_APPS || !JSON_NODE_HOLDS_OBJECT(n)) {
            json_object_remove_member(root, k);
            n_apps--;
            continue;
        }
        trim_app(json_node_get_object(n), max_items);
    }
    g_list_free(apps);

    save_root(root);
    json_object_unref(root);
}
