/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Ryan Lortie <desrt@desrt.ca>
 *          William Hua <william.hua@canonical.com>
 */

#ifndef __UNITY_GTK_MENU_SHELL_PRIVATE_H__
#define __UNITY_GTK_MENU_SHELL_PRIVATE_H__

#include "appmenu-gtk-menu-shell.h"
#include "unity-gtk-menu-item-private.h"

#include <glib-object.h>

G_BEGIN_DECLS

UnityGtkMenuShell *unity_gtk_menu_shell_new_internal(GtkMenuShell *menu_shell) G_GNUC_INTERNAL;

UnityGtkMenuItem *unity_gtk_menu_shell_get_item(UnityGtkMenuShell *shell,
                                                guint index) G_GNUC_INTERNAL;

GSequence *unity_gtk_menu_shell_get_visible_indices(UnityGtkMenuShell *shell) G_GNUC_INTERNAL;

GSequence *unity_gtk_menu_shell_get_separator_indices(UnityGtkMenuShell *shell) G_GNUC_INTERNAL;

void unity_gtk_menu_shell_handle_item_notify(UnityGtkMenuShell *shell, UnityGtkMenuItem *item,
                                             const char *property) G_GNUC_INTERNAL;

void unity_gtk_menu_shell_activate_item(UnityGtkMenuShell *shell,
                                        UnityGtkMenuItem *item) G_GNUC_INTERNAL;

void unity_gtk_menu_shell_print(UnityGtkMenuShell *shell, guint indent) G_GNUC_INTERNAL;

gboolean unity_gtk_menu_shell_is_debug(void) G_GNUC_INTERNAL;

G_END_DECLS

#endif /* __UNITY_GTK_MENU_SHELL_PRIVATE_H__ */
