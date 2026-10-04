#pragma once
#include "model.h"
#include <glib.h>

/*
 * Load history from ~/.config/mate-hud/history.json.
 * Applies use_count values to matching entries in the array.
 * wm_class: the active window's WM class string (from XGetClassHint).
 */
void history_load(GPtrArray *entries, const char *wm_class);

/*
 * Record activation of an item.
 * wm_class: the active window's WM class string.
 * path_plain: the plain-text menu path (used as history key).
 * max_entries: maximum items to keep per app (from GSettings).
 */
void history_record(const char *wm_class, const char *path_plain, int max_entries);
