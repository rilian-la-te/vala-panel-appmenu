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

#include "datastructs.h"
#include "datastructs-private.h"
#include "platform.h"

#include <appmenu-gtk-menu-shell.h>

G_GNUC_INTERNAL G_DEFINE_QUARK(window_data, window_data);
G_DEFINE_BOXED_TYPE(WindowData, window_data, (GBoxedCopyFunc)window_data_copy,
                    (GBoxedFreeFunc)window_data_free);
G_GNUC_INTERNAL G_DEFINE_QUARK(menu_shell_data, menu_shell_data);
G_DEFINE_BOXED_TYPE(MenuShellData, menu_shell_data, (GBoxedCopyFunc)menu_shell_data_copy,
                    (GBoxedFreeFunc)menu_shell_data_free);

G_GNUC_INTERNAL WindowData *window_data_new(void)
{
	return g_slice_new0(WindowData);
}

G_GNUC_INTERNAL void window_data_free(gpointer data)
{
	WindowData *window_data = data;

	if (window_data != NULL)
	{
		GDBusConnection *session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

		if (window_data->action_group_export_id)
			g_dbus_connection_unexport_action_group(session,
			                                        window_data
			                                            ->action_group_export_id);

		if (window_data->menu_model_export_id)
			g_dbus_connection_unexport_menu_model(session,
			                                      window_data->menu_model_export_id);

		if (window_data->action_group != NULL)
			g_object_unref(window_data->action_group);

		if (window_data->menu_model != NULL)
			g_object_unref(window_data->menu_model);

		if (window_data->old_model != NULL)
			g_object_unref(window_data->old_model);

		if (window_data->menus != NULL)
			g_slist_free_full(window_data->menus, g_object_unref);

		g_slice_free(WindowData, window_data);
	}
}

G_GNUC_INTERNAL WindowData *window_data_copy(WindowData *source)
{
	WindowData *ret             = window_data_new();
	ret->action_group_export_id = source->action_group_export_id;
	ret->menu_model_export_id   = source->menu_model_export_id;
	if (source->action_group != NULL)
		ret->action_group = g_object_ref(source->action_group);

	if (source->menu_model != NULL)
		ret->menu_model = g_object_ref(source->menu_model);

	if (source->old_model != NULL)
		ret->old_model = g_object_ref(source->old_model);

	if (source->menus != NULL)
		ret->menus = g_slist_copy_deep(source->menus, (GCopyFunc)g_object_ref, NULL);

	return ret;
}

G_GNUC_INTERNAL MenuShellData *menu_shell_data_new(void)
{
	return g_slice_new0(MenuShellData);
}

G_GNUC_INTERNAL void menu_shell_data_free(gpointer data)
{
	if (data != NULL)
		g_slice_free(MenuShellData, data);
}

G_GNUC_INTERNAL MenuShellData *menu_shell_data_copy(MenuShellData *source)
{
	MenuShellData *ret = menu_shell_data_new();
	ret->window        = source->window;
	return ret;
}

G_GNUC_INTERNAL bool menu_shell_data_has_window(MenuShellData *source)
{
	return source->window != NULL;
}

G_GNUC_INTERNAL GtkWindow *menu_shell_data_get_window(MenuShellData *source)
{
	return source->window;
}

G_GNUC_INTERNAL MenuShellData *gtk_menu_shell_get_menu_shell_data(GtkMenuShell *menu_shell)
{
	MenuShellData *menu_shell_data;

	g_return_val_if_fail(GTK_IS_MENU_SHELL(menu_shell), NULL);

	menu_shell_data = g_object_get_qdata(G_OBJECT(menu_shell), menu_shell_data_quark());

	if (menu_shell_data == NULL)
	{
		menu_shell_data = menu_shell_data_new();

		g_object_set_qdata_full(G_OBJECT(menu_shell),
		                        menu_shell_data_quark(),
		                        menu_shell_data,
		                        menu_shell_data_free);
	}

	return menu_shell_data;
}

G_GNUC_INTERNAL WindowData *gtk_window_get_window_data(GtkWindow *window)
{
	WindowData *window_data = NULL;

	g_return_val_if_fail(GTK_IS_WINDOW(window), NULL);

#if (defined(GDK_WINDOWING_WAYLAND))
	if (GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default()))
		window_data = gtk_wayland_window_get_window_data(window);
#endif
#if (defined(GDK_WINDOWING_X11))
#if GTK_MAJOR_VERSION == 3
	if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
#endif
		window_data = gtk_x11_window_get_window_data(window);
#endif
	return window_data;
}

G_GNUC_INTERNAL void gtk_window_disconnect_menu_shell(GtkWindow *window, GtkMenuShell *menu_shell)
{
	WindowData *window_data;
	MenuShellData *menu_shell_data;

	g_return_if_fail(GTK_IS_WINDOW(window));
	g_return_if_fail(GTK_IS_MENU_SHELL(menu_shell));

	menu_shell_data = gtk_menu_shell_get_menu_shell_data(menu_shell);

	g_warn_if_fail(window == menu_shell_data->window);

	window_data = gtk_window_get_window_data(menu_shell_data->window);

	if (window_data != NULL)
	{
		GSList *iter;
		guint i = 0;

		if (window_data->old_model != NULL)
			i++;

		for (iter = window_data->menus; iter != NULL; iter = g_slist_next(iter), i++)
			if (UNITY_GTK_MENU_SHELL(iter->data)->menu_shell == menu_shell)
				break;

		if (iter != NULL)
		{
			g_menu_remove(window_data->menu_model, i);

			unity_gtk_action_group_disconnect_shell(window_data->action_group,
			                                        iter->data);

			g_object_unref(iter->data);

			window_data->menus = g_slist_delete_link(window_data->menus, iter);
		}

		menu_shell_data->window = NULL;
	}
}

G_GNUC_INTERNAL void gtk_window_connect_menu_shell(GtkWindow *window, GtkMenuShell *menu_shell)
{
	MenuShellData *menu_shell_data;

	g_return_if_fail(GTK_IS_WINDOW(window));
	g_return_if_fail(GTK_IS_MENU_SHELL(menu_shell));

	menu_shell_data = gtk_menu_shell_get_menu_shell_data(menu_shell);

	if (window != menu_shell_data->window)
	{
		WindowData *window_data;

		if (menu_shell_data->window != NULL)
			gtk_window_disconnect_menu_shell(menu_shell_data->window, menu_shell);

		window_data = gtk_window_get_window_data(window);

		if (window_data != NULL)
		{
			GSList *iter;

			for (iter = window_data->menus; iter != NULL; iter = g_slist_next(iter))
				if (UNITY_GTK_MENU_SHELL(iter->data)->menu_shell == menu_shell)
					break;

			if (iter == NULL)
			{
				UnityGtkMenuShell *shell = unity_gtk_menu_shell_new(menu_shell);

				unity_gtk_action_group_connect_shell(window_data->action_group,
				                                     shell);

				g_menu_append_section(window_data->menu_model,
				                      NULL,
				                      G_MENU_MODEL(shell));

				window_data->menus = g_slist_append(window_data->menus, shell);
			}
		}

		menu_shell_data->window = window;
	}
}
