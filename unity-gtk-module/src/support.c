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
#if GTK_MAJOR_VERSION >= 3
	if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
		is_platform_supported = true;
	else if (GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default()))
		is_platform_supported = true;
	else
		is_platform_supported = false;
#else
	is_platform_supported = true;
#endif
	is_program_supported =
	    (proxy == NULL || is_true(proxy)) && !is_blacklisted(g_get_prgname());
	return is_program_supported && is_platform_supported;
}

G_GNUC_INTERNAL void enable_debug()
{
	unity_gtk_menu_shell_set_debug(is_true(g_getenv("UNITY_GTK_MENU_SHELL_DEBUG")));
	unity_gtk_action_group_set_debug(is_true(g_getenv("UNITY_GTK_ACTION_GROUP_DEBUG")));
}

#if GTK_MAJOR_VERSION == 3
G_GNUC_INTERNAL bool gtk_widget_shell_shows_menubar(GtkWidget *widget)
{
	GtkSettings *settings;
	GParamSpec *pspec;
	gboolean shell_shows_menubar;

	g_return_val_if_fail(GTK_IS_WIDGET(widget), FALSE);

	settings = gtk_widget_get_settings(widget);

	g_return_val_if_fail(GTK_IS_SETTINGS(settings), FALSE);

	pspec =
	    g_object_class_find_property(G_OBJECT_GET_CLASS(settings), "gtk-shell-shows-menubar");

	g_return_val_if_fail(G_IS_PARAM_SPEC(pspec), FALSE);
	g_return_val_if_fail(pspec->value_type == G_TYPE_BOOLEAN, FALSE);

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
#else
static void g_settings_handle_gtk_shell_shows_menubar(GSettings *settings, gchar *key,
                                                      gpointer user_data)
{
	gtk_widget_queue_resize(user_data);
}

G_GNUC_INTERNAL bool gtk_widget_shell_shows_menubar(GtkWidget *widget)
{
	GSettings *settings =
	    G_SETTINGS(g_object_get_data(G_OBJECT(widget), UNITY_GTK_MODULE_SCHEMA));
	if (settings == NULL)
	{
		settings = g_settings_new(UNITY_GTK_MODULE_SCHEMA);
		g_object_set_data_full(G_OBJECT(widget),
		                       UNITY_GTK_MODULE_SCHEMA,
		                       (gpointer)settings,
		                       (GDestroyNotify)g_object_unref);
		g_signal_connect(settings,
		                 "changed::" SHELL_SHOWS_MENUBAR_KEY,
		                 G_CALLBACK(g_settings_handle_gtk_shell_shows_menubar),
		                 widget);
	}
	return g_settings_get_boolean(settings, SHELL_SHOWS_MENUBAR_KEY);
}
G_GNUC_INTERNAL void gtk_widget_connect_settings(GtkWidget *widget)
{
	GSettings *settings =
	    G_SETTINGS(g_object_get_data(G_OBJECT(widget), UNITY_GTK_MODULE_SCHEMA));
	if (settings == NULL)
	{
		settings = g_settings_new(UNITY_GTK_MODULE_SCHEMA);
		g_object_set_data_full(G_OBJECT(widget),
		                       UNITY_GTK_MODULE_SCHEMA,
		                       (gpointer)settings,
		                       (GDestroyNotify)g_object_unref);
		g_signal_connect(settings,
		                 "changed::" SHELL_SHOWS_MENUBAR_KEY,
		                 G_CALLBACK(g_settings_handle_gtk_shell_shows_menubar),
		                 widget);
	}
}

G_GNUC_INTERNAL void gtk_widget_disconnect_settings(GtkWidget *widget)
{
	GSettings *settings =
	    G_SETTINGS(g_object_get_data(G_OBJECT(widget), UNITY_GTK_MODULE_SCHEMA));
	if (settings != NULL)
	{
		g_signal_handlers_disconnect_by_data(settings, widget);
		g_object_set_data(G_OBJECT(widget), UNITY_GTK_MODULE_SCHEMA, NULL);
	}
}
#endif
