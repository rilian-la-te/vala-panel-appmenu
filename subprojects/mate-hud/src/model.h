#pragma once
#include <glib.h>
#include <gtk/gtk.h>

typedef enum {
    BACKEND_DBUSMENU,
    BACKEND_MENUMODEL,
} MenuBackend;

typedef struct {
    /* Display */
    char        *path_markup;   /* Pango markup, e.g. "<u>F</u>ile ▸ <u>O</u>pen" */
    char        *path_plain;    /* Plain text for search matching */
    char        *shortcut;      /* "Ctrl+O" or NULL */

    /* Invocation — DBusMenu */
    char        *dbus_service;
    char        *dbus_object_path;
    int          dbus_item_id;

    /* Invocation — GMenuModel */
    char        *action_bus_name;
    char        *action_group_path;
    char        *action_name;
    GVariant    *action_target;  /* owned ref, may be NULL */

    MenuBackend  backend;
    gboolean     enabled;

    /* Ranking */
    int          use_count;
} MenuEntry;

void        menu_entry_free(MenuEntry *e);

/*
 * GtkListStore column indices.
 * COL_PATH_MARKUP and COL_PATH_PLAIN are kept in sync with entries[].
 */
enum {
    COL_PATH_MARKUP = 0,
    COL_PATH_PLAIN,
    COL_ALT_INDEX,
    COL_SHORTCUT,
    COL_ENTRY_IDX,   /* guint index into the GPtrArray */
    COL_VISIBLE,
    NUM_COLUMNS
};

GtkListStore *model_new_store(void);

/* Populate store from entries array. Clears previous contents. */
void model_populate(GtkListStore *store, GPtrArray *entries);

/*
 * Refilter: set COL_VISIBLE on every row based on query, then call refilter.
 * Also updates COL_ALT_INDEX for the first 10 visible rows.
 * filter_model must wrap store.
 */
void model_refilter(GtkListStore *store,
                    GtkTreeModelFilter *filter_model,
                    GPtrArray *entries,
                    const char *query);

/* Sort visible rows by rank (after refilter). */
void model_sort_by_rank(GtkListStore *store, GPtrArray *entries, const char *query);
