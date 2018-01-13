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

#ifndef SUPPORT_H
#define SUPPORT_H

#include <gtk/gtk.h>
#include <stdbool.h>

G_GNUC_INTERNAL bool gtk_widget_shell_shows_menubar(GtkWidget *widget);
G_GNUC_INTERNAL void gtk_widget_connect_settings(GtkWidget *widget);
G_GNUC_INTERNAL void gtk_widget_disconnect_settings(GtkWidget *widget);
G_GNUC_INTERNAL bool gtk_module_should_run();
G_GNUC_INTERNAL void watch_registrar_dbus();
G_GNUC_INTERNAL void enable_debug();

#endif
