#include <stdbool.h>
#include <stdio.h>
#include <utils.h>

#include "definitions.h"

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

bool source_state_false(gpointer *data)
{
	GSimpleAction *submenu = G_SIMPLE_ACTION(data);
	g_simple_action_set_state(submenu, g_variant_new_boolean(false));
	return true;
}

static void state_submenu_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	DBusMenuXml *xml = DBUS_MENU_XML(user_data);
	u_int32_t id;
	bool request_open = g_variant_get_boolean(parameter);
	bool need_update  = true;
	sscanf(g_action_get_name(G_ACTION(action)), SUBMENU_PREFIX "%u", &id);
	if (request_open)
	{
		dbus_menu_xml_call_event_sync(xml,
		                              id,
		                              "opened",
		                              g_variant_new("v", g_variant_new_int32(0)),
		                              CURRENT_TIME,
		                              NULL,
		                              NULL);
		dbus_menu_xml_call_about_to_show_sync(xml, 0, (gboolean *)&need_update, NULL, NULL);
		if (need_update)
		{
			// TODD: Populate layout after request;
		}
		g_simple_action_set_state(action, g_variant_new_boolean(true));
		// TODO: change state to false after menu closing, not by time
		g_timeout_add(500, (GSourceFunc)source_state_false, action);
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

static void activate_radio_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	DBusMenuXml *xml = DBUS_MENU_XML(user_data);
	u_int32_t id     = g_variant_get_uint32(parameter);
	// use CURRENT_TIME instead of gtk_get_current_event_time to avoid linking to GTK.
	dbus_menu_xml_call_event_sync(xml,
	                              id,
	                              "clicked",
	                              g_variant_new("v", g_variant_new_int32(0)),
	                              CURRENT_TIME,
	                              NULL,
	                              NULL);
	g_action_change_state(G_ACTION(action), parameter);
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
		    g_signal_connect(ret, "activated", G_CALLBACK(activate_checkbox_cb), xml);
		g_object_set_data(ret, ACTIVATE_ID_QUARK_STR, GUINT_TO_POINTER(handler));
	}
	else if (!g_strcmp0(action_type, DBUS_MENU_TOGGLE_TYPE_RADIO))
	{
		g_autofree char *name = g_strdup_printf(RADIO_ACTION_PREFIX "%u", id);
		ret                   = g_simple_action_new_stateful(name,
                                                   G_VARIANT_TYPE_UINT32,
                                                   g_variant_new_uint32(0));
		unsigned long handler =
		    g_signal_connect(ret, "activated", G_CALLBACK(activate_radio_cb), xml);
		g_object_set_data(ret, ACTIVATE_ID_QUARK_STR, GUINT_TO_POINTER(handler));
	}
	else if (!g_strcmp0(action_type, DBUS_MENU_CHILDREN_DISPLAY_SUBMENU))
	{
		g_autofree char *name = g_strdup_printf(SUBMENU_PREFIX "%u", id);
		ret = g_simple_action_new_stateful(name, NULL, g_variant_new_boolean(false));
		g_signal_connect(ret, "state-changed", G_CALLBACK(state_submenu_cb), xml);
	}
	else
	{
		g_autofree char *name = g_strdup_printf(ACTION_PREFIX "%u", id);
		ret                   = g_simple_action_new(name, NULL);
		g_signal_connect(ret, "activated", G_CALLBACK(activate_ordinary_cb), xml);
	}
	g_assert_not_reached();
}
