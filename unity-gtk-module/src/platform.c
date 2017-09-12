#include "platform.h"

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#if GTK_MAJOR_VERSION == 3
#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif
#endif

#ifdef GDK_WINDOWING_X11
G_GNUC_INTERNAL gchar *gtk_widget_get_x11_property_string(GtkWidget *widget, const gchar *name)
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
			gchar *string = g_strdup((const gchar *)prop);

			if (prop != NULL)
				XFree(prop);

			return string;
		}
		else
			return NULL;
	}

	return NULL;
}

G_GNUC_INTERNAL void gtk_widget_set_x11_property_string(GtkWidget *widget, const gchar *name,
                                                        const gchar *value)
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
#endif
#ifdef GDK_WINDOWING_WAYLAND
void gdk_wayland_window_set_dbus_properties_libgtk_only(
    GdkWindow *window, const char *application_id, const char *app_menu_path,
    const char *menubar_path, const char *window_object_path, const char *application_object_path,
    const char *unique_bus_name);

#endif
G_GNUC_INTERNAL void gtk_widget_set_property_string(GtkWidget *widget, const gchar *name,
                                                    const gchar *value)
{
#if GTK_MAJOR_VERSION == 3
	if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
#endif
		gtk_widget_set_x11_property_string(widget, name, value);
}

G_GNUC_INTERNAL gchar *gtk_widget_get_property_string(GtkWidget *widget, const gchar *name)
{
#if GTK_MAJOR_VERSION == 3
	if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
#endif
		return gtk_widget_get_x11_property_string(widget, name);
	g_assert_not_reached();
}
