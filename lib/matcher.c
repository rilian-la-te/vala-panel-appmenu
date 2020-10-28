/*
 * vala-panel
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

#include "matcher.h"

struct _ValaPanelMatcher
{
	GObject parent_instance;
	GHashTable *startupids;
	GHashTable *simpletons;
	GHashTable *desktops;
	GHashTable *exec_cache;
	GHashTable *pid_cache;
	GAppInfoMonitor *monitor;
	bool invalidated;
	GRecMutex __lock_invalidated;
	GDBusConnection *bus;
};

static uint app_changed_singal;

G_DEFINE_TYPE(ValaPanelMatcher, vala_panel_matcher, G_TYPE_OBJECT)

static ValaPanelMatcher *default_matcher = NULL;

static void vala_panel_matcher_finalize(GObject *obj)
{
	ValaPanelMatcher *self = VALA_PANEL_MATCHER(obj);
	g_clear_pointer(&self->startupids, g_hash_table_unref);
	g_clear_pointer(&self->simpletons, g_hash_table_unref);
	g_clear_pointer(&self->desktops, g_hash_table_unref);
	g_clear_pointer(&self->exec_cache, g_hash_table_unref);
	g_clear_pointer(&self->pid_cache, g_hash_table_unref);
	g_rec_mutex_clear(&self->__lock_invalidated);
	g_clear_object(&self->bus);
	g_clear_object(&self->monitor);
	G_OBJECT_CLASS(vala_panel_matcher_parent_class)->finalize(obj);
}

static void create_simpletons(ValaPanelMatcher *self)
{
	g_hash_table_insert(self->simpletons,
	                    g_strdup("google-chrome-stable"),
	                    g_strdup("google-chrome"));
	g_hash_table_insert(self->simpletons, g_strdup("calibre-gui"), g_strdup("calibre"));
	g_hash_table_insert(self->simpletons, g_strdup("code - oss"), g_strdup("vscode-oss"));
	g_hash_table_insert(self->simpletons, g_strdup("code"), g_strdup("vscode"));
	g_hash_table_insert(self->simpletons, g_strdup("psppire"), g_strdup("pspp"));
	g_hash_table_insert(self->simpletons,
	                    g_strdup("gnome-twitch"),
	                    g_strdup("com.vinszent.gnometwitch"));
	g_hash_table_insert(self->simpletons, g_strdup("anoise.py"), g_strdup("anoise"));
}

static void vala_panel_matcher_init(ValaPanelMatcher *self)
{
	self->simpletons = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	create_simpletons(self);
	self->pid_cache  = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
	self->desktops   = NULL;
	self->exec_cache = NULL;
	self->startupids = NULL;
	self->monitor    = g_app_info_monitor_get();
	g_rec_mutex_init(&self->__lock_invalidated);
	self->invalidated = false;
}

static void matcher_reload_ids(ValaPanelMatcher *self)
{
	g_clear_pointer(&self->startupids, g_hash_table_unref);
	g_clear_pointer(&self->desktops, g_hash_table_unref);
	g_clear_pointer(&self->exec_cache, g_hash_table_unref);
	self->startupids = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	self->desktops   = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
	self->exec_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	GList *app_info_list = g_app_info_get_all();
	for (GList *l = app_info_list; l != NULL; l = g_list_next(l))
	{
		GDesktopAppInfo *dinfo = G_DESKTOP_APP_INFO(l->data);
		const char *id         = g_app_info_get_id(G_APP_INFO(dinfo));
		if (g_desktop_app_info_get_startup_wm_class(dinfo) != NULL)
		{
			char *down_index =
			    g_utf8_strdown(g_desktop_app_info_get_startup_wm_class(dinfo), -1);
			g_hash_table_insert(self->startupids, down_index, g_strdup(id));
		}
		char *down_index = g_utf8_strdown(id, -1);
		g_hash_table_insert(self->desktops, down_index, dinfo);

		/* Get TryExec if we can, otherwise just Exec */
		char *try_exec = g_desktop_app_info_get_string(dinfo, "TryExec");
		if (try_exec == NULL)
		{
			const char *exec = g_app_info_get_executable(G_APP_INFO(dinfo));
			try_exec         = exec ? g_strdup(exec) : NULL;
		}
		if (try_exec == NULL)
			continue;
		/* Sanitize it */
		char *exec = g_uri_unescape_string(try_exec, NULL);
		g_clear_pointer(&try_exec, g_free);
		try_exec = g_path_get_basename(exec);
		g_clear_pointer(&exec, g_free);
		g_hash_table_insert(self->exec_cache, try_exec, g_strdup(id));
	}
	g_list_free(app_info_list);
}

static void matcher_bus_signal_subscribe(GDBusConnection *connection, const gchar *sender_name,
                                         const gchar *object_path, const gchar *interface_name,
                                         const gchar *signal_name, GVariant *parameters,
                                         gpointer user_data)
{
	ValaPanelMatcher *self              = VALA_PANEL_MATCHER(user_data);
	g_autoptr(GVariant) desktop_variant = NULL;
	int64_t pid                         = 0;
	g_variant_get(parameters,
	              "(@aysxas@a{sv})",
	              &desktop_variant,
	              NULL,
	              &pid,
	              NULL,
	              NULL,
	              NULL);
	const char *desktop_file = g_variant_get_bytestring(desktop_variant);
	if (!g_strcmp0(desktop_file, "") || !pid)
		return;

	g_hash_table_insert(self->pid_cache, GINT_TO_POINTER(pid), g_strdup(desktop_file));
	g_signal_emit(self, app_changed_singal, 0, desktop_file);
}

static void matcher_bus_get_finish(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	g_autoptr(GError) err  = NULL;
	ValaPanelMatcher *self = VALA_PANEL_MATCHER(user_data);
	self->bus              = g_bus_get_finish(res, &err);
	if (err)
	{
		g_warning("%s\n", err->message);
		return;
	}
	g_dbus_connection_signal_subscribe(self->bus,
	                                   NULL,
	                                   "org.gtk.gio.DesktopAppInfo",
	                                   "Launched",
	                                   "/org/gtk/gio/DesktopAppInfo",
	                                   NULL,
	                                   (GDBusSignalFlags)0,
	                                   matcher_bus_signal_subscribe,
	                                   self,
	                                   NULL);
}

static bool invalidate_ids(void *data)
{
	ValaPanelMatcher *self = VALA_PANEL_MATCHER(data);
	g_rec_mutex_lock(&self->__lock_invalidated);
	self->invalidated = true;
	g_rec_mutex_unlock(&self->__lock_invalidated);
	return false;
}

static void on_monitor_changed(GAppInfoMonitor *gappinfomonitor, gpointer user_data)
{
	g_idle_add((GSourceFunc)invalidate_ids, user_data);
}

static GObject *vala_panel_matcher_constructor(GType type, guint n_construct_properties,
                                               GObjectConstructParam *construct_properties)
{
	GObjectClass *parent_class = G_OBJECT_CLASS(vala_panel_matcher_parent_class);
	GObject *obj =
	    parent_class->constructor(type, n_construct_properties, construct_properties);
	ValaPanelMatcher *self = VALA_PANEL_MATCHER(obj);
	g_bus_get(G_BUS_TYPE_SESSION, NULL, matcher_bus_get_finish, self);
	self->monitor = g_app_info_monitor_get();
	g_signal_connect(self->monitor, "changed", G_CALLBACK(on_monitor_changed), self);
	matcher_reload_ids(self);
	return obj;
}

static void matcher_check_invalidated(ValaPanelMatcher *self)
{
	if (self->invalidated)
	{
		g_rec_mutex_lock(&self->__lock_invalidated);
		matcher_reload_ids(self);
		self->invalidated = false;
		g_rec_mutex_unlock(&self->__lock_invalidated);
	}
}

ValaPanelMatcher *vala_panel_matcher_get()
{
	if (VALA_PANEL_IS_MATCHER(default_matcher))
		return g_object_ref(default_matcher);

	return (default_matcher = g_object_new(vala_panel_matcher_get_type(), NULL));
}

GDesktopAppInfo *vala_panel_matcher_match_arbitrary(ValaPanelMatcher *self, const char *class,
                                                    const char *group, const char *gtk, int64_t pid)
{
	matcher_check_invalidated(self);
	const char *checks[] = { class, group };
	for (int i = 0; i < 2; i++)
	{
		if (!checks[i])
			continue;

		/* First, check startupids for this app */
		g_autofree char *check = g_utf8_strdown(checks[i], -1);
		if (g_hash_table_contains(self->startupids, check))
		{
			g_autofree char *dname =
			    g_utf8_strdown((const char *)g_hash_table_lookup(self->startupids,
			                                                     check),
			                   -1);
			if (g_hash_table_contains(self->desktops, dname))
				return G_DESKTOP_APP_INFO(
				    g_hash_table_lookup(self->desktops, dname));
		}
		/* Then try class -> desktop match */
		g_autofree char *dname = g_strdup_printf("%s.desktop", check);
		if (g_hash_table_contains(self->desktops, dname))
			return G_DESKTOP_APP_INFO(g_hash_table_lookup(self->desktops, dname));
	}

	/* If no classes matched, try PID cache */
	if (g_hash_table_contains(self->pid_cache, GINT_TO_POINTER(pid)))
	{
		const char *filename =
		    (const char *)g_hash_table_lookup(self->pid_cache, GINT_TO_POINTER(pid));
		return g_desktop_app_info_new_from_filename(filename);
	}

	/* Next, check GtkApplication ID */
	if (gtk != NULL)
	{
		g_autofree char *app_id = g_utf8_strdown(gtk, -1);
		g_autofree char *gtk_id = g_strdup_printf("%s.desktop", app_id);
		if (g_hash_table_contains(self->desktops, gtk_id))
			return G_DESKTOP_APP_INFO(g_hash_table_lookup(self->desktops, gtk_id));
	}

	/* Check hardcoded matches */
	if (group)
	{
		g_autofree char *grp = g_utf8_strdown(group, -1);
		if (g_hash_table_contains(self->simpletons, grp))
		{
			g_autofree char *dname = g_strdup_printf("%s.desktop", grp);
			if (g_hash_table_contains(self->desktops, dname))
				return G_DESKTOP_APP_INFO(
				    g_hash_table_lookup(self->desktops, dname));
		}
	}
	if (class)
	{
		g_autofree char *grp = g_utf8_strdown(class, -1);
		if (g_hash_table_contains(self->simpletons, grp))
		{
			g_autofree char *dname = g_strdup_printf("%s.desktop", grp);
			if (g_hash_table_contains(self->desktops, dname))
				return G_DESKTOP_APP_INFO(
				    g_hash_table_lookup(self->desktops, dname));
		}
	}

	/* Lastly, try to match an exec line */
	for (int i = 0; i < 2; i++)
	{
		if (!checks[i])
			continue;

		g_autofree char *check = g_utf8_strdown(checks[i], -1);
		const char *id         = (const char *)g_hash_table_lookup(self->exec_cache, check);
		if (id == NULL)
			continue;
		GDesktopAppInfo *a = G_DESKTOP_APP_INFO(g_hash_table_lookup(self->desktops, id));
		if (a != NULL)
			return a;
	}

	/* IDK. Sorry. */
	return NULL;
}

static void vala_panel_matcher_class_init(ValaPanelMatcherClass *klass)
{
	vala_panel_matcher_parent_class    = g_type_class_peek_parent(klass);
	G_OBJECT_CLASS(klass)->constructor = vala_panel_matcher_constructor;
	G_OBJECT_CLASS(klass)->finalize    = vala_panel_matcher_finalize;
	app_changed_singal                 = g_signal_new("app-launched",
                                          vala_panel_matcher_get_type(),
                                          G_SIGNAL_RUN_LAST,
                                          0,
                                          NULL,
                                          NULL,
                                          g_cclosure_marshal_VOID__STRING,
                                          G_TYPE_NONE,
                                          1,
                                          G_TYPE_STRING);
}
