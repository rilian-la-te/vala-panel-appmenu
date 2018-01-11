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

#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <gtk/gtk.h>
#include <stdbool.h>

typedef struct _WindowData WindowData;

G_GNUC_INTERNAL GQuark window_data_quark(void);
G_GNUC_INTERNAL WindowData *window_data_new(void);
G_GNUC_INTERNAL WindowData *window_data_copy(WindowData *source);
G_GNUC_INTERNAL WindowData *gtk_window_get_window_data(GtkWindow *window);
G_GNUC_INTERNAL void window_data_free(gpointer data);
G_DEFINE_AUTOPTR_CLEANUP_FUNC(WindowData, window_data_free);

typedef struct _MenuShellData MenuShellData;

G_GNUC_INTERNAL MenuShellData *menu_shell_data_new(void);
G_GNUC_INTERNAL MenuShellData *menu_shell_data_copy(MenuShellData *source);
G_GNUC_INTERNAL MenuShellData *gtk_menu_shell_get_menu_shell_data(GtkMenuShell *menu_shell);
G_GNUC_INTERNAL bool menu_shell_data_has_window(MenuShellData *source);
G_GNUC_INTERNAL GtkWindow *menu_shell_data_get_window(MenuShellData *source);
G_GNUC_INTERNAL void menu_shell_data_free(gpointer data);
G_DEFINE_AUTOPTR_CLEANUP_FUNC(MenuShellData, menu_shell_data_free);

G_GNUC_INTERNAL void gtk_window_connect_menu_shell(GtkWindow *window, GtkMenuShell *menu_shell);
G_GNUC_INTERNAL void gtk_window_disconnect_menu_shell(GtkWindow *window, GtkMenuShell *menu_shell);

#endif // DATASTRUCTS_H
