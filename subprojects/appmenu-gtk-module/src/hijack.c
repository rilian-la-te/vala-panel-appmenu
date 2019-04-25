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

#include <gtk/gtk.h>

#include <appmenu-gtk-action-group.h>

#include "consts.h"
#include "datastructs.h"
#include "hijack.h"
#include "platform.h"
#include "support.h"

static void (*pre_hijacked_window_realize)(GtkWidget *widget);

static void (*pre_hijacked_window_unrealize)(GtkWidget *widget);

#if GTK_MAJOR_VERSION == 3
static void (*pre_hijacked_application_window_realize)(GtkWidget *widget);
#endif

static void (*pre_hijacked_menu_bar_realize)(GtkWidget *widget);

static void (*pre_hijacked_menu_bar_unrealize)(GtkWidget *widget);

static void (*pre_hijacked_widget_size_allocate)(GtkWidget *widget, GtkAllocation *allocation);

static void (*pre_hijacked_menu_bar_size_allocate)(GtkWidget *widget, GtkAllocation *allocation);

#if GTK_MAJOR_VERSION == 2
static void (*pre_hijacked_menu_bar_size_request)(GtkWidget *widget, GtkRequisition *requisition);
#elif GTK_MAJOR_VERSION == 3
static void (*pre_hijacked_menu_bar_get_preferred_width)(GtkWidget *widget, gint *minimum_width,
                                                         gint *natural_width);

static void (*pre_hijacked_menu_bar_get_preferred_height)(GtkWidget *widget, gint *minimum_height,
                                                          gint *natural_height);

static void (*pre_hijacked_menu_bar_get_preferred_width_for_height)(GtkWidget *widget, gint height,
                                                                    gint *minimum_width,
                                                                    gint *natural_width);

static void (*pre_hijacked_menu_bar_get_preferred_height_for_width)(GtkWidget *widget, gint width,
                                                                    gint *minimum_height,
                                                                    gint *natural_height);
#endif

static void hijacked_window_realize(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_WINDOW(widget));

	GdkScreen *screen      = gtk_widget_get_screen(widget);
	GdkVisual *visual      = gdk_screen_get_rgba_visual(screen);
	GdkWindowTypeHint hint = gtk_window_get_type_hint(GTK_WINDOW(widget));
	bool is_hint_viable =
	    ((hint == GDK_WINDOW_TYPE_HINT_NORMAL) || (hint == GDK_WINDOW_TYPE_HINT_DIALOG));
	if (visual && (hint == GDK_WINDOW_TYPE_HINT_DND))
		gtk_widget_set_visual(widget, visual);

// In Wayland the DBUS Menu need to be register before realize the window.
#ifdef GDK_WINDOWING_WAYLAND
	if (GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default()) && is_hint_viable &&
	    (!GTK_IS_APPLICATION_WINDOW(GTK_WINDOW(widget))))
		gtk_window_get_window_data(GTK_WINDOW(widget));
#endif

	if (pre_hijacked_window_realize != NULL)
		pre_hijacked_window_realize(widget);

#ifdef GDK_WINDOWING_X11
	if (is_hint_viable
#if GTK_MAJOR_VERSION == 3
	    && GDK_IS_X11_DISPLAY(gdk_display_get_default()) && (!GTK_IS_APPLICATION_WINDOW(widget))
#endif
	)
		gtk_window_get_window_data(GTK_WINDOW(widget));
#endif
}

static void hijacked_window_unrealize(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_WINDOW(widget));

	if (pre_hijacked_window_unrealize != NULL)
		pre_hijacked_window_unrealize(widget);

	g_object_set_qdata(G_OBJECT(widget), window_data_quark(), NULL);
}

#if GTK_MAJOR_VERSION == 3
static void hijacked_application_window_realize(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_APPLICATION_WINDOW(widget));

#ifdef GDK_WINDOWING_WAYLAND
	if (GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default()))
		gtk_window_get_window_data(GTK_WINDOW(widget));
#endif

	if (pre_hijacked_application_window_realize != NULL)
		pre_hijacked_application_window_realize(widget);

#ifdef GDK_WINDOWING_X11
	if (GDK_IS_X11_DISPLAY(gdk_display_get_default()))
		gtk_window_get_window_data(GTK_WINDOW(widget));
#endif
}
#endif

static void hijacked_menu_bar_realize(GtkWidget *widget)
{
	GtkWidget *window;

	g_return_if_fail(GTK_IS_MENU_BAR(widget));

	if (pre_hijacked_menu_bar_realize != NULL)
		(*pre_hijacked_menu_bar_realize)(widget);

	window = gtk_widget_get_toplevel(widget);

	if (GTK_IS_WINDOW(window))
		gtk_window_connect_menu_shell(GTK_WINDOW(window), GTK_MENU_SHELL(widget));

	gtk_widget_connect_settings(widget);
}

static void hijacked_menu_bar_unrealize(GtkWidget *widget)
{
	MenuShellData *menu_shell_data;

	g_return_if_fail(GTK_IS_MENU_BAR(widget));

	menu_shell_data = gtk_menu_shell_get_menu_shell_data(GTK_MENU_SHELL(widget));

	gtk_widget_disconnect_settings(widget);

	if (menu_shell_data_has_window(menu_shell_data))
		gtk_window_disconnect_menu_shell(menu_shell_data_get_window(menu_shell_data),
		                                 GTK_MENU_SHELL(widget));

	if (pre_hijacked_menu_bar_unrealize != NULL)
		pre_hijacked_menu_bar_unrealize(widget);
}

static void hijacked_menu_bar_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	GtkAllocation zero = { 0, 0, 0, 0 };
	GdkWindow *window;

	g_return_if_fail(GTK_IS_MENU_BAR(widget));

	if (gtk_widget_shell_shows_menubar(widget))
	{
		/*
		 * We manually assign an empty allocation to the menu bar to
		 * prevent the container from attempting to draw it at all.
		 */
		if (pre_hijacked_widget_size_allocate != NULL)
			pre_hijacked_widget_size_allocate(widget, &zero);

		/*
		 * Then we move the GdkWindow belonging to the menu bar outside of
		 * the clipping rectangle of the parent window so that we can't
		 * see it.
		 */
		window = gtk_widget_get_window(widget);

		if (window != NULL)
			gdk_window_move_resize(window, -1, -1, 1, 1);
	}
	else if (pre_hijacked_menu_bar_size_allocate != NULL)
		pre_hijacked_menu_bar_size_allocate(widget, allocation);
}

#if GTK_MAJOR_VERSION == 2
static void hijacked_menu_bar_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	g_return_if_fail(GTK_IS_MENU_BAR(widget));

	if (pre_hijacked_menu_bar_size_request != NULL)
		pre_hijacked_menu_bar_size_request(widget, requisition);

	if (gtk_widget_shell_shows_menubar(widget))
	{
		requisition->width  = 0;
		requisition->height = 0;
	}
}
#elif GTK_MAJOR_VERSION == 3
static void hijacked_menu_bar_get_preferred_width(GtkWidget *widget, gint *minimum_width,
                                                  gint *natural_width)
{
	g_return_if_fail(GTK_IS_MENU_BAR(widget));

	if (pre_hijacked_menu_bar_get_preferred_width != NULL)
		pre_hijacked_menu_bar_get_preferred_width(widget, minimum_width, natural_width);

	if (gtk_widget_shell_shows_menubar(widget))
	{
		*minimum_width = 0;
		*natural_width = 0;
	}
}

static void hijacked_menu_bar_get_preferred_height(GtkWidget *widget, gint *minimum_height,
                                                   gint *natural_height)
{
	g_return_if_fail(GTK_IS_MENU_BAR(widget));

	if (pre_hijacked_menu_bar_get_preferred_height != NULL)
		pre_hijacked_menu_bar_get_preferred_height(widget, minimum_height, natural_height);

	if (gtk_widget_shell_shows_menubar(widget))
	{
		*minimum_height = 0;
		*natural_height = 0;
	}
}

static void hijacked_menu_bar_get_preferred_width_for_height(GtkWidget *widget, gint height,
                                                             gint *minimum_width,
                                                             gint *natural_width)
{
	g_return_if_fail(GTK_IS_MENU_BAR(widget));

	if (pre_hijacked_menu_bar_get_preferred_width_for_height != NULL)
		pre_hijacked_menu_bar_get_preferred_width_for_height(widget,
		                                                     height,
		                                                     minimum_width,
		                                                     natural_width);

	if (gtk_widget_shell_shows_menubar(widget))
	{
		*minimum_width = 0;
		*natural_width = 0;
	}
}

static void hijacked_menu_bar_get_preferred_height_for_width(GtkWidget *widget, gint width,
                                                             gint *minimum_height,
                                                             gint *natural_height)
{
	g_return_if_fail(GTK_IS_MENU_BAR(widget));

	if (pre_hijacked_menu_bar_get_preferred_height_for_width != NULL)
		pre_hijacked_menu_bar_get_preferred_height_for_width(widget,
		                                                     width,
		                                                     minimum_height,
		                                                     natural_height);

	if (gtk_widget_shell_shows_menubar(widget))
	{
		*minimum_height = 0;
		*natural_height = 0;
	}
}
#endif

static void hijack_window_class_vtable(GType type)
{
	GtkWidgetClass *widget_class = g_type_class_ref(type);
	GType *children;
	guint n;
	guint i;

	if (widget_class->realize == pre_hijacked_window_realize)
		widget_class->realize = hijacked_window_realize;

#if GTK_MAJOR_VERSION == 3
	if (widget_class->realize == pre_hijacked_application_window_realize)
		widget_class->realize = hijacked_application_window_realize;
#endif

	if (widget_class->unrealize == pre_hijacked_window_unrealize)
		widget_class->unrealize = hijacked_window_unrealize;

	children = g_type_children(type, &n);

	for (i = 0; i < n; i++)
		hijack_window_class_vtable(children[i]);

	g_free(children);
}

G_GNUC_INTERNAL void store_pre_hijacked()
{
	GtkWidgetClass *widget_class;
	/* store the base GtkWidget size_allocate vfunc */
	widget_class                      = g_type_class_ref(GTK_TYPE_WIDGET);
	pre_hijacked_widget_size_allocate = widget_class->size_allocate;

#if GTK_MAJOR_VERSION == 3
	/* store the base GtkApplicationWindow realize vfunc */
	widget_class                            = g_type_class_ref(GTK_TYPE_APPLICATION_WINDOW);
	pre_hijacked_application_window_realize = widget_class->realize;
#endif

	/* intercept window realize vcalls on GtkWindow */
	widget_class                  = g_type_class_ref(GTK_TYPE_WINDOW);
	pre_hijacked_window_realize   = widget_class->realize;
	pre_hijacked_window_unrealize = widget_class->unrealize;
	hijack_window_class_vtable(GTK_TYPE_WINDOW);

	/* intercept size request and allocate vcalls on GtkMenuBar (for hiding) */
	widget_class                        = g_type_class_ref(GTK_TYPE_MENU_BAR);
	pre_hijacked_menu_bar_realize       = widget_class->realize;
	pre_hijacked_menu_bar_unrealize     = widget_class->unrealize;
	pre_hijacked_menu_bar_size_allocate = widget_class->size_allocate;
#if GTK_MAJOR_VERSION == 2
	pre_hijacked_menu_bar_size_request = widget_class->size_request;
#elif GTK_MAJOR_VERSION == 3
	pre_hijacked_menu_bar_get_preferred_width  = widget_class->get_preferred_width;
	pre_hijacked_menu_bar_get_preferred_height = widget_class->get_preferred_height;
	pre_hijacked_menu_bar_get_preferred_width_for_height =
	    widget_class->get_preferred_width_for_height;
	pre_hijacked_menu_bar_get_preferred_height_for_width =
	    widget_class->get_preferred_height_for_width;
#endif
}
G_GNUC_INTERNAL void hijack_menu_bar_class_vtable(GType type)
{
	GtkWidgetClass *widget_class = g_type_class_ref(type);
	GType *children;
	guint n;
	guint i;

	/* This fixes lp:1113008. */
	widget_class->hierarchy_changed = NULL;

	if (widget_class->realize == pre_hijacked_menu_bar_realize)
		widget_class->realize = hijacked_menu_bar_realize;

	if (widget_class->unrealize == pre_hijacked_menu_bar_unrealize)
		widget_class->unrealize = hijacked_menu_bar_unrealize;

	if (widget_class->size_allocate == pre_hijacked_menu_bar_size_allocate)
		widget_class->size_allocate = hijacked_menu_bar_size_allocate;

#if GTK_MAJOR_VERSION == 2
	if (widget_class->size_request == pre_hijacked_menu_bar_size_request)
		widget_class->size_request = hijacked_menu_bar_size_request;
#elif GTK_MAJOR_VERSION == 3
	if (widget_class->get_preferred_width == pre_hijacked_menu_bar_get_preferred_width)
		widget_class->get_preferred_width = hijacked_menu_bar_get_preferred_width;

	if (widget_class->get_preferred_height == pre_hijacked_menu_bar_get_preferred_height)
		widget_class->get_preferred_height = hijacked_menu_bar_get_preferred_height;

	if (widget_class->get_preferred_width_for_height ==
	    pre_hijacked_menu_bar_get_preferred_width_for_height)
		widget_class->get_preferred_width_for_height =
		    hijacked_menu_bar_get_preferred_width_for_height;

	if (widget_class->get_preferred_height_for_width ==
	    pre_hijacked_menu_bar_get_preferred_height_for_width)
		widget_class->get_preferred_height_for_width =
		    hijacked_menu_bar_get_preferred_height_for_width;
#endif

	children = g_type_children(type, &n);

	for (i = 0; i < n; i++)
		hijack_menu_bar_class_vtable(children[i]);

	g_free(children);
}
