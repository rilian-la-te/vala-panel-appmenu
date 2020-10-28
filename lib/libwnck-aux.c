/*
 * vala-panel
 * Copyright (C) 2020 Konstantin Pugin <ria.freelander@gmail.com>
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

#include <X11/Xatom.h>
#include <gdk/gdkx.h>

#include "libwnck-aux.h"

static Display *get_xdisplay(gboolean *opened)
{
	Display *xdisplay;
	xdisplay = gdk_x11_get_default_xdisplay();

	if (opened)
		*opened = FALSE;

	if (!xdisplay)
	{
		xdisplay = XOpenDisplay(NULL);

		if (xdisplay)
		{
			if (opened)
				*opened = TRUE;
		}
	}

	return xdisplay;
}

static void libwnck_aux_get_string_window_hint(ulong xid, const char *atom_name, char **return_hint)
{
	Display *XDisplay;
	gint format;
	gulong numItems;
	gulong bytesAfter;
	Atom type;
	unsigned char *buffer;
	gboolean close_display;

	if (return_hint)
		*return_hint = NULL;

	g_return_if_fail(xid != 0);
	g_return_if_fail(return_hint);

	close_display = FALSE;
	XDisplay      = get_xdisplay(&close_display);

	if (!XDisplay)
	{
		g_warning("%s: Unable to get a valid XDisplay", G_STRFUNC);
		return;
	}

	gdk_x11_display_error_trap_push(GDK_X11_DISPLAY(gdk_display_get_default()));

	int result = XGetWindowProperty(XDisplay,
	                                xid,
	                                gdk_x11_get_xatom_by_name(atom_name),
	                                0,
	                                G_MAXINT,
	                                False,
	                                AnyPropertyType,
	                                &type,
	                                &format,
	                                &numItems,
	                                &bytesAfter,
	                                &buffer);

	bool x_error = gdk_x11_display_error_trap_pop(GDK_X11_DISPLAY(gdk_display_get_default()));

	if (close_display)
		XCloseDisplay(XDisplay);

	if (x_error)
	{
		XFree(buffer);
		return;
	}

	if (result == Success && numItems > 0)
	{
		if (return_hint && buffer && buffer[0] != '\0')
		{
			if (type == XA_STRING || type == gdk_x11_get_xatom_by_name("UTF8_STRING"))
				*return_hint = g_strdup((char *)buffer);
		}

		XFree(buffer);
	}
}

/**
 * Obtain utf8 property for a given window
 */
char *libwnck_aux_get_utf8_prop(ulong window, const char *prop)
{
	char *ret;
	libwnck_aux_get_string_window_hint(window, prop, &ret);
	return ret;
}

GDesktopAppInfo *libwnck_aux_match_wnck_window(ValaPanelMatcher *self, WnckWindow *window)
{
	if (!window)
		return NULL;
	ulong xid               = wnck_window_get_xid(window);
	int64_t pid             = wnck_window_get_pid(window);
	const char *cls_name    = wnck_window_get_class_instance_name(window);
	const char *grp_name    = wnck_window_get_class_group_name(window);
	g_autofree char *gtk_id = libwnck_aux_get_utf8_prop(xid, "_GTK_APPLICATION_ID");
	return vala_panel_matcher_match_arbitrary(self, cls_name, grp_name, gtk_id, pid);
}
