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

#ifndef __UNITY_GTK_ACTION_PRIVATE_H__
#define __UNITY_GTK_ACTION_PRIVATE_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _UnityGtkAction UnityGtkAction;
typedef GObjectClass UnityGtkActionClass;

#define UNITY_GTK_TYPE_ACTION (unity_gtk_action_get_type())
#define UNITY_GTK_ACTION(obj)                                                                      \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), UNITY_GTK_TYPE_ACTION, UnityGtkAction))
#define UNITY_GTK_IS_ACTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), UNITY_GTK_TYPE_ACTION))
#define UNITY_GTK_ACTION_CLASS(klass)                                                              \
	(G_TYPE_CHECK_CLASS_CAST((klass), UNITY_GTK_TYPE_ACTION, UnityGtkActionClass))
#define UNITY_GTK_IS_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), UNITY_GTK_TYPE_ACTION))
#define UNITY_GTK_ACTION_GET_CLASS(obj)                                                            \
	(G_TYPE_INSTANCE_GET_CLASS((obj), UNITY_GTK_TYPE_ACTION, UnityGtkActionClass))

G_END_DECLS

#include "unity-gtk-menu-item-private.h"

G_BEGIN_DECLS

struct _UnityGtkAction
{
	GObject parent_instance;

	/*< private >*/
	char *name;
	char *subname;
	UnityGtkMenuItem *item;
	GHashTable *items_by_name;
};

GType unity_gtk_action_get_type(void) G_GNUC_INTERNAL;

UnityGtkAction *unity_gtk_action_new(const char *name, UnityGtkMenuItem *item) G_GNUC_INTERNAL;

UnityGtkAction *unity_gtk_action_new_radio(const char *name) G_GNUC_INTERNAL;

void unity_gtk_action_set_name(UnityGtkAction *action, const char *name) G_GNUC_INTERNAL;

void unity_gtk_action_set_subname(UnityGtkAction *action, const char *subname) G_GNUC_INTERNAL;

void unity_gtk_action_set_item(UnityGtkAction *action, UnityGtkMenuItem *item) G_GNUC_INTERNAL;

void unity_gtk_action_print(UnityGtkAction *action, guint indent) G_GNUC_INTERNAL;

G_END_DECLS

#endif /* __UNITY_GTK_ACTION_PRIVATE_H__ */
