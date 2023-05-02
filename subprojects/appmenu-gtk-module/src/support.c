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

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#if GTK_MAJOR_VERSION >= 3
#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#else
#define GDK_IS_WAYLAND_DISPLAY(display) 0
#endif
#endif

#include <appmenu-gtk-action-group.h>
#include <appmenu-gtk-menu-shell.h>

#include "blacklist.h"
#include "consts.h"
#include "support.h"

static bool is_true(const char *value)
{
	return value != NULL && value[0] != '\0' && g_ascii_strcasecmp(value, "0") != 0 &&
	       g_ascii_strcasecmp(value, "no") != 0 && g_ascii_strcasecmp(value, "off") != 0 &&
	       g_ascii_strcasecmp(value, "false") != 0;
}

G_GNUC_INTERNAL bool gtk_module_should_run()
{
	const char *proxy          = g_getenv("UBUNTU_MENUPROXY");
	bool is_platform_supported = false;
	bool is_program_supported  = false;
	bool should_run            = false;
	static bool run_once       = true;
#if GTK_MAJOR_VERSION >= 3
	if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
		is_platform_supported = true;
	else if (GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default()))
	{
		g_autoptr(GSettings) gsettings = g_settings_new(UNITY_GTK_MODULE_SCHEMA);
		bool use_wayland                  = g_settings_get_boolean(gsettings, RUN_ON_WAYLAND);
		is_platform_supported = use_wayland;
	}
	else
		is_platform_supported = false;
#else
	is_platform_supported = true;
#endif
	is_program_supported =
	    (proxy == NULL || is_true(proxy)) && !is_blacklisted(g_get_prgname());
	should_run = is_program_supported && is_platform_supported && run_once;
	run_once   = !(is_program_supported && is_platform_supported);
	return should_run;
}

G_GNUC_INTERNAL void enable_debug()
{
	unity_gtk_menu_shell_set_debug(is_true(g_getenv("UNITY_GTK_MENU_SHELL_DEBUG")));
	unity_gtk_action_group_set_debug(is_true(g_getenv("UNITY_GTK_ACTION_GROUP_DEBUG")));
}

G_GNUC_INTERNAL bool gtk_widget_shell_shows_menubar(GtkWidget *widget)
{
	GtkSettings *settings;
	GParamSpec *pspec;
	gboolean shell_shows_menubar;

	g_return_val_if_fail(GTK_IS_WIDGET(widget), false);

	settings = gtk_widget_get_settings(widget);

	g_return_val_if_fail(GTK_IS_SETTINGS(settings), false);

	pspec =
	    g_object_class_find_property(G_OBJECT_GET_CLASS(settings), "gtk-shell-shows-menubar");

	g_return_val_if_fail(G_IS_PARAM_SPEC(pspec), false);
	g_return_val_if_fail(pspec->value_type == G_TYPE_BOOLEAN, false);

	g_object_get(settings, "gtk-shell-shows-menubar", &shell_shows_menubar, NULL);

	return shell_shows_menubar;
}
static void gtk_settings_handle_gtk_shell_shows_menubar(GObject *object, GParamSpec *pspec,
                                                        gpointer user_data)
{
	gtk_widget_queue_resize(user_data);
}
G_GNUC_INTERNAL void gtk_widget_connect_settings(GtkWidget *widget)
{
	GtkSettings *settings = gtk_widget_get_settings(widget);
	g_signal_connect(settings,
	                 "notify::gtk-shell-shows-menubar",
	                 G_CALLBACK(gtk_settings_handle_gtk_shell_shows_menubar),
	                 widget);
}

G_GNUC_INTERNAL void gtk_widget_disconnect_settings(GtkWidget *widget)
{
	GtkSettings *settings = gtk_widget_get_settings(widget);
	if (settings != NULL)
		g_signal_handlers_disconnect_by_data(settings, widget);
}

#if (GTK_MAJOR_VERSION < 3) || defined(GDK_WINDOWING_WAYLAND)
static uint watcher_id = 0;

static gboolean is_dbus_present()
{
	GDBusConnection *connection;
	GVariant *ret, *names;
	GVariantIter *iter;
	char *name;
	gboolean is_present;
	GError *error = NULL;

	is_present = false;

	connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	if (connection == NULL)
	{
		g_warning("Unable to connect to dbus: %s", error->message);
		g_error_free(error);
		return false;
	}

	ret = g_dbus_connection_call_sync(connection,
	                                  "org.freedesktop.DBus",
	                                  "/org/freedesktop/DBus",
	                                  "org.freedesktop.DBus",
	                                  "ListNames",
	                                  NULL,
	                                  G_VARIANT_TYPE("(as)"),
	                                  G_DBUS_CALL_FLAGS_NONE,
	                                  -1,
	                                  NULL,
	                                  &error);
	if (ret == NULL)
	{
		g_warning("Unable to query dbus: %s", error->message);
		g_error_free(error);
		return false;
	}
	names = g_variant_get_child_value(ret, 0);
	g_variant_get(names, "as", &iter);
	while (g_variant_iter_loop(iter, "s", &name))
	{
		if (g_str_equal(name, "com.canonical.AppMenu.Registrar"))
		{
			is_present = true;
			break;
		}
	}
	g_variant_iter_free(iter);
	g_variant_unref(names);
	g_variant_unref(ret);

	return is_present;
}

static bool set_gtk_shell_shows_menubar(bool shows)
{
	GtkSettings *settings = gtk_settings_get_default();

	g_return_val_if_fail(GTK_IS_SETTINGS(settings), false);

	GParamSpec *pspec =
	    g_object_class_find_property(G_OBJECT_GET_CLASS(settings), "gtk-shell-shows-menubar");

	g_return_val_if_fail(G_IS_PARAM_SPEC(pspec), false);
	g_return_val_if_fail(pspec->value_type == G_TYPE_BOOLEAN, false);

	g_autoptr(GSettings) gsettings = g_settings_new(UNITY_GTK_MODULE_SCHEMA);
	bool need_set                  = !g_settings_get_boolean(gsettings, INNER_MENU_KEY);

	g_object_set(settings, "gtk-shell-shows-menubar", need_set ? shows : false, NULL);

	return true;
}

static void on_name_appeared(GDBusConnection *connection, const char *name, const char *name_owner,
                             gpointer user_data)
{
	g_debug("Name %s on the session bus is owned by %s\n", name, name_owner);

	set_gtk_shell_shows_menubar(true);
}

static void on_name_vanished(GDBusConnection *connection, const char *name, gpointer user_data)
{
	g_debug("Name %s does not exist on the session bus\n", name);

	set_gtk_shell_shows_menubar(false);
}
#endif

G_GNUC_INTERNAL void watch_registrar_dbus()
{
#if (GTK_MAJOR_VERSION < 3) || defined(GDK_WINDOWING_WAYLAND)
	set_gtk_shell_shows_menubar(is_dbus_present());

	if (watcher_id == 0)
	{
		watcher_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
		                              "com.canonical.AppMenu.Registrar",
		                              G_BUS_NAME_WATCHER_FLAGS_NONE,
		                              on_name_appeared,
		                              on_name_vanished,
		                              NULL,
		                              NULL);
	}
#endif
}
