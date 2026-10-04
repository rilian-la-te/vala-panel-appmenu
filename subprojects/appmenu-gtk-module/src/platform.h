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

#ifndef PLATFORM_H
#define PLATFORM_H

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#if GTK_MAJOR_VERSION >= 3
#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif
#endif

#include "datastructs.h"

#ifdef GDK_WINDOWING_X11
G_GNUC_INTERNAL WindowData *gtk_x11_window_get_window_data(GtkWindow *window);
#endif

#ifdef GDK_WINDOWING_WAYLAND
G_GNUC_INTERNAL WindowData *gtk_wayland_window_get_window_data(GtkWindow *window);
#endif

#endif // PLATFORM_H
