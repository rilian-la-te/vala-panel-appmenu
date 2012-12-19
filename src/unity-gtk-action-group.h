/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

typedef struct _UnityGtkActionGroup      UnityGtkActionGroup;
typedef struct _UnityGtkActionGroupClass UnityGtkActionGroupClass;

#define UNITY_GTK_TYPE_ACTION_GROUP            (unity_gtk_action_group_get_type ())
#define UNITY_GTK_ACTION_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroup))
#define UNITY_GTK_IS_ACTION_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_GTK_TYPE_ACTION_GROUP))
#define UNITY_GTK_ACTION_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroupClass))
#define UNITY_GTK_IS_ACTION_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_GTK_TYPE_ACTION_GROUP))
#define UNITY_GTK_ACTION_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroupClass))

#include <glib-object.h>
#include "unity-gtk-menu-shell.h"

struct _UnityGtkActionGroup
{
  GObject parent_instance;

  /*< private >*/
  GActionGroup *old_group;
  GHashTable   *actions_by_name;
  GHashTable   *names_by_radio_menu_item;
};

struct _UnityGtkActionGroupClass
{
  GObjectClass parent_class;
};

GType                 unity_gtk_action_group_get_type         (void)                           G_GNUC_INTERNAL;

UnityGtkActionGroup * unity_gtk_action_group_new              (GActionGroup        *old_group) G_GNUC_INTERNAL;

void                  unity_gtk_action_group_connect_item     (UnityGtkActionGroup *group,
                                                               UnityGtkMenuItem    *item)      G_GNUC_INTERNAL;

void                  unity_gtk_action_group_disconnect_item  (UnityGtkActionGroup *group,
                                                               UnityGtkMenuItem    *item)      G_GNUC_INTERNAL;

void                  unity_gtk_action_group_connect_shell    (UnityGtkActionGroup *group,
                                                               UnityGtkMenuShell   *shell)     G_GNUC_INTERNAL;

void                  unity_gtk_action_group_disconnect_shell (UnityGtkActionGroup *group,
                                                               UnityGtkMenuShell   *shell)     G_GNUC_INTERNAL;

void                  unity_gtk_action_group_print            (UnityGtkActionGroup *group,
                                                               guint                indent)    G_GNUC_INTERNAL;

#endif /* __UNITY_GTK_ACTION_GROUP_H__ */
