/*
 * appmenu-gtk-module
 * Copyright 2012 Canonical Ltd.
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
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
 *
 * Authors: Ryan Lortie <desrt@desrt.ca>
 *          William Hua <william.hua@canonical.com>
 *          Konstantin Pugin <ria.freelander@gmail.com>
 *          Lester Carballo Perez <lestcape@gmail.com>
 */

#ifndef DATASTRUCTSPRIVATE_H
#define DATASTRUCTSPRIVATE_H

#include <appmenu-gtk-action-group.h>
#include <gtk/gtk.h>

struct _WindowData
{
	uint window_id;
	ulong wayland_window_id;
	GMenu *menu_model;
	guint menu_model_export_id;
	GSList *menus;
	GMenuModel *old_model;
	UnityGtkActionGroup *action_group;
	guint action_group_export_id;
};

struct _MenuShellData
{
	GtkWindow *window;
};

#endif // DATASTRUCTSPRIVATE_H
