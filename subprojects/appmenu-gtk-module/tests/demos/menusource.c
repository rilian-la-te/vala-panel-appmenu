/*
 * Copyright © 2011 Canonical Limited
 *
 * All rights reserved.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include "menusource.h"

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <libwnck/libwnck.h>

typedef enum
{
	MENU_BAR_MODE_GLOBAL,
	MENU_BAR_MODE_WHEN_MAXIMIZED
} MenuBarMode;

enum
{
	XPROP_BUS_NAME,
	XPROP_OBJECT_PATH,
	N_XPROP
};

typedef GObjectClass MenuSourceClass;
struct _MenuSource
{
	GObject parent_instance;

	GDBusConnection *session;

	GdkScreen *gdk_screen;
	WnckScreen *screen;
	gulong active_window_changed_id;
	WnckWindow *active_window;
	gulong state_changed_id;
	GdkWindow *gdk_window;
	gboolean maximised;
	gchar *properties[N_XPROP];

	GActionGroup *actions;
	GMenuModel *menu;
};

G_DEFINE_TYPE(MenuSource, menu_source, G_TYPE_OBJECT)

static GParamSpec *menu_source_menu_param;
static GSettings *menubar_mode_settings;
static MenuBarMode menubar_mode;
static GSList *menu_sources;
static Atom atoms[N_XPROP];

static void menu_source_update(MenuSource *menu_source)
{
	g_print("msu\n");
	if (menu_source->menu)
		g_object_unref(menu_source->menu);

	if (menu_source->properties[0] && menu_source->properties[1])
	{
		GDBusMenuModel *proxy;

		proxy             = g_dbus_menu_model_get(menu_source->session,
                                              menu_source->properties[0],
                                              menu_source->properties[1]);
		menu_source->menu = G_MENU_MODEL(proxy);
	}
	else
		menu_source->menu = NULL;

	g_object_notify_by_pspec(G_OBJECT(menu_source), menu_source_menu_param);
}

static void menubar_mode_changed(GSettings *settings, const gchar *key, gpointer user_data)
{
	MenuBarMode new_mode;

	new_mode = g_settings_get_enum(settings, "menubar-mode");

	if (new_mode != menubar_mode)
	{
		GSList *node;

		menubar_mode = new_mode;

		for (node = menu_sources; node; node = node->next)
			menu_source_update(node->data);
	}
}

gboolean menu_source_is_maximised(WnckWindowState state)
{
	WnckWindowState maximised_mask;

	maximised_mask =
	    WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY | WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY;

	return (state & maximised_mask) == maximised_mask;
}

static void menu_source_state_changed(WnckWindow *window, WnckWindowState changed_mask,
                                      WnckWindowState new_state, gpointer user_data)
{
	MenuSource *menu_source = user_data;
	gboolean maximised;

	maximised = menu_source_is_maximised(new_state);

	if (maximised != menu_source->maximised)
	{
		menu_source->maximised = maximised;

		if (menubar_mode == MENU_BAR_MODE_WHEN_MAXIMIZED)
			menu_source_update(menu_source);
	}
}

static gboolean menu_source_update_window_property(MenuSource *menu_source, gint atom_index,
                                                   gboolean deleted)
{
	gchar *value = NULL;
	gboolean changed;

	g_print("uwp %d\n", deleted);

	if (!deleted)
	{
		gchar *name = "UTF8_STRING";
		unsigned long bytes_after_return;
		unsigned long nitems_return;
		unsigned char *prop_return;
		gint actual_format_return;
		Atom actual_type_return;
		Atom utf8_atom;

		XInternAtoms(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
		             &name,
		             1,
		             False,
		             &utf8_atom);

		if (XGetWindowProperty(GDK_SCREEN_XDISPLAY(menu_source->gdk_screen),
		                       gdk_x11_window_get_xid(menu_source->gdk_window),
		                       atoms[atom_index],
		                       0,
		                       1024,
		                       False,
		                       utf8_atom,
		                       &actual_type_return,
		                       &actual_format_return,
		                       &nitems_return,
		                       &bytes_after_return,
		                       &prop_return) == Success)
		{
			if (actual_format_return == 8 && bytes_after_return == 0)
			{
				g_print("I see %s\n", value);
				value = g_strndup((gchar *)prop_return, nitems_return);
			}

			XFree(prop_return);
		}
	}

	changed = g_strcmp0(menu_source->properties[atom_index], value) != 0;
	g_free(menu_source->properties[atom_index]);
	menu_source->properties[atom_index] = value;

	return changed;
}

static GdkFilterReturn window_filter_func(GdkXEvent *xevent, GdkEvent *event, gpointer user_data)
{
	MenuSource *menu_source = user_data;
	XAnyEvent *any          = (XAnyEvent *)xevent;

	if (any->type == PropertyNotify)
	{
		XPropertyEvent *property = (XPropertyEvent *)xevent;
		gint i;

		for (i = 0; i < N_XPROP; i++)
		{
			if (property->atom != atoms[i])
				continue;

			if (menu_source_update_window_property(menu_source,
			                                       i,
			                                       property->state == PropertyDelete))
				menu_source_update(menu_source);

			break;
		}
	}

	return GDK_FILTER_CONTINUE;
}

static void menu_source_active_window_changed(WnckScreen *screen,
                                              WnckWindow *previously_active_window,
                                              gpointer user_data)
{
	MenuSource *menu_source = user_data;
	WnckWindow *active_window;

	active_window = wnck_screen_get_active_window(screen);

	if (active_window != menu_source->active_window)
	{
		gboolean changed = FALSE;
		gint i;

		if (menu_source->active_window != NULL)
		{
			gdk_window_remove_filter(menu_source->gdk_window,
			                         window_filter_func,
			                         menu_source);
			g_object_unref(menu_source->gdk_window);
			menu_source->gdk_window = NULL;

			g_signal_handler_disconnect(menu_source->active_window,
			                            menu_source->state_changed_id);
			g_object_unref(menu_source->active_window);
			menu_source->active_window = NULL;
		}

		if (active_window != NULL)
		{
			Window xid;

			menu_source->active_window = g_object_ref(active_window);
			menu_source->state_changed_id =
			    g_signal_connect(active_window,
			                     "state-changed",
			                     G_CALLBACK(menu_source_state_changed),
			                     menu_source);
			menu_source->maximised =
			    menu_source_is_maximised(wnck_window_get_state(active_window));

			xid = wnck_window_get_xid(active_window);
			menu_source->gdk_window =
			    gdk_x11_window_foreign_new_for_display(gdk_display_get_default(), xid);
			gdk_window_add_filter(menu_source->gdk_window,
			                      window_filter_func,
			                      menu_source);
		}

		for (i = 0; i < N_XPROP; i++)
			changed |= menu_source_update_window_property(menu_source,
			                                              i,
			                                              active_window == NULL);

		if (changed)
			menu_source_update(menu_source);
	}
}

static void menu_source_get_property(GObject *object, guint prop_id, GValue *value,
                                     GParamSpec *pspec)
{
	MenuSource *menu_source = MENU_SOURCE(object);

	g_assert(prop_id == 1);

	g_value_set_object(value, menu_source->menu);
}

static void menu_source_finalize(GObject *object)
{
	MenuSource *menu_source = MENU_SOURCE(object);

	menu_sources = g_slist_remove(menu_sources, menu_source);

	/* If this is the last one, stop watching GSettings */
	if (menu_sources == NULL)
	{
		/* no need to remove the signal handler -- this is the only ref */
		g_object_unref(menubar_mode_settings);
		menubar_mode_settings = NULL;
	}

	if (menu_source->screen != NULL)
	{
		g_signal_handler_disconnect(menu_source->screen,
		                            menu_source->active_window_changed_id);
		g_object_unref(menu_source->screen);
	}

	if (menu_source->active_window != NULL)
	{
		g_signal_handler_disconnect(menu_source->active_window,
		                            menu_source->state_changed_id);
		g_object_unref(menu_source->active_window);
	}

	g_object_unref(menu_source->gdk_screen);
	g_object_unref(menu_source->screen);

	G_OBJECT_CLASS(menu_source_parent_class)->finalize(object);
}

static void menu_source_init(MenuSource *menu_source)
{
}

static void menu_source_class_init(MenuSourceClass *class)
{
	class->get_property = menu_source_get_property;
	class->finalize     = menu_source_finalize;

	menu_source_menu_param = g_param_spec_object("menu",
	                                             "active menu",
	                                             "the GMenuModel for the active window",
	                                             G_TYPE_MENU_MODEL,
	                                             G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(class, 1, menu_source_menu_param);
}

static MenuSource *menu_source_new(GdkScreen *screen)
{
	MenuSource *menu_source;

	menu_source             = g_object_new(MENU_SOURCE_TYPE, NULL);
	menu_source->session    = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
	menu_source->gdk_screen = g_object_ref(screen);
	menu_source->screen     = g_object_ref(wnck_screen_get(gdk_screen_get_number(screen)));
	g_signal_connect(menu_source->screen,
	                 "active-window-changed",
	                 G_CALLBACK(menu_source_active_window_changed),
	                 menu_source);
	menu_source_active_window_changed(menu_source->screen, NULL, menu_source);

	return menu_source;
}

GMenuModel *menu_source_get_menu(MenuSource *menu_source)
{
	g_return_val_if_fail(IS_MENU_SOURCE(menu_source), NULL);

	return menu_source->menu;
}

MenuSource *menu_source_get_for_screen(GdkScreen *screen)
{
	MenuSource *menu_source;
	GSList *node;

	for (node = menu_sources; node; node = node->next)
	{
		menu_source = node->data;

		if (menu_source->gdk_screen == screen)
			return g_object_ref(menu_source);
	}

	g_return_val_if_fail(GDK_IS_SCREEN(screen), NULL);
	g_return_val_if_fail(gdk_screen_get_display(screen) == gdk_display_get_default(), NULL);

	menu_source = menu_source_new(screen);

	/* if this is the first one, create the GSettings */
	if (menu_sources == NULL)
	{
		// menubar_mode_settings = g_settings_new ("com.canonical.Unity.GtkModule");
		// g_signal_connect (menubar_mode_settings, "changed::menubar-mode", G_CALLBACK
		// (menubar_mode_changed), NULL);
		// menubar_mode = g_settings_get_enum (menubar_mode_settings, "menubar-mode");
	}
	menubar_mode = MENU_BAR_MODE_GLOBAL;

	menu_sources = g_slist_prepend(menu_sources, menu_source);

	if (atoms[0] == 0)
	{
		const gchar *const names[] = { "_GTK_UNIQUE_BUS_NAME", "_GTK_MENUBAR_OBJECT_PATH" };
		G_STATIC_ASSERT(G_N_ELEMENTS(names) == G_N_ELEMENTS(atoms));
		XInternAtoms(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
		             (char **)names,
		             2,
		             False,
		             atoms);
	}

	return menu_source;
}
