#pragma once
#include "model.h"
#include <glib.h>

/*
 * Harvest all menu items from a DBusMenu service.
 *
 * Returns a GPtrArray of MenuEntry* on success (may be empty if the menu has
 * no actionable leaves), or NULL on hard failure (service unreachable, timeout).
 *
 * Caller takes ownership; free with g_ptr_array_unref().
 */
GPtrArray *collect_dbusmenu(const char *service, const char *object_path);
