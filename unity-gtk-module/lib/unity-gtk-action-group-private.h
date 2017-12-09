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

#ifndef __UNITY_GTK_ACTION_GROUP_PRIVATE_H__
#define __UNITY_GTK_ACTION_GROUP_PRIVATE_H__

#include "appmenu-gtk-action-group.h"
#include "unity-gtk-menu-item-private.h"
#include <glib.h>

G_BEGIN_DECLS

void unity_gtk_action_group_connect_item(UnityGtkActionGroup *group,
                                         UnityGtkMenuItem *item) G_GNUC_INTERNAL;

void unity_gtk_action_group_disconnect_item(UnityGtkActionGroup *group,
                                            UnityGtkMenuItem *item) G_GNUC_INTERNAL;

void unity_gtk_action_group_print(UnityGtkActionGroup *group, guint indent) G_GNUC_INTERNAL;

gboolean unity_gtk_action_group_is_debug(void) G_GNUC_INTERNAL;

G_END_DECLS

#endif /* __UNITY_GTK_ACTION_GROUP_PRIVATE_H__ */
