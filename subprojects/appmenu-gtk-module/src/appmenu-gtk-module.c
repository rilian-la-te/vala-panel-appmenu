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

#include <gtk/gtk.h>

#include "hijack.h"
#include "support.h"

static void sync_gtk2_settings()
{
#if GTK_MAJOR_VERSION < 3
	GParamSpec *pspec;

	pspec = g_object_class_find_property(g_type_class_ref(GTK_TYPE_SETTINGS),
	                                     "gtk-shell-shows-menubar");

	if (!G_IS_PARAM_SPEC(pspec))
	{
		gtk_settings_install_property(
		    g_param_spec_boolean("gtk-shell-shows-menubar",
		                         "Desktop shell shows the menubar",
		                         "Set to TRUE if the desktop environment is displaying the "
		                         "menubar, FALSE if the app should display it itself.",
		                         FALSE,
		                         G_PARAM_READWRITE));
	}
#endif
}

void gtk_module_init(void)
{
	if (gtk_module_should_run())
	{
		sync_gtk2_settings();
		watch_registrar_dbus();
		enable_debug();
		store_pre_hijacked();
		hijack_menu_bar_class_vtable(GTK_TYPE_MENU_BAR);
	}
}
