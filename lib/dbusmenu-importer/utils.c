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

G_GNUC_INTERNAL GAction *dbus_menu_action_new(DBusMenuXml *xml, u_int32_t id,
                                              const char *action_type)
{
	GSimpleAction *ret;
	if (!g_strcmp0(action_type, DBUS_MENU_TOGGLE_TYPE_CHECK))
	{
		g_autofree char *name = g_strdup_printf(ACTION_PREFIX "%u", id);
		ret = g_simple_action_new_stateful(name, NULL, g_variant_new_boolean(false));
		unsigned long handler =
		    g_signal_connect(ret, "activate", G_CALLBACK(activate_checkbox_cb), xml);
		g_object_set_data(ret, ACTIVATE_ID_QUARK_STR, GUINT_TO_POINTER(handler));
		return ret;
	}
	else if (!g_strcmp0(action_type, DBUS_MENU_TOGGLE_TYPE_RADIO))
	{
		g_autofree char *name = g_strdup_printf(ACTION_PREFIX "%u", id);
		ret                   = g_simple_action_new_stateful(name,
                                                   G_VARIANT_TYPE_STRING,
                                                   g_variant_new_string(
                                                       DBUS_MENU_ACTION_RADIO_UNSELECTED));
		unsigned long handler =
		    g_signal_connect(ret, "activate", G_CALLBACK(state_radio_cb), xml);
		g_object_set_data(ret, ACTIVATE_ID_QUARK_STR, GUINT_TO_POINTER(handler));
		return ret;
	}
	else
	{
		g_autofree char *name = g_strdup_printf(ACTION_PREFIX "%u", id);
		ret                   = g_simple_action_new(name, NULL);
		g_signal_connect(ret, "activate", G_CALLBACK(activate_ordinary_cb), xml);
		return ret;
	}
	g_assert_not_reached();
}

bool source_state_false(gpointer *data)
{
	GSimpleAction *submenu = G_SIMPLE_ACTION(data);
	g_simple_action_set_state(submenu, g_variant_new_boolean(false));
	return true;
}

static void state_submenu_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	DBusMenuModel *model = DBUS_MENU_MODEL(user_data);
	DBusMenuXml *xml;
	u_int32_t id;
	g_object_get(model, "parent-id", &id, "xml", &xml, NULL);
	bool request_open = g_variant_get_boolean(parameter);
	bool need_update  = true;
	if (request_open)
	{
        dbus_menu_xml_call_about_to_show_sync(xml, id, (gboolean *)&need_update, NULL, NULL);
		const char *populated = (const char *)g_object_get_data(action, POPULATED_QUARK);
        if (populated == NULL)
			need_update = true;
		if (need_update)
		{
			g_object_set_data(action, POPULATED_QUARK, POPULATED_QUARK);
			// TODD: Populate layout after request;
			if (DBUS_MENU_IS_MODEL(model))
				dbus_menu_model_update_layout(model);
		}
		g_simple_action_set_state(action, g_variant_new_boolean(true));
        dbus_menu_xml_call_event_sync(xml,
                                      id,
                                      "opened",
                                      g_variant_new("v", g_variant_new_int32(0)),
                                      CURRENT_TIME,
                                      NULL,
                                      NULL);
		// TODO: change state to false after menu closing, not by time
		//		g_timeout_add(500, (GSourceFunc)source_state_false, action);
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
	}
}

G_GNUC_INTERNAL GAction *dbus_menu_submenu_action_new(DBusMenuModel *model)
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
