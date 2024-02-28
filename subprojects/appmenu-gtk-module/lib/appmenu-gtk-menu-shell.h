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

#ifndef __UNITY_GTK_MENU_SHELL_H__
#define __UNITY_GTK_MENU_SHELL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define UNITY_GTK_TYPE_MENU_SHELL (unity_gtk_menu_shell_get_type())
G_DECLARE_FINAL_TYPE(UnityGtkMenuShell, unity_gtk_menu_shell, UNITY_GTK, MENU_SHELL, GMenuModel)

G_END_DECLS

#include "appmenu-gtk-action-group.h"

G_BEGIN_DECLS

/**
 * UnityGtkMenuShell:
 *
 * Opaque #GMenuModel proxy for #GtkMenuShell.
 */
struct _UnityGtkMenuShell
{
	GMenuModel parent_instance;

	/*< private >*/
	GtkMenuShell *menu_shell;
	gboolean has_mnemonics;
	GPtrArray *items;
	GPtrArray *sections;
	GSequence *visible_indices;
	GSequence *separator_indices;
	UnityGtkActionGroup *action_group;
};

GType unity_gtk_menu_shell_get_type(void);

UnityGtkMenuShell *unity_gtk_menu_shell_new(GtkMenuShell *menu_shell);

void unity_gtk_menu_shell_set_debug(gboolean debug);

G_END_DECLS

#endif /* __UNITY_GTK_MENU_SHELL_H__ */
