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

/**
 * SECTION:unity-gtk-action-group
 * @short_description: Action group collector
 * @include: unity-gtk-parser.h
 *
 * A #UnityGtkActionGroup is a #GActionGroup that accumulates the
 * actions of multiple #UnityGtkMenuShell<!-- -->s into a single object.
 * This can be used for purposes such as exporting actions over DBus
 * with g_dbus_connection_export_action_group ().
 */

#include "unity-gtk-action-group-private.h"
#include "unity-gtk-action-private.h"
#include <gio/gio.h>

static void unity_gtk_action_group_action_group_init(GActionGroupInterface *iface);

G_DEFINE_TYPE_WITH_CODE(UnityGtkActionGroup, unity_gtk_action_group, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(G_TYPE_ACTION_GROUP,
                                              unity_gtk_action_group_action_group_init));

static gboolean unity_gtk_action_group_debug;

static gboolean g_signal_emit_hide(gpointer user_data)
{
	g_signal_emit_by_name(user_data, "hide");

	return G_SOURCE_REMOVE;
}

static void unity_gtk_action_group_handle_group_action_added(GActionGroup *action_group,
                                                             char *action_name, gpointer user_data)
{
	UnityGtkActionGroup *group;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(user_data));

	group = UNITY_GTK_ACTION_GROUP(user_data);

	g_warn_if_fail(action_group == group->old_group);

	g_action_group_action_added(G_ACTION_GROUP(group), action_name);
}

static void unity_gtk_action_group_handle_group_action_removed(GActionGroup *action_group,
                                                               char *action_name,
                                                               gpointer user_data)
{
	UnityGtkActionGroup *group;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(user_data));

	group = UNITY_GTK_ACTION_GROUP(user_data);

	g_warn_if_fail(action_group == group->old_group);

	g_action_group_action_removed(G_ACTION_GROUP(group), action_name);
}

static void unity_gtk_action_group_handle_group_action_enabled_changed(GActionGroup *action_group,
                                                                       char *action_name,
                                                                       gboolean enabled,
                                                                       gpointer user_data)
{
	UnityGtkActionGroup *group;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(user_data));

	group = UNITY_GTK_ACTION_GROUP(user_data);

	g_warn_if_fail(action_group == group->old_group);

	g_action_group_action_enabled_changed(G_ACTION_GROUP(group), action_name, enabled);
}

static void unity_gtk_action_group_handle_group_action_state_changed(GActionGroup *action_group,
                                                                     char *action_name,
                                                                     GVariant *value,
                                                                     gpointer user_data)
{
	UnityGtkActionGroup *group;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(user_data));

	group = UNITY_GTK_ACTION_GROUP(user_data);

	g_warn_if_fail(action_group == group->old_group);

	g_action_group_action_state_changed(G_ACTION_GROUP(group), action_name, value);
}

static void unity_gtk_action_group_set_old_group(UnityGtkActionGroup *group,
                                                 GActionGroup *old_group)
{
	GActionGroup *old_old_group;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(group));

	old_old_group = group->old_group;

	if (old_group != old_old_group)
	{
		if (old_old_group != NULL)
		{
			char **names;

			g_signal_handlers_disconnect_by_data(old_old_group, group);

			names            = g_action_group_list_actions(old_old_group);
			group->old_group = NULL;
			g_object_unref(old_old_group);

			if (names != NULL)
			{
				char **i;

				for (i = names; *i != NULL; i++)
					g_action_group_action_removed(G_ACTION_GROUP(group), *i);

				g_strfreev(names);
			}
		}

		if (old_group != NULL)
		{
			char **names = g_action_group_list_actions(old_group);

			group->old_group = g_object_ref(old_group);
			g_signal_connect(old_group,
			                 "action-added",
			                 G_CALLBACK(
			                     unity_gtk_action_group_handle_group_action_added),
			                 group);
			g_signal_connect(old_group,
			                 "action-removed",
			                 G_CALLBACK(
			                     unity_gtk_action_group_handle_group_action_removed),
			                 group);
			g_signal_connect(
			    old_group,
			    "action-enabled-changed",
			    G_CALLBACK(unity_gtk_action_group_handle_group_action_enabled_changed),
			    group);
			g_signal_connect(
			    old_group,
			    "action-state-changed",
			    G_CALLBACK(unity_gtk_action_group_handle_group_action_state_changed),
			    group);

			if (names != NULL)
			{
				char **i;

				for (i = names; *i != NULL; i++)
					g_action_group_action_added(G_ACTION_GROUP(group), *i);

				g_strfreev(names);
			}
		}
	}
}

static void unity_gtk_action_group_dispose(GObject *object)
{
	UnityGtkActionGroup *group;
	GHashTable *actions_by_name;
	GHashTable *names_by_radio_menu_item;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(object));

	group                    = UNITY_GTK_ACTION_GROUP(object);
	actions_by_name          = group->actions_by_name;
	names_by_radio_menu_item = group->names_by_radio_menu_item;

	if (names_by_radio_menu_item != NULL)
	{
		group->names_by_radio_menu_item = NULL;
		g_hash_table_unref(names_by_radio_menu_item);
	}

	if (actions_by_name != NULL)
	{
		group->actions_by_name = NULL;
		g_hash_table_unref(actions_by_name);
	}

	unity_gtk_action_group_set_old_group(group, NULL);

	G_OBJECT_CLASS(unity_gtk_action_group_parent_class)->dispose(object);
}

static char **unity_gtk_action_group_list_actions(GActionGroup *action_group)
{
	UnityGtkActionGroup *group;

	g_return_val_if_fail(UNITY_GTK_IS_ACTION_GROUP(action_group), NULL);

	group = UNITY_GTK_ACTION_GROUP(action_group);

	if (group->actions_by_name != NULL)
	{
		char **names;
		char **new_names;
		GHashTableIter iter;
		gpointer key;
		guint n;
		guint i;

		names     = NULL;
		new_names = NULL;
		n         = g_hash_table_size(group->actions_by_name);

		if (group->old_group != NULL)
		{
			char **old_names = g_action_group_list_actions(group->old_group);

			if (old_names != NULL)
			{
				for (i = 0; old_names[i] != NULL; i++)
					;

				names     = g_malloc_n(i + n + 1, sizeof(char *));
				new_names = names + i;

				for (i = 0; old_names[i] != NULL; i++)
					names[i] = old_names[i];

				g_free(old_names);
			}
			else
				g_warn_if_reached();
		}

		if (names == NULL)
			new_names = names = g_malloc_n(n + 1, sizeof(char *));

		g_hash_table_iter_init(&iter, group->actions_by_name);
		for (i = 0; i < n && g_hash_table_iter_next(&iter, &key, NULL); i++)
			new_names[i] = g_strdup(key);

		new_names[i] = NULL;

		return names;
	}

	g_warn_if_reached();

	return group->old_group != NULL ? g_action_group_list_actions(group->old_group) : NULL;
}

static void unity_gtk_action_group_really_change_action_state(GActionGroup *action_group,
                                                              const char *name, GVariant *value)
{
	UnityGtkActionGroup *group;
	GHashTable *actions_by_name;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(action_group));

	group           = UNITY_GTK_ACTION_GROUP(action_group);
	actions_by_name = group->actions_by_name;

	if (actions_by_name != NULL)
	{
		UnityGtkAction *action = g_hash_table_lookup(actions_by_name, name);

		if (action != NULL)
		{
			if (g_strcmp0(name, action->name) == 0)
			{
				if (action->items_by_name != NULL)
				{
					if (value != NULL)
					{
						const char *name;
						UnityGtkMenuItem *item;

						g_return_if_fail(
						    g_variant_is_of_type(value,
						                         G_VARIANT_TYPE_STRING));

						name = g_variant_get_string(value, NULL);
						item = g_hash_table_lookup(action->items_by_name,
						                           name);

						if (item == NULL ||
						    !unity_gtk_menu_item_is_check(item))
						{
							g_warn_if_reached();

							value = NULL;
						}
						else
							gtk_check_menu_item_set_active(
							    GTK_CHECK_MENU_ITEM(item->menu_item),
							    TRUE);
					}

					if (value == NULL)
					{
						GHashTableIter iter;
						gpointer value;

						g_hash_table_iter_init(&iter,
						                       action->items_by_name);
						while (g_hash_table_iter_next(&iter, NULL, &value))
						{
							UnityGtkMenuItem *item = value;

							if (unity_gtk_menu_item_is_check(item))
								gtk_check_menu_item_set_active(
								    GTK_CHECK_MENU_ITEM(
								        item->menu_item),
								    FALSE);
						}
					}
				}
				else if (action->item != NULL &&
				         unity_gtk_menu_item_is_check(action->item))
				{
					g_return_if_fail(
					    value != NULL &&
					    g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN));

					gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
					                                   action->item->menu_item),
					                               g_variant_get_boolean(
					                                   value));
				}
				else
					g_warn_if_fail(value == NULL);

				return;
			}
			else if (g_strcmp0(name, action->subname) == 0)
			{
				GtkWidget *submenu;

				g_return_if_fail(
				    value != NULL &&
				    g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN));
				g_return_if_fail(action->item != NULL &&
				                 action->item->menu_item != NULL);

				submenu = gtk_menu_item_get_submenu(action->item->menu_item);

				g_return_if_fail(submenu != NULL);

				if (g_variant_get_boolean(value))
					g_signal_emit_by_name(submenu, "show");
				else
					g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
					                g_signal_emit_hide,
					                g_object_ref(submenu),
					                g_object_unref);

				return;
			}
			else
				g_warn_if_reached();
		}
		else
			g_warn_if_reached();
	}
	else
		g_warn_if_reached();

	if (group->old_group != NULL)
		g_action_group_change_action_state(group->old_group, name, value);
	else
		g_warn_if_reached();
}

static void unity_gtk_action_group_change_action_state(GActionGroup *action_group, const char *name,
                                                       GVariant *value)
{
	g_variant_ref_sink(value);
	unity_gtk_action_group_really_change_action_state(action_group, name, value);
	g_variant_unref(value);
}

static void unity_gtk_action_group_activate_action(GActionGroup *action_group, const char *name,
                                                   GVariant *parameter)
{
	UnityGtkActionGroup *group;
	GHashTable *actions_by_name;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(action_group));

	group           = UNITY_GTK_ACTION_GROUP(action_group);
	actions_by_name = group->actions_by_name;

	if (actions_by_name != NULL)
	{
		UnityGtkAction *action = g_hash_table_lookup(actions_by_name, name);

		if (action != NULL)
		{
			if (g_strcmp0(name, action->name) == 0)
			{
				if (action->items_by_name != NULL)
				{
					const char *name;
					UnityGtkMenuItem *item;

					g_return_if_fail(
					    parameter != NULL &&
					    g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING));

					name = g_variant_get_string(parameter, NULL);
					item = g_hash_table_lookup(action->items_by_name, name);

					if (item != NULL)
						unity_gtk_menu_item_activate(item);

					g_action_group_action_state_changed(G_ACTION_GROUP(group),
					                                    action->name,
					                                    parameter);
				}
				else if (action->item != NULL)
				{
					if (unity_gtk_menu_item_get_draw_as_radio(action->item))
						g_warn_if_fail(
						    g_variant_is_of_type(parameter,
						                         G_VARIANT_TYPE_STRING));
					else
						g_warn_if_fail(parameter == NULL);

					unity_gtk_menu_item_activate(action->item);
				}

				return;
			}
			else
				g_warn_if_reached();
		}
		else
			g_warn_if_reached();
	}
	else
		g_warn_if_reached();

	if (group->old_group != NULL)
		g_action_group_activate_action(group->old_group, name, parameter);
	else
		g_warn_if_reached();
}

static gboolean unity_gtk_action_group_query_action(GActionGroup *action_group, const char *name,
                                                    gboolean *enabled,
                                                    const GVariantType **parameter_type,
                                                    const GVariantType **state_type,
                                                    GVariant **state_hint, GVariant **state)
{
	UnityGtkActionGroup *group;
	GHashTable *actions_by_name;

	g_return_val_if_fail(UNITY_GTK_IS_ACTION_GROUP(action_group), FALSE);

	group           = UNITY_GTK_ACTION_GROUP(action_group);
	actions_by_name = group->actions_by_name;

	if (actions_by_name != NULL)
	{
		UnityGtkAction *action = g_hash_table_lookup(actions_by_name, name);

		if (action != NULL)
		{
			if (g_strcmp0(name, action->name) == 0)
			{
				if (enabled != NULL)
				{
					if (action->items_by_name != NULL)
					{
						GHashTableIter iter;
						gpointer value;

						*enabled = FALSE;

						g_hash_table_iter_init(&iter,
						                       action->items_by_name);
						while (!*enabled &&
						       g_hash_table_iter_next(&iter, NULL, &value))
							*enabled =
							    unity_gtk_menu_item_is_sensitive(value);
					}
					else
						*enabled =
						    action->item != NULL &&
						    unity_gtk_menu_item_is_sensitive(action->item);
				}

				if (parameter_type != NULL)
				{
					if (action->items_by_name != NULL ||
					    (action->item != NULL &&
					     unity_gtk_menu_item_get_draw_as_radio(action->item)))
						*parameter_type = G_VARIANT_TYPE_STRING;
					else
						*parameter_type = NULL;
				}

				if (state_type != NULL)
				{
					if (action->items_by_name != NULL ||
					    (action->item != NULL &&
					     unity_gtk_menu_item_get_draw_as_radio(action->item)))
						*state_type = G_VARIANT_TYPE_STRING;
					else if (action->item != NULL &&
					         unity_gtk_menu_item_is_check(action->item))
						*state_type = G_VARIANT_TYPE_BOOLEAN;
					else
						*state_type = NULL;
				}

				if (state_hint != NULL)
				{
					if (action->items_by_name != NULL)
					{
						GVariantBuilder builder;
						GHashTableIter iter;
						gpointer key;

						g_variant_builder_init(&builder,
						                       G_VARIANT_TYPE_ARRAY);

						g_hash_table_iter_init(&iter,
						                       action->items_by_name);
						while (g_hash_table_iter_next(&iter, &key, NULL))
							g_variant_builder_add(&builder, "s", key);

						*state_hint = g_variant_ref_sink(
						    g_variant_builder_end(&builder));
					}
					else if (action->item != NULL &&
					         unity_gtk_menu_item_is_check(action->item))
					{
						GVariantBuilder builder;

						if (unity_gtk_menu_item_get_draw_as_radio(
						        action->item))
						{
							g_variant_builder_init(
							    &builder, G_VARIANT_TYPE_ARRAY);
							g_variant_builder_add(&builder,
							                      "s",
							                      action->name);
							*state_hint = g_variant_ref_sink(
							    g_variant_builder_end(&builder));
						}
						else
						{
							g_variant_builder_init(
							    &builder, G_VARIANT_TYPE_TUPLE);
							g_variant_builder_add(&builder, "b", FALSE);
							g_variant_builder_add(&builder, "b", TRUE);
							*state_hint = g_variant_ref_sink(
							    g_variant_builder_end(&builder));
						}
					}
					else
						*state_hint = NULL;
				}

				if (state != NULL)
				{
					if (action->items_by_name != NULL)
					{
						GHashTableIter iter;
						gpointer key;
						gpointer value;

						*state = NULL;

						g_hash_table_iter_init(&iter,
						                       action->items_by_name);
						while (*state == NULL &&
						       g_hash_table_iter_next(&iter, &key, &value))
							if (unity_gtk_menu_item_is_active(value))
								*state = g_variant_ref_sink(
								    g_variant_new_string(key));
					}
					else if (action->item != NULL &&
					         unity_gtk_menu_item_is_check(action->item))
					{
						if (unity_gtk_menu_item_get_draw_as_radio(
						        action->item))
						{
							if (unity_gtk_menu_item_is_active(
							        action->item))
								*state = g_variant_ref_sink(
								    g_variant_new_string(
								        action->name));
							else
								*state = g_variant_ref_sink(
								    g_variant_new_string(""));
						}
						else
							*state = g_variant_ref_sink(
							    g_variant_new_boolean(
							        unity_gtk_menu_item_is_active(
							            action->item)));
					}
					else
						*state = NULL;
				}

				return TRUE;
			}
			else if (g_strcmp0(name, action->subname) == 0)
			{
				if (enabled != NULL)
					*enabled = TRUE;

				if (parameter_type != NULL)
					*parameter_type = NULL;

				if (state_type != NULL)
					*state_type = G_VARIANT_TYPE_BOOLEAN;

				if (state_hint != NULL)
				{
					GVariantBuilder builder;

					g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
					g_variant_builder_add(&builder, "b", FALSE);
					g_variant_builder_add(&builder, "b", TRUE);
					*state_hint =
					    g_variant_ref_sink(g_variant_builder_end(&builder));
				}

				if (state != NULL)
					*state = g_variant_ref_sink(g_variant_new_boolean(TRUE));

				return TRUE;
			}
			else
				g_warn_if_reached();
		}
	}
	else
		g_warn_if_reached();

	if (group->old_group != NULL)
		return g_action_group_query_action(group->old_group,
		                                   name,
		                                   enabled,
		                                   parameter_type,
		                                   state_type,
		                                   state_hint,
		                                   state);

	g_warn_if_reached();

	return FALSE;
}

static void unity_gtk_action_group_class_init(UnityGtkActionGroupClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = unity_gtk_action_group_dispose;
}

static void unity_gtk_action_group_action_group_init(GActionGroupInterface *iface)
{
	iface->list_actions        = unity_gtk_action_group_list_actions;
	iface->change_action_state = unity_gtk_action_group_change_action_state;
	iface->activate_action     = unity_gtk_action_group_activate_action;
	iface->query_action        = unity_gtk_action_group_query_action;
}

static void unity_gtk_action_group_init(UnityGtkActionGroup *self)
{
	self->actions_by_name =
	    g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);
	self->names_by_radio_menu_item =
	    g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
}

/**
 * unity_gtk_action_group_new:
 * @old_group: a fallback #GActionGroup.
 *
 * Creates a new #UnityGtkMenuShell based on the contents of the given
 * @menu_shell. Any subsequent changes to @menu_shell are reflected in
 * the returned #UnityGtkMenuShell.
 *
 * Actions not found in an attached #UnityGtkMenuShell are queried in
 * @old_group before failing.
 *
 * Returns: a new #UnityGtkActionGroup.
 */
UnityGtkActionGroup *unity_gtk_action_group_new(GActionGroup *old_group)
{
	UnityGtkActionGroup *group = g_object_new(UNITY_GTK_TYPE_ACTION_GROUP, NULL);

	unity_gtk_action_group_set_old_group(group, old_group);

	return group;
}

static char *g_strdup_normalize(const char *str)
{
	char *string = NULL;

	if (str != NULL)
	{
		guint i = 0;
		guint j;

		string = g_strdup(str);

		for (j = 0; str[j] != '\0'; j++)
		{
			if (g_ascii_isalnum(str[j]))
				string[i++] = str[j];
			else
				string[i++] = '-';
		}

		string[i] = '\0';
	}

	return string;
}

static char *unity_gtk_action_group_get_action_name(UnityGtkActionGroup *group,
                                                    UnityGtkMenuItem *item)
{
	GtkMenuItem *menu_item;
	const char *name;
	char *normalized_name;
	GHashTable *actions_by_name;
	GActionGroup *old_group;

	g_return_val_if_fail(UNITY_GTK_IS_ACTION_GROUP(group), NULL);
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), NULL);

	menu_item = item->menu_item;

	g_return_val_if_fail(menu_item != NULL, NULL);

	if (GTK_IS_RADIO_MENU_ITEM(menu_item))
	{
		GtkRadioMenuItem *radio_menu_item = GTK_RADIO_MENU_ITEM(menu_item);
		GSList *iter = g_slist_last(gtk_radio_menu_item_get_group(radio_menu_item));

		if (iter != NULL)
			menu_item = iter->data;
	}

	name = NULL;
#if !GTK_CHECK_VERSION(3, 10, 0)
	GtkAction *action = gtk_activatable_get_related_action(GTK_ACTIVATABLE(menu_item));

	if (action != NULL)
		name = gtk_action_get_name(action);
#endif
	if (name == NULL || name[0] == '\0')
		name = gtk_menu_item_get_label(menu_item);

	if (name == NULL || name[0] == '\0')
		name = gtk_menu_item_get_nth_label_label(menu_item, 0);

	if (name != NULL && name[0] == '\0')
		name = NULL;

	normalized_name = g_strdup_normalize(name);
	actions_by_name = group->actions_by_name;
	old_group       = group->old_group;

	if (normalized_name == NULL ||
	    (actions_by_name != NULL && g_hash_table_contains(actions_by_name, normalized_name)) ||
	    (old_group != NULL && g_action_group_has_action(old_group, normalized_name)))
	{
		char *next_normalized_name = NULL;
		guint i                    = 0;

		do
		{
			g_free(next_normalized_name);

			if (normalized_name != NULL)
				next_normalized_name =
				    g_strdup_printf("%s-%u", normalized_name, i++);
			else
				next_normalized_name = g_strdup_printf("%u", i++);
		} while ((actions_by_name != NULL &&
		          g_hash_table_contains(actions_by_name, next_normalized_name)) ||
		         (old_group != NULL &&
		          g_action_group_has_action(old_group, next_normalized_name)));

		g_free(normalized_name);
		normalized_name = next_normalized_name;
	}

	return normalized_name;
}

static char *unity_gtk_action_group_get_state_name(UnityGtkActionGroup *group,
                                                   UnityGtkMenuItem *item)
{
	char *name = NULL;

	g_return_val_if_fail(UNITY_GTK_IS_ACTION_GROUP(group), NULL);
	g_return_val_if_fail(UNITY_GTK_IS_MENU_ITEM(item), NULL);

	if (unity_gtk_menu_item_is_radio(item))
	{
		const char *label = unity_gtk_menu_item_get_label(item);

		if (label != NULL && label[0] != '\0')
		{
			char *normalized_label = g_strdup_normalize(label);
			UnityGtkAction *action = item->action;

			if (action != NULL)
			{
				if (action->items_by_name != NULL)
				{
					if (g_hash_table_contains(action->items_by_name,
					                          normalized_label))
					{
						guint i = 0;

						do
						{
							g_free(name);
							name = g_strdup_printf("%s-%u",
							                       normalized_label,
							                       i++);
						} while (
						    g_hash_table_contains(action->items_by_name,
						                          name));

						g_free(normalized_label);
					}
					else
						name = normalized_label;
				}
				else
				{
					g_warn_if_reached();
					name = normalized_label;
				}
			}
			else
				name = normalized_label;
		}
#if !GTK_CHECK_VERSION(3, 10, 0)
		else
		{
			GtkActivatable *activatable = GTK_ACTIVATABLE(item->menu_item);
			GtkAction *action = gtk_activatable_get_related_action(activatable);

			if (action != NULL)
			{
				GtkRadioAction *radio_action = GTK_RADIO_ACTION(action);
				const char *action_name      = gtk_action_get_name(action);
				char *normalized_action_name = NULL;
				gint value;

				g_object_get(radio_action, "value", &value, NULL);

				if (action_name != NULL && action_name[0] != '\0')
					normalized_action_name = g_strdup_normalize(action_name);

				if (normalized_action_name != NULL)
				{
					if (normalized_action_name[0] != '\0')
						name = g_strdup_printf("%s-%d",
						                       normalized_action_name,
						                       value);
					else
						name = g_strdup_printf("%d", value);

					g_free(normalized_action_name);
				}
				else
					name = g_strdup_printf("%d", value);

				if (item->action != NULL)
				{
					GHashTable *items_by_name = item->action->items_by_name;

					if (items_by_name != NULL &&
					    g_hash_table_contains(items_by_name, name))
					{
						char *next_name = NULL;
						guint i         = 0;

						do
						{
							g_free(next_name);
							next_name =
							    g_strdup_printf("%s-%u", name, i++);
						} while (g_hash_table_contains(items_by_name,
						                               next_name));

						g_free(name);
						name = next_name;
					}
				}
			}
		}
#endif

		if (name == NULL)
		{
			/*
			 * We tried to find a good name for this radio menu item state, but
			 * the application wasn't nice enough to give us one, either by label
			 * or by action. So we have to pick a bad name which is unique for
			 * any given UnityGtkMenuItem.
			 */

			name = g_strdup_printf("%p", item);
		}
	}

	return name;
}

void unity_gtk_action_group_connect_item(UnityGtkActionGroup *group, UnityGtkMenuItem *item)
{
	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(group));
	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(item));

	if (item->parent_shell != NULL &&
	    (item->parent_shell->action_group != group || item->action == NULL))
	{
		UnityGtkAction *new_action = NULL;
		UnityGtkAction *action     = NULL;

		if (item->action != NULL)
		{
			if (item->parent_shell->action_group != NULL)
				unity_gtk_action_group_disconnect_item(item->parent_shell
				                                           ->action_group,
				                                       item);
			else
				unity_gtk_menu_item_set_action(item, NULL);
		}

		if (unity_gtk_menu_item_is_radio(item))
		{
			GtkRadioMenuItem *radio_menu_item = GTK_RADIO_MENU_ITEM(item->menu_item);
			const char *action_name;
			char *state_name;

			g_return_if_fail(group->actions_by_name != NULL);
			g_return_if_fail(group->names_by_radio_menu_item != NULL);

			action_name =
			    g_hash_table_lookup(group->names_by_radio_menu_item, radio_menu_item);

			if (action_name == NULL)
			{
				GtkRadioMenuItem *last_radio_menu_item = NULL;
				GSList *iter = gtk_radio_menu_item_get_group(radio_menu_item);

				while (action_name == NULL && iter != NULL)
				{
					last_radio_menu_item = iter->data;
					action_name =
					    g_hash_table_lookup(group->names_by_radio_menu_item,
					                        last_radio_menu_item);
					iter = g_slist_next(iter);
				}

				if (action_name == NULL)
				{
					char *new_action_name =
					    unity_gtk_action_group_get_action_name(group, item);

					g_hash_table_insert(group->names_by_radio_menu_item,
					                    radio_menu_item,
					                    new_action_name);

					if (last_radio_menu_item != NULL &&
					    last_radio_menu_item != radio_menu_item)
						g_hash_table_insert(group->names_by_radio_menu_item,
						                    last_radio_menu_item,
						                    g_strdup(new_action_name));

					action_name = new_action_name;
				}
				else
					g_hash_table_insert(group->names_by_radio_menu_item,
					                    radio_menu_item,
					                    g_strdup(action_name));
			}

			action = g_hash_table_lookup(group->actions_by_name, action_name);

			if (action == NULL)
				action = new_action = unity_gtk_action_new_radio(action_name);

			state_name = unity_gtk_action_group_get_state_name(group, item);
			g_hash_table_insert(action->items_by_name, state_name, g_object_ref(item));
		}
		else if (!unity_gtk_menu_item_is_separator(item))
		{
			char *name = unity_gtk_action_group_get_action_name(group, item);
			action = new_action = unity_gtk_action_new(name, item);
			g_free(name);
		}

		unity_gtk_menu_item_set_action(item, action);

		if (new_action != NULL)
		{
			if (group->actions_by_name != NULL)
				g_hash_table_insert(group->actions_by_name,
				                    new_action->name,
				                    new_action);
			else
				g_warn_if_reached();

			g_action_group_action_added(G_ACTION_GROUP(group), new_action->name);

			/* Add a new submenu action so we can detect opening and closing. */
			if (item->menu_item != NULL &&
			    gtk_menu_item_get_submenu(item->menu_item) != NULL)
			{
				char *subname = unity_gtk_action_group_get_action_name(group, item);
				unity_gtk_action_set_subname(new_action, subname);
				g_free(subname);

				if (group->actions_by_name != NULL)
					g_hash_table_insert(group->actions_by_name,
					                    new_action->subname,
					                    g_object_ref(new_action));
				else
					g_warn_if_reached();

				g_action_group_action_added(G_ACTION_GROUP(group),
				                            new_action->subname);
			}
		}
	}
}

void unity_gtk_action_group_disconnect_item(UnityGtkActionGroup *group, UnityGtkMenuItem *item)
{
	UnityGtkAction *action;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(group));
	g_return_if_fail(UNITY_GTK_IS_MENU_ITEM(item));
	g_warn_if_fail(item->parent_shell != NULL);

	action = item->action;

	if (action != NULL)
	{
		if (action->items_by_name != NULL)
		{
			if (group->names_by_radio_menu_item != NULL)
			{
				const char *name = NULL;
				GHashTableIter iter;
				gpointer key;
				gpointer value;

				g_hash_table_iter_init(&iter, action->items_by_name);
				while (name == NULL && g_hash_table_iter_next(&iter, &key, &value))
					if (value == item)
						name = key;

				if (name != NULL)
				{
					g_hash_table_remove(action->items_by_name, name);

					if (group->names_by_radio_menu_item != NULL)
						g_hash_table_remove(group->names_by_radio_menu_item,
						                    item->menu_item);
					else
						g_warn_if_reached();

					if (g_hash_table_size(action->items_by_name) == 0)
					{
						/* Remove the submenu action used to detect opening
						 * and closing. */
						if (action->subname != NULL)
						{
							if (group->actions_by_name != NULL)
								g_hash_table_remove(
								    group->actions_by_name,
								    action->subname);
							else
								g_warn_if_reached();

							g_action_group_action_removed(
							    G_ACTION_GROUP(group), action->subname);
						}

						if (group->actions_by_name != NULL)
							g_hash_table_remove(group->actions_by_name,
							                    action->name);
						else
							g_warn_if_reached();

						g_action_group_action_removed(G_ACTION_GROUP(group),
						                              action->name);
					}
				}
				else
					g_warn_if_reached();
			}
			else
				g_warn_if_reached();
		}
		else
		{
			/* Remove the submenu action used to detect opening and closing. */
			if (action->subname != NULL)
			{
				if (group->actions_by_name != NULL)
					g_hash_table_remove(group->actions_by_name,
					                    action->subname);
				else
					g_warn_if_reached();

				g_action_group_action_removed(G_ACTION_GROUP(group),
				                              action->subname);
			}

			if (group->actions_by_name != NULL)
				g_hash_table_remove(group->actions_by_name, action->name);
			else
				g_warn_if_reached();

			g_action_group_action_removed(G_ACTION_GROUP(group), action->name);
		}
	}

	unity_gtk_menu_item_set_action(item, NULL);
}

/**
 * unity_gtk_action_group_connect_shell:
 * @group: a #UnityGtkActionGroup.
 * @shell: a #UnityGtkMenuShell.
 *
 * Creates actions for all menu items in @shell and adds them to @group.
 * Subsequent changes to @shell also affect @group.
 */
void unity_gtk_action_group_connect_shell(UnityGtkActionGroup *group, UnityGtkMenuShell *shell)
{
	GSequence *visible_indices;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(group));
	g_return_if_fail(UNITY_GTK_IS_MENU_SHELL(shell));

	visible_indices = shell->visible_indices;

	if (shell->action_group != NULL && shell->action_group != group)
		unity_gtk_action_group_disconnect_shell(shell->action_group, shell);

	if (visible_indices != NULL)
	{
		GSequenceIter *iter = g_sequence_get_begin_iter(visible_indices);

		while (!g_sequence_iter_is_end(iter))
		{
			guint i                = GPOINTER_TO_UINT(g_sequence_get(iter));
			UnityGtkMenuItem *item = g_ptr_array_index(shell->items, i);

			unity_gtk_action_group_connect_item(group, item);

			if (item->child_shell != NULL)
			{
				if (item->child_shell_valid)
					unity_gtk_action_group_connect_shell(group,
					                                     item->child_shell);
				else
					g_warn_if_reached();
			}

			iter = g_sequence_iter_next(iter);
		}
	}

	if (shell->action_group == NULL)
		shell->action_group = g_object_ref(group);
}

/**
 * unity_gtk_action_group_disconnect_shell:
 * @group: a #UnityGtkActionGroup.
 * @shell: a #UnityGtkMenuShell.
 *
 * Removes the actions for @shell from @group.
 */
void unity_gtk_action_group_disconnect_shell(UnityGtkActionGroup *group, UnityGtkMenuShell *shell)
{
	UnityGtkActionGroup *action_group;
	GSequence *visible_indices;

	g_return_if_fail(UNITY_GTK_IS_ACTION_GROUP(group));
	g_return_if_fail(UNITY_GTK_IS_MENU_SHELL(shell));
	g_warn_if_fail(shell->action_group == NULL || shell->action_group == group);

	visible_indices = shell->visible_indices;

	if (visible_indices != NULL)
	{
		GSequenceIter *iter = g_sequence_get_begin_iter(visible_indices);

		while (!g_sequence_iter_is_end(iter))
		{
			guint i                = GPOINTER_TO_UINT(g_sequence_get(iter));
			UnityGtkMenuItem *item = g_ptr_array_index(shell->items, i);

			unity_gtk_action_group_disconnect_item(group, item);

			if (item->child_shell != NULL)
			{
				if (item->child_shell_valid)
					unity_gtk_action_group_disconnect_shell(group,
					                                        item->child_shell);
				else
					g_warn_if_reached();
			}

			iter = g_sequence_iter_next(iter);
		}
	}

	action_group = shell->action_group;

	if (action_group != NULL)
	{
		shell->action_group = NULL;
		g_object_unref(action_group);
	}
}

void unity_gtk_action_group_print(UnityGtkActionGroup *group, guint indent)
{
	char *space;

	g_return_if_fail(group == NULL || UNITY_GTK_IS_ACTION_GROUP(group));

	space = g_strnfill(indent, ' ');

	if (group != NULL)
	{
		g_print("%s(%s *) %p\n",
		        space,
		        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(group)),
		        group);

		if (group->actions_by_name != NULL)
		{
			GHashTableIter iter;
			gpointer key;
			gpointer value;

			g_hash_table_iter_init(&iter, group->actions_by_name);
			while (g_hash_table_iter_next(&iter, &key, &value))
			{
				g_print("%s  \"%s\" ->\n", space, (const char *)key);
				unity_gtk_action_print(value, indent + 4);
			}
		}

		if (group->names_by_radio_menu_item != NULL)
		{
			GHashTableIter iter;
			gpointer key;
			gpointer value;

			g_hash_table_iter_init(&iter, group->names_by_radio_menu_item);
			while (g_hash_table_iter_next(&iter, &key, &value))
				g_print("%s  (%s *) %p -> \"%s\"\n",
				        space,
				        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(key)),
				        key,
				        (const char *)value);
		}
	}
	else
		g_print("%sNULL\n", space);

	g_free(space);
}

gboolean unity_gtk_action_group_is_debug(void)
{
	return unity_gtk_action_group_debug;
}

/**
 * unity_gtk_action_group_set_debug:
 * @debug: #TRUE to enable debugging output
 *
 * Sets if action group changes should be logged using g_print ().
 */
void unity_gtk_action_group_set_debug(gboolean debug)
{
	unity_gtk_action_group_debug = debug;
}
