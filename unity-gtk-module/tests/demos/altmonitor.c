/*
 * Copyright © 2011 Canonical Limited
 *
 * All rights reserved.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include "altmonitor.h"

#include <X11/XKBlib.h>
#include <gdk/gdkx.h>

/**
 * SECTION:altmonitor
 * @title: AltMonitor
 * @short_description: a utility class to monitor the Alt key
 *
 * #AltMonitor is a small utility class for monitoring the state of the
 * 'Alt' key on a particular #GdkDisplay.  It depends on a functioning
 * Xkb extension on the display.
 *
 * #AltMonitor has one property: "alt-pressed".
 *
 * To watch for changes, connect to the GObject::notify signal for the
 * "alt-pressed" property.  Query the current state with
 * alt_monitor_get_alt_pressed().
 **/

/**
 * AltMonitor:
 *
 * AltMonitor is an opaque structure type with no public fields.
 **/

typedef GObjectClass AltMonitorClass;
struct _AltMonitor
{
	GObject parent_instance;

	GdkDisplay *display;
	gint xkb_event_type;
	gboolean is_pressed;
};

static GParamSpec *alt_pressed_param;
static GSList *alt_monitors;

G_DEFINE_TYPE(AltMonitor, alt_monitor, G_TYPE_OBJECT);

static GdkFilterReturn alt_monitor_event_filter(GdkXEvent *xevent, GdkEvent *event,
                                                gpointer user_data)
{
	XAnyEvent *any_event = (XAnyEvent *)xevent;
	AltMonitor *monitor  = NULL;

	/* This is all a little bit silly.  GDK has no per-GdkDisplay event
	 * filters, so we have no chance to attach the user-data per-display.
	 *
	 * Instead, we have to scan our list for the correct display.
	 *
	 * In practice, this is going to be quite fast... :)
	 */
	{
		GSList *node;

		for (node = alt_monitors; node; node = node->next)
		{
			AltMonitor *item = node->data;

			if (GDK_DISPLAY_XDISPLAY(item->display) == any_event->display)
			{
				monitor = item;
				break;
			}
		}
	}

	if (monitor != NULL && monitor->xkb_event_type != 0 &&
	    monitor->xkb_event_type == any_event->type)
	{
		XkbAnyEvent *xkb_event = (XkbAnyEvent *)any_event;

		if (xkb_event->xkb_type == XkbStateNotify)
		{
			XkbStateNotifyEvent *sn_event = (XkbStateNotifyEvent *)xkb_event;
			gboolean is_pressed;

			is_pressed = (sn_event->mods & Mod1Mask) != 0;

			if (is_pressed != monitor->is_pressed)
			{
				monitor->is_pressed = is_pressed;
				g_object_notify_by_pspec(G_OBJECT(monitor), alt_pressed_param);
			}
		}
	}

	return GDK_FILTER_CONTINUE;
}

static void alt_monitor_get_property(GObject *object, guint prop_id, GValue *value,
                                     GParamSpec *pspec)
{
	AltMonitor *monitor = ALT_MONITOR(object);
	g_assert(pspec == alt_pressed_param);

	g_value_set_boolean(value, monitor->is_pressed);
}

static void alt_monitor_finalize(GObject *object)
{
	AltMonitor *monitor = ALT_MONITOR(object);

	alt_monitors = g_slist_remove(alt_monitors, monitor);
	g_object_unref(monitor->display);

	/* If this is the last one, remove the filter */
	if (alt_monitors == NULL)
		gdk_window_remove_filter(NULL, alt_monitor_event_filter, NULL);

	G_OBJECT_CLASS(alt_monitor_parent_class)->finalize(object);
}

static void alt_monitor_init(AltMonitor *monitor)
{
}

static void alt_monitor_class_init(AltMonitorClass *class)
{
	class->finalize     = alt_monitor_finalize;
	class->get_property = alt_monitor_get_property;

	alt_pressed_param = g_param_spec_boolean("alt-pressed",
	                                         "alt key pressed",
	                                         "TRUE if 'Alt' is currently pressed",
	                                         TRUE,
	                                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(class, 1, alt_pressed_param);
}

static gint alt_monitor_get_xkb_event_type(Display *dpy)
{
	gint major = XkbMajorVersion;
	gint minor = XkbMinorVersion;
	gint event_type;

	if (!XkbLibraryVersion(&major, &minor))
		return 0;

	if (!XkbQueryExtension(dpy, NULL, &event_type, NULL, &major, &minor))
		return 0;

	return event_type;
}

static AltMonitor *alt_monitor_new(GdkDisplay *display)
{
	AltMonitor *monitor;
	Display *dpy;

	dpy = GDK_DISPLAY_XDISPLAY(display);

	monitor                 = g_object_new(ALT_MONITOR_TYPE, NULL);
	monitor->display        = g_object_ref(display);
	monitor->xkb_event_type = alt_monitor_get_xkb_event_type(dpy);

	if (monitor->xkb_event_type)
	{
		XkbStateRec state;

		/* Since we have no way of knowing if someone else comes along
		 * after us and requests XkbModifierStateMask, we can never undo
		 * this...
		 */
		XkbSelectEventDetails(dpy,
		                      XkbUseCoreKbd,
		                      XkbStateNotify,
		                      XkbModifierStateMask,
		                      XkbModifierStateMask);

		/* Get the initial state in case 'Alt' was pressed to start. */
		XkbGetState(dpy, XkbUseCoreKbd, &state);

		monitor->is_pressed = (state.mods & Mod1Mask) != 0;
	}

	return monitor;
}

/**
 * alt_monitor_get_for_display:
 * @display: a #GdkDisplay
 *
 * Gets the #AltMonitor for @display.
 *
 * If an #AltMonitor already exists for @display, it is returned.
 * Otherwise, one is created.
 *
 * In any case, you get a reference.  Use g_object_unref() when you're
 * done.
 *
 * Returns: (transfer full): the #AltMonitor for @display
 **/
AltMonitor *alt_monitor_get_for_display(GdkDisplay *display)
{
	AltMonitor *monitor;
	GSList *node;

	for (node = alt_monitors; node; node = node->next)
	{
		monitor = node->data;

		if (monitor->display == display)
			return g_object_ref(monitor);
	}

	monitor = alt_monitor_new(display);

	/* if this is the first one, add the filter */
	if (alt_monitors == NULL)
		gdk_window_add_filter(NULL, alt_monitor_event_filter, NULL);

	alt_monitors = g_slist_prepend(alt_monitors, monitor);

	return monitor;
}

/**
 * alt_monitor_get_alt_pressed:
 * @monitor: an #AltMonitor
 *
 * Checks if the 'Alt' key is pressed.
 *
 * Returns: %TRUE if 'Alt' is currently being held down
 **/
gboolean alt_monitor_get_alt_pressed(AltMonitor *monitor)
{
	g_return_val_if_fail(IS_ALT_MONITOR(monitor), FALSE);

	return monitor->is_pressed;
}
