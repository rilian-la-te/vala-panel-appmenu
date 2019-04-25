/*
 * appmenu-gtk-module
 * Copyright 2012 Canonical Ltd.
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
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
 *
 * Authors: Ryan Lortie <desrt@desrt.ca>
 *          William Hua <william.hua@canonical.com>
 *          Konstantin Pugin <ria.freelander@gmail.com>
 *          Lester Carballo Perez <lestcape@gmail.com>
 */

#include "platform.h"
#include "consts.h"
#include "datastructs-private.h"
#include "datastructs.h"

#ifdef GDK_WINDOWING_X11
G_GNUC_INTERNAL char *gtk_widget_get_x11_property_string(GtkWidget *widget, const char *name)
{
	GdkWindow *window;
	GdkDisplay *display;
	Display *xdisplay;
	Window xwindow;
	Atom property;
	Atom actual_type;
	int actual_format;
	unsigned long nitems;
	unsigned long bytes_after;
	unsigned char *prop;

	g_return_val_if_fail(GTK_IS_WIDGET(widget), NULL);

	window   = gtk_widget_get_window(widget);
	display  = gdk_window_get_display(window);
	xdisplay = GDK_DISPLAY_XDISPLAY(display);
	xwindow  = GDK_WINDOW_XID(window);

	property = None;

	if (display != NULL)
		property = gdk_x11_get_xatom_by_name_for_display(display, name);

	if (property == None)
		property = gdk_x11_get_xatom_by_name(name);

	g_return_val_if_fail(property != None, NULL);

	if (XGetWindowProperty(xdisplay,
	                       xwindow,
	                       property,
	                       0,
	                       G_MAXLONG,
	                       False,
	                       AnyPropertyType,
	                       &actual_type,
	                       &actual_format,
	                       &nitems,
	                       &bytes_after,
	                       &prop) == Success)
	{
		if (actual_format)
		{
			char *string = g_strdup((const char *)prop);

			if (prop != NULL)
				XFree(prop);

			return string;
		}
		else
			return NULL;
	}

	return NULL;
}

G_GNUC_INTERNAL void gtk_widget_set_x11_property_string(GtkWidget *widget, const char *name,
                                                        const char *value)
{
	GdkWindow *window;
	GdkDisplay *display;
	Display *xdisplay;
	Window xwindow;
	Atom property;
	Atom type;

	g_return_if_fail(GTK_IS_WIDGET(widget));

	window   = gtk_widget_get_window(widget);
	display  = gdk_window_get_display(window);
	xdisplay = GDK_DISPLAY_XDISPLAY(display);
	xwindow  = GDK_WINDOW_XID(window);

	property = None;

	if (display != NULL)
		property = gdk_x11_get_xatom_by_name_for_display(display, name);

	if (property == None)
		property = gdk_x11_get_xatom_by_name(name);

	g_return_if_fail(property != None);

	type = None;

	if (display != NULL)
		type = gdk_x11_get_xatom_by_name_for_display(display, "UTF8_STRING");

	if (type == None)
		type = gdk_x11_get_xatom_by_name("UTF8_STRING");

	g_return_if_fail(type != None);

	if (value != NULL)
		XChangeProperty(xdisplay,
		                xwindow,
		                property,
		                type,
		                8,
		                PropModeReplace,
		                (unsigned char *)value,
		                g_utf8_strlen(value, -1));
	else
		XDeleteProperty(xdisplay, xwindow, property);
}

G_GNUC_INTERNAL WindowData *gtk_x11_window_get_window_data(GtkWindow *window)
{
	WindowData *window_data;

	g_return_val_if_fail(GTK_IS_WINDOW(window), NULL);

	window_data = g_object_get_qdata(G_OBJECT(window), window_data_quark());

	if (window_data == NULL)
	{
		static guint window_id;

		GDBusConnection *session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
		char *object_path        = g_strdup_printf(OBJECT_PATH "/%d", window_id);
		char *old_unique_bus_name =
		    gtk_widget_get_x11_property_string(GTK_WIDGET(window), _GTK_UNIQUE_BUS_NAME);
		char *old_unity_object_path =
		    gtk_widget_get_x11_property_string(GTK_WIDGET(window), _UNITY_OBJECT_PATH);
		char *old_menubar_object_path =
		    gtk_widget_get_x11_property_string(GTK_WIDGET(window),
		                                       _GTK_MENUBAR_OBJECT_PATH);
		GDBusActionGroup *old_action_group = NULL;
		GDBusMenuModel *old_menu_model     = NULL;

		if (old_unique_bus_name != NULL)
		{
			if (old_unity_object_path != NULL)
				old_action_group = g_dbus_action_group_get(session,
				                                           old_unique_bus_name,
				                                           old_unity_object_path);

			if (old_menubar_object_path != NULL)
				old_menu_model = g_dbus_menu_model_get(session,
				                                       old_unique_bus_name,
				                                       old_menubar_object_path);
		}

		window_data             = window_data_new();
		window_data->window_id  = window_id++;
		window_data->menu_model = g_menu_new();
		window_data->action_group =
		    unity_gtk_action_group_new(G_ACTION_GROUP(old_action_group));

		if (old_menu_model != NULL)
		{
			window_data->old_model = G_MENU_MODEL(g_object_ref(old_menu_model));
			g_menu_append_section(window_data->menu_model,
			                      NULL,
			                      G_MENU_MODEL(old_menu_model));
		}

		window_data->menu_model_export_id =
		    g_dbus_connection_export_menu_model(session,
		                                        old_menubar_object_path != NULL
		                                            ? old_menubar_object_path
		                                            : object_path,
		                                        G_MENU_MODEL(window_data->menu_model),
		                                        NULL);
		window_data->action_group_export_id =
		    g_dbus_connection_export_action_group(session,
		                                          old_unity_object_path != NULL
		                                              ? old_unity_object_path
		                                              : object_path,
		                                          G_ACTION_GROUP(window_data->action_group),
		                                          NULL);

		if (old_unique_bus_name == NULL)
			gtk_widget_set_x11_property_string(GTK_WIDGET(window),
			                                   _GTK_UNIQUE_BUS_NAME,
			                                   g_dbus_connection_get_unique_name(
			                                       session));

		if (old_unity_object_path == NULL)
			gtk_widget_set_x11_property_string(GTK_WIDGET(window),
			                                   _UNITY_OBJECT_PATH,
			                                   object_path);

		if (old_menubar_object_path == NULL)
			gtk_widget_set_x11_property_string(GTK_WIDGET(window),
			                                   _GTK_MENUBAR_OBJECT_PATH,
			                                   object_path);

		g_object_set_qdata_full(G_OBJECT(window),
		                        window_data_quark(),
		                        window_data,
		                        window_data_free);

		g_free(old_menubar_object_path);
		g_free(old_unity_object_path);
		g_free(old_unique_bus_name);
		g_free(object_path);
	}

	return window_data;
}
#endif
#ifdef GDK_WINDOWING_WAYLAND

#include <wayland-client.h>

void gdk_wayland_window_set_dbus_properties_libgtk_only(
    GdkWindow *window, const char *application_id, const char *app_menu_path,
    const char *menubar_path, const char *window_object_path, const char *application_object_path,
    const char *unique_bus_name);

G_GNUC_INTERNAL WindowData *gtk_wayland_window_get_window_data(GtkWindow *window)
{
	WindowData *window_data;

	g_return_val_if_fail(GTK_IS_WINDOW(window), NULL);

	window_data = g_object_get_qdata(G_OBJECT(window), window_data_quark());
	if (window_data == NULL)
	{
		static guint window_id;
		GMenuModel *old_menu_model         = NULL;
		GDBusActionGroup *old_action_group = NULL;
		GtkApplication *application;
		GApplication *gApp;
		GDBusConnection *connection;

		char *unique_bus_name;
		char *object_path;
		char *menubar_object_path;
		char *application_id;
		char *application_object_path;

		window_data             = window_data_new();
		window_data->menu_model = g_menu_new();

		if (GTK_IS_APPLICATION_WINDOW(window))
		{
			char *unity_object_path;

			application = gtk_window_get_application(window);
			g_return_val_if_fail(GTK_IS_APPLICATION(application), NULL);

			window_data->action_group = NULL;

			gApp = G_APPLICATION(application);
			g_return_val_if_fail(g_application_get_is_registered(gApp), NULL);
			g_return_val_if_fail(!g_application_get_is_remote(gApp), NULL);
			g_return_val_if_fail(window_data->menu_model == NULL ||
			                         G_IS_MENU_MODEL(window_data->menu_model),
			                     NULL);

			application_id =
			    g_strdup_printf("%s", g_application_get_application_id(gApp));
			application_object_path =
			    g_strdup_printf("%s", g_application_get_dbus_object_path(gApp));

			window_data->window_id = window_id++; // IN THE GNOME IMPLEMENTATION THIS IS
			                                      // STARTED IN ONE NOT CERO (So, we
			                                      // make is similar)

			connection  = g_application_get_dbus_connection(gApp);
			object_path = g_strdup_printf(OBJECT_PATH "/%d", window_id);

			unique_bus_name =
			    g_strdup_printf("%s", g_dbus_connection_get_unique_name(connection));
			unity_object_path =
			    g_strdup_printf("%s%s",
			                    g_application_get_dbus_object_path(gApp) != NULL
			                        ? g_application_get_dbus_object_path(gApp)
			                        : object_path,
			                    g_application_get_dbus_object_path(gApp) != NULL
			                        ? "/menus/menubar"
			                        : "");
			menubar_object_path = g_strdup_printf("%s", unity_object_path);

			old_menu_model = G_MENU_MODEL(gtk_application_get_menubar(application));
			if (old_menu_model != NULL)
			{
				old_action_group       = g_dbus_action_group_get(connection,
                                                                           unique_bus_name,
                                                                           unity_object_path);
				window_data->old_model = g_object_ref(old_menu_model);
				g_menu_append_section(window_data->menu_model,
				                      NULL,
				                      old_menu_model);
			}

			// Set the actions
			window_data->action_group =
			    unity_gtk_action_group_new(G_ACTION_GROUP(old_action_group));
			window_data->action_group_export_id =
			    g_dbus_connection_export_action_group(connection,
			                                          unity_object_path,
			                                          G_ACTION_GROUP(
			                                              window_data->action_group),
			                                          NULL);

			// Set the menubar
			gtk_application_set_menubar(GTK_APPLICATION(application),
			                            G_MENU_MODEL(window_data->menu_model));

			g_free(unity_object_path);
		}
		else
		{
			GdkWindow *gdk_win;
			const char *app_menu_path = NULL;

			window_data->window_id = window_id++;

			connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
			unique_bus_name =
			    g_strdup_printf("%s", g_dbus_connection_get_unique_name(connection));
			gdk_win     = gtk_widget_get_window(GTK_WIDGET(window));
			application = gtk_window_get_application(window);

			old_menu_model = G_MENU_MODEL(window_data->menu_model);

			window_data->action_group =
			    unity_gtk_action_group_new(G_ACTION_GROUP(old_action_group));

			if (GTK_IS_APPLICATION(application))
			{
				gApp = G_APPLICATION(application);
				application_id =
				    g_strdup_printf("%s", g_application_get_application_id(gApp));
				object_path =
				    g_strdup_printf("%s/menus/menubar/%d",
				                    g_application_get_dbus_object_path(gApp),
				                    window_data->window_id);
				application_object_path =
				    g_strdup_printf("%s", g_application_get_dbus_object_path(gApp));
				menubar_object_path = g_strdup_printf("%s/window/%d",
				                                      object_path,
				                                      window_data->window_id);
			}
			else
			{
				application_id          = g_strdup_printf("%s",
                                                                 g_get_prgname() != NULL
                                                                     ? g_get_prgname()
                                                                     : gdk_get_program_class());
				object_path             = g_strdup_printf("%s/menus/menubar/%d",
                                                              OBJECT_PATH,
                                                              window_data->window_id);
				application_object_path = g_strdup_printf("%s", OBJECT_PATH);
				menubar_object_path     = g_strdup_printf("%s/window/%d",
                                                                      object_path,
                                                                      window_data->window_id);
			}

			window_data->menu_model_export_id =
			    g_dbus_connection_export_menu_model(connection,
			                                        object_path,
			                                        G_MENU_MODEL(
			                                            window_data->menu_model),
			                                        NULL);
			window_data->action_group_export_id =
			    g_dbus_connection_export_action_group(connection,
			                                          object_path,
			                                          G_ACTION_GROUP(
			                                              window_data->action_group),
			                                          NULL);

			gdk_wayland_window_set_dbus_properties_libgtk_only(gdk_win,
			                                                   application_id,
			                                                   app_menu_path,
			                                                   object_path,
			                                                   menubar_object_path,
			                                                   application_object_path,
			                                                   unique_bus_name);
		}
		g_free(unique_bus_name);
		g_free(object_path);
		g_free(menubar_object_path);
		g_free(application_id);
		g_free(application_object_path);
		g_object_set_qdata_full(G_OBJECT(window),
		                        window_data_quark(),
		                        window_data,
		                        window_data_free);
	}
	return window_data;
}
#endif
