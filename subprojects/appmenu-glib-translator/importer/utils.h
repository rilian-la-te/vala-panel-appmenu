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

#ifndef UTILS_H
#define UTILS_H

#include "dbusmenu-interface.h"
#include "definitions.h"
#include "model.h"
#include <gio/gio.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL GAction *dbus_menu_action_reference(u_int32_t id, DBusMenuXml *xml,
                                                    DBusMenuModel *submenu,
                                                    GActionMap *action_group,
                                                    DBusMenuActionType type);

G_GNUC_INTERNAL char *dbus_menu_action_get_name(uint id, DBusMenuActionType action_type,
                                                bool use_prefix);
G_GNUC_INTERNAL void dbus_menu_action_replace_signals(GAction *action, DBusMenuXml *xml,
                                                      DBusMenuModel *submenu,
                                                      DBusMenuActionType action_type);

G_GNUC_INTERNAL void dbus_menu_action_lock(GAction *action);
G_GNUC_INTERNAL void dbus_menu_action_unlock(GAction *action);

G_END_DECLS

#endif // UTILS_H
