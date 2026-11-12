/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "platform.h"
#include "consts.h"
#include "datastructs-private.h"
#include "datastructs.h"

#include <gdk/gdkx.h>

#if GTK_MAJOR_VERSION < 3
#define GDK_IS_X11_DISPLAY GDK_IS_DISPLAY
#define GDK_IS_X11_WINDOW GDK_IS_WINDOW
#endif

static char *gtk_widget_get_x11_property_string(GtkWidget *widget, const char *name)
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

static void gtk_widget_set_x11_property_string(GtkWidget *widget, const char *name,
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

static WindowData* establish_window_data_x11(GtkWindow* window) 
{
    WindowData* window_data;
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

    g_free(old_menubar_object_path);
    g_free(old_unity_object_path);
    g_free(old_unique_bus_name);
    g_free(object_path);
    return window_data;
}

static bool is_runnable() {
    return GDK_IS_X11_DISPLAY(gdk_display_get_default());
}

static bool is_applicable(GtkWindow* window) {
    g_return_val_if_fail(GTK_IS_WINDOW(window), NULL);
	GdkWindow* gdk_win = gtk_widget_get_window((GtkWidget*)window);
    if(!gdk_win)
        return false;
    if(!GDK_IS_X11_WINDOW(gdk_win))
        return false;
    return true;
}

const ModuleBackend x11_platform = 
{
    .is_runnable = is_runnable,
    .establish_window_data = establish_window_data_x11,
    .is_applicable = is_applicable,
    .before_realize_allowed = false,
};