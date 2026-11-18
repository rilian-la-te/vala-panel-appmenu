/*
 * vala-panel-appmenu
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
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
 */

#include <stdbool.h>
#include <stdio.h>
#include <utils.h>

#include "definitions.h"
#include "model.h"
#include "item.h"

static DBusMenuItem* dbus_menu_item_get_from_action_name(DBusMenuModel* parent, const char* name);

static void ordinary_send_activated_event(GAction *action, gpointer user_data)
{
	DBusMenuModel *parent = DBUS_MENU_MODEL(user_data);
	g_autoptr(DBusMenuXml) xml = NULL;
	g_object_get(parent, "xml", &xml, NULL);
	DBusMenuItem* item = dbus_menu_item_get_from_action_name(parent,g_action_get_name(action));
	if(!xml || !item)
		return;
	// use CURRENT_TIME instead of gtk_get_current_event_time to avoid linking to GTK.
	/* Async clicked now. Just fire-and-forget */
	dbus_menu_xml_call_event(xml,
	                         item->id,
	                         DBUS_MENU_EVENT_ACTIVATED,
	                         g_variant_new("v", g_variant_new_int32(0)),
	                         CURRENT_TIME,
	                         NULL,
	                         NULL,
	                         NULL);
}

static void activate_ordinary_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	ordinary_send_activated_event(G_ACTION(action), user_data);
}

static void activate_checkbox_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	g_autoptr(GVariant) state = g_action_get_state(G_ACTION(action));
	ordinary_send_activated_event(G_ACTION(action), user_data);
	g_action_change_state(G_ACTION(action),
	                      g_variant_new_boolean(!g_variant_get_boolean(state)));
}

static void state_radio_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	ordinary_send_activated_event(G_ACTION(action), user_data);
	g_simple_action_set_state(action, parameter);
}

static GAction *dbus_menu_action_new(DBusMenuItem* item, DBusMenuModel* parent)
{
	GSimpleAction *ret;
	g_autofree char *name = dbus_menu_action_name(parent, item, false, false);
	if (item->action_type == DBUS_MENU_ACTION_CHECKMARK)
	{
		ret = g_simple_action_new_stateful(name, NULL, g_variant_new_boolean(false));
		g_signal_connect_object(ret, "activate", G_CALLBACK(activate_checkbox_cb), parent, G_CONNECT_DEFAULT);
		return G_ACTION(ret);
	}
	else if (item->action_type == DBUS_MENU_ACTION_RADIO)
	{
		ret = g_simple_action_new_stateful(name,
		                                   G_VARIANT_TYPE_STRING,
		                                   g_variant_new_string(
		                                       DBUS_MENU_ACTION_RADIO_UNSELECTED));
		g_signal_connect_object(ret, "activate", G_CALLBACK(state_radio_cb), parent, G_CONNECT_DEFAULT);
		return G_ACTION(ret);
	}
	else if (item->action_type == DBUS_MENU_ACTION_NORMAL)
	{
		ret = g_simple_action_new(name, NULL);
		g_signal_connect_object(ret, "activate", G_CALLBACK(activate_ordinary_cb), parent, G_CONNECT_DEFAULT);
		return G_ACTION(ret);
	}
	g_assert_not_reached();
}

static void state_submenu_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	g_return_if_fail(DBUS_MENU_IS_MODEL(user_data));
	DBusMenuModel *parent = DBUS_MENU_MODEL(user_data);
	DBusMenuXml *xml;
	g_object_get(parent,  "xml", &xml, NULL);
	DBusMenuItem* item = dbus_menu_item_get_from_action_name(parent,g_action_get_name(G_ACTION(action)));
	DBusMenuModel* model = dbus_menu_item_get_submenu(item);
	bool request_open = g_variant_get_boolean(parameter);
	GVariant *statev  = g_action_get_state(G_ACTION(action));
	bool opened       = g_variant_get_boolean(statev);
	g_variant_unref(statev);
	if(!xml || !item || !model)
		return;
	if (request_open)
	{
		if(!opened)
			dbus_menu_model_load(model, action);
		else 
			g_simple_action_set_state(action, g_variant_new_boolean(true));
	}
	else
	{
		/* Just fire and forget closed */
		dbus_menu_xml_call_event(xml,
								item->id,
								DBUS_MENU_EVENT_CLOSED,
								g_variant_new("v", g_variant_new_int32(0)),
								CURRENT_TIME,
								NULL,
								NULL,
								NULL);
		g_simple_action_set_state(action, g_variant_new_boolean(false));
	}
	g_object_unref(xml);
}

G_GNUC_INTERNAL GAction *dbus_menu_submenu_action_new(DBusMenuItem* item, DBusMenuModel *parent)
{
	if (!item || !parent)
        return NULL;
	g_autofree char *name = dbus_menu_action_name(parent, item, true, false);
	GSimpleAction *ret    = g_simple_action_new_stateful(name,
                                                          G_VARIANT_TYPE_BOOLEAN,
                                                          g_variant_new_boolean(false));
	g_signal_connect_object(ret, "change-state", G_CALLBACK(state_submenu_cb), parent, G_CONNECT_DEFAULT);
	return G_ACTION(ret);
}

G_GNUC_INTERNAL char *dbus_menu_action_name(DBusMenuModel *parent, DBusMenuItem *item,
                                            bool for_submenu, bool use_prefix)
{
	return g_strdup_printf("%s%s%p-%u-%u",
	                       use_prefix ? DBUS_MENU_ACTION_NAMESPACE_PREFIX : "",
	                       for_submenu ? SUBMENU_PREFIX : ORDINARY_PREFIX,
	                       parent,
	                       item->section_num,
	                       item->place);
}

static bool dbus_menu_parameters_from_name(const char* name, uint16_t* section_num, int16_t* place) {
    gint vals[2];
    const gchar *end = name + strlen(name);
    const gchar *p = end;

    for (int i = 1; i >= 0; i--) {
        while (p > name && *(p - 1) != '-') {
            p--;
        }
        if (p == name) return false;

        gchar *parsed_end = NULL;
        vals[i] = (gint)g_ascii_strtoll(p, &parsed_end, 10);
        if (parsed_end != end) return false;

        p--; 
        end = p;
    }

    *section_num = vals[0];
    *place = vals[1];
    return true;
}

static DBusMenuItem* dbus_menu_item_get_from_action_name(DBusMenuModel* parent, const char* name) {
	uint16_t section_num;
	int16_t place;
	bool ret = dbus_menu_parameters_from_name(name, &section_num, &place);
	if(!ret)
		return NULL;
	return dbus_menu_model_find_item(parent, section_num, place);
}

G_GNUC_INTERNAL void dbus_menu_action_replace_signals(GAction *action, DBusMenuItem* item, DBusMenuModel* parent)
{
	if (item->action_type == DBUS_MENU_ACTION_SUBMENU)
	{
		g_signal_handlers_disconnect_by_func_only(action, state_submenu_cb);
		g_signal_connect_object(action, "change-state", G_CALLBACK(state_submenu_cb), parent, G_CONNECT_DEFAULT);
	}
	else if (item->action_type == DBUS_MENU_ACTION_RADIO)
	{
		g_signal_handlers_disconnect_by_func_only(action, state_radio_cb);
		g_signal_connect_object(action, "activate", G_CALLBACK(state_radio_cb), parent, G_CONNECT_DEFAULT);
	}
	else if (item->action_type == DBUS_MENU_ACTION_CHECKMARK)
	{
		g_signal_handlers_disconnect_by_func_only(action, activate_checkbox_cb);
		g_signal_connect_object(action, "activate", G_CALLBACK(activate_checkbox_cb), parent, G_CONNECT_DEFAULT);
	}
	else
	{
		g_signal_handlers_disconnect_by_func_only(action, activate_ordinary_cb);
		g_signal_connect_object(action, "activate", G_CALLBACK(activate_ordinary_cb), parent, G_CONNECT_DEFAULT);
	}
}

G_GNUC_INTERNAL GAction *dbus_menu_action_reference(DBusMenuModel *parent, DBusMenuItem* item)
{
	bool is_submenu           = dbus_menu_item_is_submenu(item);
	GActionMap* action_group  = G_ACTION_MAP(item->ref_action_group);
	g_autofree char *name     = dbus_menu_action_name(parent, item, is_submenu, false);
	GAction *ret              = g_action_map_lookup_action(action_group, name);
	DBusMenuActionType action_type = item->action_type;
	bool check_parameter      = false;
	if (ret)
	{
		g_object_ref(ret);
		const GVariantType *state_type = g_action_get_state_type(ret);
		if (state_type == NULL)
			check_parameter |= action_type == DBUS_MENU_ACTION_NORMAL;
		else if (is_submenu)
		{
			check_parameter |= g_variant_type_equal(state_type, G_VARIANT_TYPE_BOOLEAN);
		}
		else
		{
			check_parameter |=  (action_type == DBUS_MENU_ACTION_RADIO &&
			                   g_variant_type_equal(state_type, G_VARIANT_TYPE_STRING));
			check_parameter |=
			    (action_type == DBUS_MENU_ACTION_CHECKMARK &&
			     g_variant_type_equal(state_type, G_VARIANT_TYPE_BOOLEAN));
		}
		if (check_parameter)
			dbus_menu_action_replace_signals(ret, item, parent);
		else {
			g_action_map_remove_action(action_group, name);
			g_clear_object(&ret);
		}
	}
	if (ret == NULL || !check_parameter)
	{
		if (is_submenu)
			ret = dbus_menu_submenu_action_new(item,parent);
		else
			ret = dbus_menu_action_new(item, parent);
		g_action_map_add_action(G_ACTION_MAP(action_group), ret);
	}
	return ret;
}

G_GNUC_INTERNAL void dbus_menu_action_lock(GAction *action)
{
	g_signal_handlers_block_by_func_only(action, activate_checkbox_cb);
	g_signal_handlers_block_by_func_only(action, state_radio_cb);
}

G_GNUC_INTERNAL void dbus_menu_action_unlock(GAction *action)
{
	g_signal_handlers_unblock_by_func_only(action, activate_checkbox_cb);
	g_signal_handlers_unblock_by_func_only(action, state_radio_cb);
}
