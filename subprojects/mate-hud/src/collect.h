#pragma once
#include "model.h"
#include <glib.h>

/*
 * Detect the menu backend for the given window XID and collect all menu entries.
 *
 * Returns a non-empty GPtrArray of MenuEntry* on success, or NULL if:
 *   - no menu is available for the window
 *   - the collected menu has no actionable items
 *   - a D-Bus or timeout error occurred
 *
 * Caller owns the array; free with g_ptr_array_unref().
 */
GPtrArray *collect_menu_for_window(gulong xid);
