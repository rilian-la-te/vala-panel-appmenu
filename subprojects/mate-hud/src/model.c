#include "model.h"
#include <string.h>

void menu_entry_free(MenuEntry *e)
{
    if (!e) return;
    g_free(e->path_markup);
    g_free(e->path_plain);
    g_free(e->shortcut);
    g_free(e->dbus_service);
    g_free(e->dbus_object_path);
    g_free(e->action_bus_name);
    g_free(e->action_group_path);
    g_free(e->action_name);
    if (e->action_target) g_variant_unref(e->action_target);
    g_free(e);
}

GtkListStore *model_new_store(void)
{
    return gtk_list_store_new(NUM_COLUMNS,
        G_TYPE_STRING,   /* COL_PATH_MARKUP */
        G_TYPE_STRING,   /* COL_PATH_PLAIN  */
        G_TYPE_STRING,   /* COL_ALT_INDEX   */
        G_TYPE_STRING,   /* COL_SHORTCUT    */
        G_TYPE_UINT,     /* COL_ENTRY_IDX   */
        G_TYPE_BOOLEAN   /* COL_VISIBLE     */
    );
}

void model_populate(GtkListStore *store, GPtrArray *entries)
{
    gtk_list_store_clear(store);
    for (guint i = 0; i < entries->len; i++) {
        MenuEntry *e = g_ptr_array_index(entries, i);
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            COL_PATH_MARKUP, e->path_markup ? e->path_markup : "",
            COL_PATH_PLAIN,  e->path_plain  ? e->path_plain  : "",
            COL_ALT_INDEX,   "",
            COL_SHORTCUT,    e->shortcut    ? e->shortcut    : "",
            COL_ENTRY_IDX,   i,
            COL_VISIBLE,     TRUE,
            -1);
    }
}

/* Compute match rank for an entry against the query (lower = better match). */
static int rank(const char *plain, const char *query)
{
    if (!query || *query == '\0') return 4; /* no query: all visible, equal rank */

    char *p = g_utf8_casefold(plain, -1);
    char *q = g_utf8_casefold(query, -1);

    /* Extract leaf: last segment after last space (we joined with spaces in plain) */
    const char *leaf = strrchr(p, ' ');
    leaf = leaf ? leaf + 1 : p;

    int r;
    if (g_strcmp0(leaf, q) == 0)          r = 0;
    else if (g_str_has_prefix(leaf, q))   r = 1;
    else if (strstr(leaf, q))             r = 2;
    else if (strstr(p, q))               r = 3;
    else                                  r = 5; /* no match */

    g_free(p);
    g_free(q);
    return r;
}

void model_refilter(GtkListStore *store,
                    GtkTreeModelFilter *filter_model,
                    GPtrArray *entries,
                    const char *query)
{
    gboolean no_query = (!query || *query == '\0');

    /* First pass: set COL_VISIBLE */
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    while (valid) {
        guint idx;
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, COL_ENTRY_IDX, &idx, -1);

        gboolean visible = no_query;
        if (!no_query && idx < entries->len) {
            MenuEntry *e = g_ptr_array_index(entries, idx);
            visible = (rank(e->path_plain ? e->path_plain : "", query) < 5);
        }
        gtk_list_store_set(store, &iter, COL_VISIBLE, visible, -1);
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }

    gtk_tree_model_filter_refilter(filter_model);

    /* Second pass: update COL_ALT_INDEX for visible rows */
    model_sort_by_rank(store, entries, query);

    int pos = 1;
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    while (valid) {
        gboolean vis;
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, COL_VISIBLE, &vis, -1);
        if (vis) {
            char label[16];
            if (pos <= 9)       g_snprintf(label, sizeof(label), "Alt+%d", pos);
            else if (pos == 10) g_snprintf(label, sizeof(label), "Alt+0");
            else                label[0] = '\0';
            gtk_list_store_set(store, &iter, COL_ALT_INDEX, label, -1);
            pos++;
        } else {
            gtk_list_store_set(store, &iter, COL_ALT_INDEX, "", -1);
        }
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }
}

typedef struct { int rank; int use_count; int original_idx; } SortKey;

static int sort_key_compare(const void *a, const void *b)
{
    const SortKey *ka = a, *kb = b;
    if (ka->rank != kb->rank)          return ka->rank - kb->rank;
    if (ka->use_count != kb->use_count) return kb->use_count - ka->use_count;
    return ka->original_idx - kb->original_idx;
}

void model_sort_by_rank(GtkListStore *store, GPtrArray *entries, const char *query)
{
    /* Build a sorted index array, then reorder the store. */
    int n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
    if (n == 0) return;

    /* Collect (store_position → entry_idx) pairs from store */
    GArray *keys = g_array_sized_new(FALSE, FALSE, sizeof(SortKey), n);
    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    int store_pos = 0;
    while (valid) {
        guint entry_idx;
        gboolean vis;
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
                           COL_ENTRY_IDX, &entry_idx,
                           COL_VISIBLE,   &vis,
                           -1);
        MenuEntry *e = (entry_idx < entries->len)
            ? g_ptr_array_index(entries, entry_idx) : NULL;

        SortKey k;
        k.original_idx = store_pos;
        k.use_count    = e ? e->use_count : 0;
        k.rank         = vis
            ? rank(e && e->path_plain ? e->path_plain : "", query)
            : 99;  /* hidden rows sink to end */

        g_array_append_val(keys, k);
        store_pos++;
        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
    }

    g_array_sort(keys, sort_key_compare);

    /* Build new_order: new_order[new_position] = old_position */
    gint *new_order = g_new(gint, n);
    for (int i = 0; i < n; i++)
        new_order[i] = ((SortKey *)keys->data)[i].original_idx;

    gtk_list_store_reorder(store, new_order);
    g_free(new_order);
    g_array_free(keys, TRUE);
}
