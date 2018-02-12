/*
 * vala-panel
 * Copyright (C) 2015-2016 Konstantin Pugin <ria.freelander@gmail.com>
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

#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "launcher.h"

typedef struct
{
	pid_t pid;
} SpawnData;

void child_spawn_func(void *data)
{
	setpgid(0, getpgid(getppid()));
}

bool vala_panel_launch(GDesktopAppInfo *app_info, GList *uris, GtkWidget *parent)
{
	g_autoptr(GError) err            = NULL;
	g_autoptr(GAppLaunchContext) cxt = G_APP_LAUNCH_CONTEXT(
	    gdk_display_get_app_launch_context(gtk_widget_get_display(parent)));
	bool ret = g_desktop_app_info_launch_uris_as_manager(G_DESKTOP_APP_INFO(app_info),
	                                                     uris,
	                                                     cxt,
	                                                     G_SPAWN_SEARCH_PATH,
	                                                     child_spawn_func,
	                                                     NULL,
	                                                     NULL,
	                                                     NULL,
	                                                     &err);
	if (err)
		g_warning("%s\n", err->message);
	return ret;
}

GAppInfo *vala_panel_get_default_for_uri(const char *uri)
{
	/* g_file_query_default_handler() calls
	 * g_app_info_get_default_for_uri_scheme() too, but we have to do it
	 * here anyway in case GFile can't parse @uri correctly.
	 */
	GAppInfo *app_info          = NULL;
	g_autofree char *uri_scheme = g_uri_parse_scheme(uri);
	if (uri_scheme != NULL && strlen(uri_scheme) <= 0)
		app_info = g_app_info_get_default_for_uri_scheme(uri_scheme);
	if (app_info == NULL)
	{
		g_autoptr(GFile) file = g_file_new_for_uri(uri);
		app_info              = g_file_query_default_handler(file, NULL, NULL);
	}
	return app_info;
}

void menu_launch_id(GSimpleAction *action, GVariant *param, gpointer user_data)
{
	const char *id                  = g_variant_get_string(param, NULL);
	g_autoptr(GDesktopAppInfo) info = g_desktop_app_info_new(id);
	GtkWidget *window               = GTK_WIDGET(user_data);
	vala_panel_launch(info, NULL, GTK_WIDGET(window));
}

void menu_launch_uri(GSimpleAction *action, GVariant *param, gpointer user_data)
{
	const char *uri                 = g_variant_get_string(param, NULL);
	g_autoptr(GList) uris           = g_list_append(NULL, (gpointer)uri);
	g_autoptr(GDesktopAppInfo) info = G_DESKTOP_APP_INFO(vala_panel_get_default_for_uri(uri));
	GtkWidget *window               = GTK_WIDGET(user_data);
	vala_panel_launch(info, uris, GTK_WIDGET(window));
}

void menu_launch_command(GSimpleAction *action, GVariant *param, gpointer user_data)
{
	g_autoptr(GError) err           = NULL;
	const char *commandline         = g_variant_get_string(param, NULL);
	g_autoptr(GDesktopAppInfo) info = G_DESKTOP_APP_INFO(
	    g_app_info_create_from_commandline(commandline,
	                                       NULL,
	                                       G_APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION,
	                                       &err));
	if (err)
		g_warning("%s\n", err->message);
	GtkWidget *window = GTK_WIDGET(user_data);
	vala_panel_launch(info, NULL, GTK_WIDGET(window));
}

// FIXME: reads only first section
char *posix_get_cmdline_string(const char *filename)
{
	char *ret = (char *)g_malloc0(1024);
	int fd    = open(filename, O_RDONLY);
	bool rr   = read(fd, ret, 1024);
	if (rr)
		return ret;
	else
	{
		g_free(ret);
		return NULL;
	}
}
