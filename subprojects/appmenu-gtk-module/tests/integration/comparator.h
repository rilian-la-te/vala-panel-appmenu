/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <debug.h>
#include <gio/gio.h>

#define G_MENU_ATTRIBUTE_ACCEL "accel"
#define G_MENU_ATTRIBUTE_ACCEL_TEXT "x-canonical-accel"
#define G_MENU_ATTRIBUTE_SUBMENU_ACTION "submenu-action"
#define G_MENU_ATTRIBUTE_HIDDEN_WHEN "hidden-when"
#define G_MENU_ATTRIBUTE_VERB_ICON "verb-icon"

gboolean compare_models(GMenuModel *a, GMenuModel *b, GString *errors, int depth);
void check_any_actions(GMenuModel *model, gboolean *found);
