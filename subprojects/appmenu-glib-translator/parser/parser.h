/* SPDX-License-Identifier: LGPL-3.0-or-later */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

void dbus_menu_print_stdout(GMenuModel *menu);
char* dbus_menu_print_string(GMenuModel *menu);
/**
 * dbus_menu_parse_one:
 * @error: (nullable): return location for a #GError, or %NULL
 *
 * Returns: (transfer full) (nullable): the parsed #GMenuModel, or %NULL on error.
 */
GMenuModel *dbus_menu_parse_one(const char *markup, const char *gettext_package, GError **error);
/**
 * dbus_menu_parsed_find_item_by_id:
 *
 * The returned #GMenuItem is new and its scalar attributes are a private copy,
 * but its submenu/section links point at the SAME live sub-models held by
 * @model, so they can be modified in place. Those links are borrowed: the item
 * must not outlive @model.
 *
 * Returns: (transfer full) (nullable): a new #GMenuItem sharing @model's links,
 *   or %NULL when no item declares that id.
 */
GMenuItem *dbus_menu_parsed_find_item_by_id(GMenuModel *model, const char *id);

G_END_DECLS
