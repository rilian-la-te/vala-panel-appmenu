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

#include "unity-gtk-action-private.h"

G_DEFINE_TYPE(UnityGtkAction, unity_gtk_action, G_TYPE_OBJECT);

static void unity_gtk_action_dispose(GObject *object)
{
	UnityGtkAction *action;
	GHashTable *items_by_name;

	g_return_if_fail(UNITY_GTK_IS_ACTION(object));

	action        = UNITY_GTK_ACTION(object);
	items_by_name = action->items_by_name;

	if (items_by_name != NULL)
	{
		action->items_by_name = NULL;
		g_hash_table_unref(items_by_name);
	}

	unity_gtk_action_set_item(action, NULL);
	unity_gtk_action_set_subname(action, NULL);
	unity_gtk_action_set_name(action, NULL);

	G_OBJECT_CLASS(unity_gtk_action_parent_class)->dispose(object);
}

static void unity_gtk_action_class_init(UnityGtkActionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = unity_gtk_action_dispose;
}

static void unity_gtk_action_init(UnityGtkAction *self)
{
}

UnityGtkAction *unity_gtk_action_new(const char *name, UnityGtkMenuItem *item)
{
	UnityGtkAction *action = g_object_new(UNITY_GTK_TYPE_ACTION, NULL);

	unity_gtk_action_set_name(action, name);
	unity_gtk_action_set_item(action, item);

	return action;
}

UnityGtkAction *unity_gtk_action_new_radio(const char *name)
{
	UnityGtkAction *action = g_object_new(UNITY_GTK_TYPE_ACTION, NULL);

	unity_gtk_action_set_name(action, name);
	action->items_by_name =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);

	return action;
}

void unity_gtk_action_set_name(UnityGtkAction *action, const char *name)
{
	g_return_if_fail(UNITY_GTK_IS_ACTION(action));

	g_free(action->name);
	action->name = g_strdup(name);
}

void unity_gtk_action_set_subname(UnityGtkAction *action, const char *subname)
{
	g_return_if_fail(UNITY_GTK_IS_ACTION(action));

	g_free(action->subname);
	action->subname = g_strdup(subname);
}

void unity_gtk_action_set_item(UnityGtkAction *action, UnityGtkMenuItem *item)
{
	UnityGtkMenuItem *old_item;

	g_return_if_fail(UNITY_GTK_IS_ACTION(action));

	old_item = action->item;

	if (item != old_item)
	{
		if (old_item != NULL)
		{
			action->item = NULL;
			g_object_unref(old_item);
		}

		if (item != NULL)
			action->item = g_object_ref(item);
	}
}

void unity_gtk_action_print(UnityGtkAction *action, guint indent)
{
	char *space;

	g_return_if_fail(action == NULL || UNITY_GTK_IS_ACTION(action));

	space = g_strnfill(indent, ' ');

	if (action != NULL)
	{
		g_print("%s(%s *) %p\n",
		        space,
		        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(action)),
		        action);

		if (action->name != NULL)
			g_print("%s  \"%s\"\n", space, action->name);

		if (action->subname != NULL)
			g_print("%s  \"%s\"\n", space, action->subname);

		if (action->item != NULL)
			g_print("%s  (%s *) %p\n",
			        space,
			        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(action->item)),
			        action->item);

		if (action->items_by_name != NULL)
		{
			GHashTableIter iter;
			gpointer key;
			gpointer value;

			g_hash_table_iter_init(&iter, action->items_by_name);
			while (g_hash_table_iter_next(&iter, &key, &value))
				g_print("%s  \"%s\" -> (%s *) %p\n",
				        space,
				        (const char *)key,
				        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(value)),
				        value);
		}
	}
	else
		g_print("%sNULL\n", space);

	g_free(space);
}
