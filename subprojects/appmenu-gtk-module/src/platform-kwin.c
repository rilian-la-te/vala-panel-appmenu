/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "consts.h"
#include "datastructs-private.h"
#include "datastructs.h"
#include "glibconfig.h"
#include "org.kde.appmenu-client-protocol.h"
#include "platform.h"

#include <exporter.h>
#include <gdk/gdkwayland.h>
#include <wayland-client.h>

extern bool wl_has(const char *interface);

static struct org_kde_kwin_appmenu_manager *kwin_manager = NULL;

static void kwin_registry_global(void *data, struct wl_registry *registry, uint32_t name,
                                 const char *interface, uint32_t version)
{
	(void)data;

	if (kwin_manager == NULL &&
	    g_strcmp0(interface, org_kde_kwin_appmenu_manager_interface.name) == 0)
	{
		kwin_manager = wl_registry_bind(registry,
		                                name,
		                                &org_kde_kwin_appmenu_manager_interface,
		                                MIN((uint32_t)version, 1));
	}
}

static void kwin_registry_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
	(void)data;
	(void)registry;
	(void)name;
}

static const struct wl_registry_listener kwin_registry_listener = {
	.global        = kwin_registry_global,
	.global_remove = kwin_registry_global_remove,
};

G_GNUC_INTERNAL G_DEFINE_QUARK(kwin_backend, kwin_backend);

static struct org_kde_kwin_appmenu_manager *get_kwin_manager(void)
{
	struct wl_display *display;
	struct wl_registry *registry;

	if (kwin_manager)
		return kwin_manager;

	display = gdk_wayland_display_get_wl_display(gdk_display_get_default());
	if (display == NULL)
		return NULL;

	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &kwin_registry_listener, &kwin_manager);
	wl_display_roundtrip(display);

	wl_registry_destroy(registry);
	registry = NULL;

	return kwin_manager;
}

static gboolean on_window_map_event(GtkWidget *w, GdkEvent *ev, GObject *exporter)
{
	struct org_kde_kwin_appmenu_manager *manager = get_kwin_manager();
	struct wl_surface *wl_surface = gdk_wayland_window_get_wl_surface(gtk_widget_get_window(w));
	char *bus                     = NULL;
	char *path                    = NULL;
	g_object_get(exporter, "bus-name", &bus, "object-path", &path, NULL);

	if (manager != NULL && wl_surface != NULL)
	{
		struct org_kde_kwin_appmenu *fresh =
		    org_kde_kwin_appmenu_manager_create(manager, wl_surface);
		org_kde_kwin_appmenu_set_address(fresh, bus, path);
		g_object_set_qdata_full(exporter,
		                        kwin_backend_quark(),
		                        fresh,
		                        (GDestroyNotify)org_kde_kwin_appmenu_release);
	}
	g_free(bus);
	g_free(path);
	return FALSE;
}

static WindowData *establish_window_data_kwin(GtkWindow *window)
{
	static guint window_id;
	GtkApplication *application;
	GApplication *gApp;
	g_autoptr(GDBusConnection) connection = NULL;
	WindowData *window_data;

	char *unique_bus_name;
	char *object_path;

	GdkWindow *gdk_win;
	struct wl_surface *wl_surface;
	struct org_kde_kwin_appmenu *appmenu;
	struct org_kde_kwin_appmenu_manager *manager = get_kwin_manager();

	window_data             = window_data_new();
	window_data->menu_model = g_menu_new();
	window_data->window_id  = window_id++;

	connection      = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
	unique_bus_name = g_strdup(g_dbus_connection_get_unique_name(connection));

	gdk_win     = gtk_widget_get_window(GTK_WIDGET(window));
	application = gtk_window_get_application(window);

	window_data->action_group = unity_gtk_action_group_new(G_ACTION_GROUP(NULL));
	GMenuModel *old_menu_model = NULL;

	if (GTK_IS_APPLICATION(application))
	{
		gApp        = G_APPLICATION(application);
		object_path = g_strdup_printf("%s/menus/dbusmenu/%d",
		                              g_application_get_dbus_object_path(gApp),
		                              window_data->window_id);
		old_menu_model = gtk_application_get_menubar(application);
		if (old_menu_model != NULL && g_menu_model_get_n_items(old_menu_model) != 0)
		{
			window_data->old_model = G_MENU_MODEL(g_object_ref(old_menu_model));
			g_menu_append_section(window_data->menu_model,
			                      NULL,
			                      G_MENU_MODEL(old_menu_model));
		}
	}
	else
	{
		object_path =
		    g_strdup_printf("%s/menus/dbusmenu/%d", OBJECT_PATH, window_data->window_id);
	}
	window_data->exporter =
	    G_OBJECT(dbus_menu_exporter_new(G_MENU_MODEL(window_data->menu_model),
	                                    "unity",
	                                    G_ACTION_GROUP(window_data->action_group),
	                                    unique_bus_name,
	                                    object_path));

	g_autofree const gchar **prefixes = gtk_widget_list_action_prefixes(GTK_WIDGET(window));
	if (prefixes)
	{
		for (gint i = 0; prefixes[i] != NULL; i++)
		{
			const gchar *prefix = prefixes[i];
			GActionGroup *group =
			    gtk_widget_get_action_group(GTK_WIDGET(window), prefix);

			if (group)
				dbus_menu_exporter_insert(DBUS_MENU_EXPORTER(window_data->exporter),
				                          prefix,
				                          group);
		}
	}

	wl_surface = gdk_wayland_window_get_wl_surface(gdk_win);

	if (manager != NULL && wl_surface != NULL)
	{
		appmenu = org_kde_kwin_appmenu_manager_create(manager, wl_surface);
		org_kde_kwin_appmenu_set_address(appmenu, unique_bus_name, object_path);

		/* Proxy lives exactly as long as the exporter it describes. */
		g_object_set_qdata_full(window_data->exporter,
		                        kwin_backend_quark(),
		                        appmenu,
		                        (GDestroyNotify)org_kde_kwin_appmenu_release);
		g_signal_connect_object(window,
		                        "map-event",
		                        G_CALLBACK(on_window_map_event),
		                        window_data->exporter,
		                        0);
	}

	g_free(unique_bus_name);
	g_free(object_path);
	return window_data;
}

static bool is_applicable(GtkWindow *window)
{
	g_return_val_if_fail(GTK_IS_WINDOW(window), false);
	GdkWindow *gdk_win = gtk_widget_get_window((GtkWidget *)window);
	if (!gdk_win)
		return false;
	if (!GDK_IS_WAYLAND_WINDOW(gdk_win))
		return false;
	return wl_has(org_kde_kwin_appmenu_manager_interface.name);
}

static bool is_runnable()
{
	bool wl                        = GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default());
	return wl && wl_has(org_kde_kwin_appmenu_manager_interface.name);
}

const ModuleBackend kwin_platform = {
	.name                   = "kwin-wayland",
	.is_runnable            = is_runnable,
	.establish_window_data  = establish_window_data_kwin,
	.is_applicable          = is_applicable,
	.before_realize_allowed = false,
};
