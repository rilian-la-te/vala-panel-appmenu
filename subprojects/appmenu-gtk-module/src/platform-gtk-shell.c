
/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "platform.h"
#include "consts.h"
#include "datastructs-private.h"
#include "datastructs.h"
#include "gtk-shell-client-protocol.h"

#include <wayland-client.h>
#include <gdk/gdkwayland.h>


void gdk_wayland_window_set_dbus_properties_libgtk_only(
    GdkWindow *window, const char *application_id, const char *app_menu_path,
    const char *menubar_path, const char *window_object_path, const char *application_object_path,
    const char *unique_bus_name);

extern bool wl_has(const char *interface);

static WindowData* establish_window_data_gtk_shell(GtkWindow* window) 
{
    static guint window_id;
    GDBusActionGroup *old_action_group = NULL;
    GtkApplication *application;
    GApplication *gApp;
    GDBusConnection *connection;
    WindowData* window_data;

    char *unique_bus_name;
    char *object_path;
    char *menubar_object_path;
    char *application_id;
    char *application_object_path;

    window_data             = window_data_new();
    window_data->menu_model = g_menu_new();

    GdkWindow *gdk_win;
    const char *app_menu_path = NULL;

    window_data->window_id = window_id++;

    connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
    unique_bus_name =
        g_strdup_printf("%s", g_dbus_connection_get_unique_name(connection));
    gdk_win     = gtk_widget_get_window(GTK_WIDGET(window));
    application = gtk_window_get_application(window);

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
    g_free(unique_bus_name);
    g_free(object_path);
    g_free(menubar_object_path);
    g_free(application_id);
    g_free(application_object_path);
    g_object_set_qdata_full(G_OBJECT(window),
                            window_data_quark(),
                            window_data,
                            window_data_free);
    return window_data;
}


static bool is_applicable(GtkWindow* window) {
    g_return_val_if_fail(GTK_IS_WINDOW(window), NULL);
	GdkWindow* gdk_win = gtk_widget_get_window((GtkWidget*)window);
    if(!gdk_win)
        return false;
    if(!GDK_IS_WAYLAND_WINDOW(gdk_win))
        return false;
    return wl_has(gtk_surface1_interface.name);
}

static bool is_runnable() {
    bool wl = GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default());
    g_autoptr(GSettings) gsettings = g_settings_new(UNITY_GTK_MODULE_SCHEMA);
	bool use_wayland                  = g_settings_get_boolean(gsettings, RUN_ON_WAYLAND);
    return wl && use_wayland && wl_has(gtk_surface1_interface.name);
}

const ModuleBackend gtk_shell_platform = 
{
    .is_runnable = is_runnable,
    .establish_window_data = establish_window_data_gtk_shell,
    .is_applicable = is_applicable,
    .before_realize_allowed = true,
};