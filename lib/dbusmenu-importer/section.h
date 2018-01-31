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

#ifndef SECTION_H
#define SECTION_H

#include "model.h"
#include <gio/gio.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuSectionModel, dbus_menu_section_model, DBUS_MENU, SECTION_MODEL,
                     GMenuModel)

struct _DBusMenuSectionModel
{
	GMenuModel parent_instance;

	DBusMenuModel *parent_model;
	GSequence *items;
	uint section_index;
};

DBusMenuSectionModel *dbus_menu_section_model_new(DBusMenuModel *parent, int section_index);

G_END_DECLS

#endif // SECTION_H
