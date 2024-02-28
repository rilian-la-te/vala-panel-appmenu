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

#ifndef __UNITY_GTK_ACTION_GROUP_H__
#define __UNITY_GTK_ACTION_GROUP_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define UNITY_GTK_TYPE_ACTION_GROUP (unity_gtk_action_group_get_type())
G_DECLARE_FINAL_TYPE(UnityGtkActionGroup, unity_gtk_action_group, UNITY_GTK, ACTION_GROUP, GObject)

G_END_DECLS

#include "appmenu-gtk-menu-shell.h"

G_BEGIN_DECLS

/**
 * UnityGtkActionGroup:
 *
 * Opaque action group collector for #UnityGtkMenuShell.
 */
struct _UnityGtkActionGroup
{
	GObject parent_instance;

	/*< private >*/
	GActionGroup *old_group;
	GHashTable *actions_by_name;
	GHashTable *names_by_radio_menu_item;
};

GType unity_gtk_action_group_get_type(void);

UnityGtkActionGroup *unity_gtk_action_group_new(GActionGroup *old_group);

void unity_gtk_action_group_connect_shell(UnityGtkActionGroup *group, UnityGtkMenuShell *shell);

void unity_gtk_action_group_disconnect_shell(UnityGtkActionGroup *group, UnityGtkMenuShell *shell);

void unity_gtk_action_group_set_debug(gboolean debug);

G_END_DECLS

#endif /* __UNITY_GTK_ACTION_GROUP_H__ */
