#include <stdbool.h>
#include <stdio.h>
#include <utils.h>

#include "definitions.h"
#include "model.h"

static void activate_ordinary_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	DBusMenuXml *xml = DBUS_MENU_XML(user_data);
	u_int32_t id;
	sscanf(g_action_get_name(G_ACTION(action)), ACTION_PREFIX "%u", &id);
	// use CURRENT_TIME instead of gtk_get_current_event_time to avoid linking to GTK.
	dbus_menu_xml_call_event_sync(xml,
	                              id,
	                              "clicked",
	                              g_variant_new("v", g_variant_new_int32(0)),
	                              CURRENT_TIME,
	                              NULL,
	                              NULL);
}

static void activate_checkbox_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	DBusMenuXml *xml = DBUS_MENU_XML(user_data);
	u_int32_t id;
	sscanf(g_action_get_name(G_ACTION(action)), ACTION_PREFIX "%u", &id);
	g_autoptr(GVariant) state = g_action_get_state(G_ACTION(action));
	// use CURRENT_TIME instead of gtk_get_current_event_time to avoid linking to GTK.
	dbus_menu_xml_call_event_sync(xml,
	                              id,
	                              "clicked",
	                              g_variant_new("v", g_variant_new_int32(0)),
	                              CURRENT_TIME,
	                              NULL,
	                              NULL);
	g_action_change_state(G_ACTION(action),
	                      g_variant_new_boolean(!g_variant_get_boolean(state)));
}

static void state_radio_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	DBusMenuXml *xml   = DBUS_MENU_XML(user_data);
	const char *id_str = g_action_get_name(G_ACTION(action));
	uint id;
	sscanf(id_str, ACTION_PREFIX "%u", &id);
	// use CURRENT_TIME instead of gtk_get_current_event_time to avoid linking to GTK.
	dbus_menu_xml_call_event_sync(xml,
	                              id,
	                              "clicked",
	                              g_variant_new("v", g_variant_new_int32(0)),
	                              CURRENT_TIME,
	                              NULL,
	                              NULL);
	g_simple_action_set_state(action, parameter);
}

static GAction *dbus_menu_action_new(DBusMenuXml *xml, u_int32_t id, DBusMenuActionType action_type)
{
	GSimpleAction *ret;
	g_autofree char *name = g_strdup_printf(ACTION_PREFIX "%u", id);
	if (action_type == DBUS_MENU_ACTION_CHECKMARK)
	{
		ret = g_simple_action_new_stateful(name, NULL, g_variant_new_boolean(false));
		g_signal_connect(ret, "activate", G_CALLBACK(activate_checkbox_cb), xml);
		return ret;
	}
	else if (action_type == DBUS_MENU_ACTION_RADIO)
	{
		ret = g_simple_action_new_stateful(name,
		                                   G_VARIANT_TYPE_STRING,
		                                   g_variant_new_string(
		                                       DBUS_MENU_ACTION_RADIO_UNSELECTED));
		g_signal_connect(ret, "activate", G_CALLBACK(state_radio_cb), xml);
		return ret;
	}
	else if (action_type == DBUS_MENU_ACTION_NORMAL)
	{
		ret = g_simple_action_new(name, NULL);
		g_signal_connect(ret, "activate", G_CALLBACK(activate_ordinary_cb), xml);
		return ret;
	}
	g_assert_not_reached();
}

static bool source_state_false(gpointer *data)
{
	GSimpleAction *submenu = G_SIMPLE_ACTION(data);
	g_simple_action_set_state(submenu, g_variant_new_boolean(false));
	return true;
}

static void state_submenu_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	g_return_if_fail(DBUS_MENU_IS_MODEL(user_data));
	DBusMenuModel *model = DBUS_MENU_MODEL(user_data);
	DBusMenuXml *xml;
	u_int32_t id;
	g_object_get(model, "parent-id", &id, "xml", &xml, NULL);
	bool request_open = g_variant_get_boolean(parameter);
	bool opened       = g_variant_get_boolean(g_action_get_state(action));
	bool need_update  = true;
	if (request_open && !opened)
	{
		dbus_menu_xml_call_about_to_show_sync(xml,
		                                      id,
		                                      (gboolean *)&need_update,
		                                      NULL,
		                                      NULL);
		const char *populated = (const char *)g_object_get_data(action, POPULATED_QUARK);
		if (populated == NULL || g_menu_model_get_n_items(model) == 0)
			need_update = true;
		need_update = need_update || dbus_menu_model_is_layout_update_required(model);
		if (need_update)
		{
			g_object_set_data(action, POPULATED_QUARK, POPULATED_QUARK);
			// TODD: Populate layout after request;
			if (DBUS_MENU_IS_MODEL(model))
				dbus_menu_model_update_layout(model);
		}
		dbus_menu_xml_call_event_sync(xml,
		                              id,
		                              "opened",
		                              g_variant_new("v", g_variant_new_int32(0)),
		                              CURRENT_TIME,
		                              NULL,
		                              NULL);
		g_simple_action_set_state(action, g_variant_new_boolean(true));
		//         TODO: change state to false after menu closing, not by time
		//                g_timeout_add(500, (GSourceFunc)source_state_false, action);
	}
	else if (request_open)
	{
		g_simple_action_set_state(action, g_variant_new_boolean(true));
		need_update = dbus_menu_model_is_layout_update_required(model);
		if (need_update)
		{
			// TODD: Populate layout after request;
			if (DBUS_MENU_IS_MODEL(model))
				dbus_menu_model_update_layout(model);
		}
	}
	else
	{
		dbus_menu_xml_call_event_sync(xml,
		                              id,
		                              "closed",
		                              g_variant_new("v", g_variant_new_int32(0)),
		                              CURRENT_TIME,
		                              NULL,
		                              NULL);
		g_simple_action_set_state(action, g_variant_new_boolean(false));
	}
}

static GAction *dbus_menu_submenu_action_new(DBusMenuModel *model)
{
	uint id;
	g_object_get(model, "parent-id", &id, NULL);
	g_autofree char *name = g_strdup_printf(SUBMENU_PREFIX "%u", id);
	GSimpleAction *ret    = g_simple_action_new_stateful(name,
                                                          G_VARIANT_TYPE_BOOLEAN,
                                                          g_variant_new_boolean(false));
	g_signal_connect(ret, "change-state", G_CALLBACK(state_submenu_cb), model);
	return ret;
}

G_GNUC_INTERNAL char *dbus_menu_action_get_name(uint id, DBusMenuActionType action_type,
                                                bool use_prefix)
{
	return g_strdup_printf("%s%s%u",
	                       use_prefix ? DBUS_MENU_ACTION_NAMESPACE_PREFIX : "",
	                       action_type == DBUS_MENU_ACTION_SUBMENU ? SUBMENU_PREFIX
	                                                               : ACTION_PREFIX,
	                       id);
}

G_GNUC_INTERNAL void dbus_menu_action_replace_signals(GAction *action, DBusMenuXml *xml,
                                                      DBusMenuModel *submenu,
                                                      DBusMenuActionType action_type)
{
	g_print("Action %s is replacing signals\n", g_action_get_name(action));
	if (action_type == DBUS_MENU_ACTION_SUBMENU)
	{
		g_signal_handlers_disconnect_by_func_only(action, state_submenu_cb);
		g_signal_connect(action, "change-state", G_CALLBACK(state_submenu_cb), submenu);
	}
	else if (action_type == DBUS_MENU_ACTION_RADIO)
	{
		g_signal_handlers_disconnect_by_func_only(action, state_radio_cb);
		g_signal_connect(action, "activate", G_CALLBACK(state_radio_cb), xml);
	}
	else if (action_type == DBUS_MENU_ACTION_CHECKMARK)
	{
		g_signal_handlers_disconnect_by_func_only(action, activate_checkbox_cb);
		g_signal_connect(action, "activate", G_CALLBACK(activate_checkbox_cb), xml);
	}
	else
	{
		g_signal_handlers_disconnect_by_func_only(action, activate_ordinary_cb);
		g_signal_connect(action, "activate", G_CALLBACK(activate_ordinary_cb), xml);
	}
}

G_GNUC_INTERNAL GAction *dbus_menu_action_reference(u_int32_t id, DBusMenuXml *xml,
                                                    DBusMenuModel *submenu,
                                                    GActionMap *action_group,
                                                    DBusMenuActionType action_type)
{
	bool is_submenu           = submenu != NULL;
	const char *action_prefix = is_submenu ? SUBMENU_PREFIX : ACTION_PREFIX;
	g_autofree char *name     = g_strdup_printf("%s%u", action_prefix, id);
	GAction *ret              = g_action_map_lookup_action(action_group, name);
	bool check_parameter      = false;
	if (ret)
	{
		g_object_ref(ret);
		const GVariantType *state_type = g_action_get_state_type(ret);
		if (state_type == NULL)
			check_parameter = check_parameter || action_type == DBUS_MENU_ACTION_NORMAL;
		else if (is_submenu)
		{
			check_parameter = check_parameter ||
			                  g_variant_type_equal(state_type, G_VARIANT_TYPE_BOOLEAN);
		}
		else
		{
			check_parameter = check_parameter ||
			                  (action_type == DBUS_MENU_ACTION_RADIO &&
			                   g_variant_type_equal(state_type, G_VARIANT_TYPE_STRING));
			check_parameter =
			    check_parameter ||
			    (action_type == DBUS_MENU_ACTION_CHECKMARK &&
			     g_variant_type_equal(state_type, G_VARIANT_TYPE_BOOLEAN));
		}
		if (check_parameter)
			dbus_menu_action_replace_signals(ret, xml, submenu, action_type);
		else
			g_action_map_remove_action(action_group, name);
	}
	if (ret == NULL || !check_parameter)
	{
		if (is_submenu)
			ret = dbus_menu_submenu_action_new(submenu);
		else
			ret = dbus_menu_action_new(xml, id, action_type);
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
