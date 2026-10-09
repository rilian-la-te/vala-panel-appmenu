/*
 * vala-panel-appmenu
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <gio/gio.h>
void g_menu_markup_print_to_console(GMenuModel *menu);
void dbus_menu_print_variant(GVariant *var);

gchar *gtk_print_action_and_target(const gchar *action_namespace, const gchar *action_name,
                                   GVariant *target);
gchar *gtk_normalise_detailed_action_name(const gchar *detailed_action_name);

#endif // DEBUG_H
