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

#ifndef MODEL_H
#define MODEL_H

#include "dbusmenu-interface.h"
#include <gio/gio.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuModel, dbus_menu_model, DBUS_MENU, MODEL, GMenuModel)
G_GNUC_INTERNAL DBusMenuModel *dbus_menu_model_new(uint parent_id, DBusMenuModel *parent,
                                                   DBusMenuXml *xml, GActionGroup *action_group);
G_GNUC_INTERNAL void dbus_menu_model_update_layout(DBusMenuModel *menu);
G_GNUC_INTERNAL void dbus_menu_model_update_layout_sync(DBusMenuModel *menu);
G_GNUC_INTERNAL bool dbus_menu_model_is_layout_update_required(DBusMenuModel *model);
G_GNUC_INTERNAL void dbus_menu_model_set_layout_update_required(DBusMenuModel *model,
                                                                bool required);

G_END_DECLS

#endif
