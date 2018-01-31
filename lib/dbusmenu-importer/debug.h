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
G_GNUC_INTERNAL GString *g_menu_markup_print_string(GString *string, GMenuModel *model, gint indent,
                                                    gint tabstop);
G_GNUC_INTERNAL void g_menu_markup_print_to_console(GMenuModel *menu);
G_GNUC_INTERNAL void dbus_menu_print_variant(GVariant *var);
#endif // DEBUG_H
