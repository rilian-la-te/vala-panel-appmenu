/*
 * Copyright © 2011 Canonical Limited
 *
 * All rights reserved.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include "altgrabber.h"

#include <X11/XKBlib.h>
#include <gdk/gdkx.h>

/**
 * SECTION:altgrabber
 * @title: AltGrabber
 * @short_description: a utility class to make grabs for keys
 *
 * #AltGrabber is a small utility class for establishing global grabs on
 * particular Alt+letter key sequences.  Letters to grab are added with
 * alt_grabber_add_unichar() and the list of grabs is cleared with
 * alt_grabber_clear().
 *
 * When one of the grabbed key sequences is used, the "activated" signal
 * is emitted.
 **/

/**
 * AltGrabber:
 *
 * AltGrabber is an opaque structure type with no public fields.
 **/

typedef GObjectClass AltGrabberClass;
struct _AltGrabber
{
	GObject parent_instance;

	GHashTable *grabs;
	guint finish_clear_id;

	GdkScreen *screen;
	GdkKeymap *keymap;
	GdkWindow *root;
};

typedef struct
{
	gunichar c;
	gint ref_count;

	AltGrabberCallback callback;
	gpointer user_data;
	GDestroyNotify notify;
} KeyGrab;

static GSList *alt_grabbers;

G_DEFINE_TYPE(AltGrabber, alt_grabber, G_TYPE_OBJECT);

static GdkFilterReturn alt_grabber_event_filter(GdkXEvent *xevent, GdkEvent *event,
                                                gpointer user_data)
{
	XAnyEvent *any_event = (XAnyEvent *)xevent;
	AltGrabber *grabber  = user_data;

	if (any_event->type == KeyPress)
	{
		XKeyEvent *key_event = (XKeyEvent *)any_event;
		guint keycode        = key_event->keycode;
		GSList *grabs;

		grabs = g_hash_table_lookup(grabber->grabs, GINT_TO_POINTER(keycode));

		while (grabs)
		{
			KeyGrab *grab = grabs->data;

			(*grab->callback)(grabber, grab->c, grab->user_data);

			grabs = grabs->next;
		}
	}

	return GDK_FILTER_CONTINUE;
}

static gboolean alt_grabber_finish_clear(gpointer data)
{
	AltGrabber *grabber = data;
	GHashTableIter iter;
	gpointer key, value;
	Display *dpy;
	Window root;

	dpy  = GDK_DISPLAY_XDISPLAY(gdk_screen_get_display(grabber->screen));
	root = GDK_WINDOW_XID(grabber->root);

	gdk_error_trap_push();

	g_hash_table_iter_init(&iter, grabber->grabs);
	while (g_hash_table_iter_next(&iter, &key, &value))
		if (value == NULL)
		{
			guint keycode = GPOINTER_TO_INT(key);
			guint mask;

			for (mask = 0; mask < 0x100; mask++)
				if ((mask & (ShiftMask | ControlMask | Mod1Mask)) == Mod1Mask)
					XUngrabKey(dpy, keycode, mask, root);

			g_hash_table_iter_remove(&iter);
		}

	gdk_error_trap_pop_ignored();

	grabber->finish_clear_id = 0;
	return FALSE;
}

static void alt_grabber_finalize(GObject *object)
{
	AltGrabber *grabber = ALT_GRABBER(object);

	alt_grabber_clear(grabber);

	if (grabber->finish_clear_id != 0)
	{
		g_source_remove(grabber->finish_clear_id);
		alt_grabber_finish_clear(grabber);
	}

	alt_grabbers = g_slist_remove(alt_grabbers, grabber);

	g_assert(g_hash_table_size(grabber->grabs) == 0);
	g_hash_table_unref(grabber->grabs);

	g_object_unref(grabber->screen);
	g_object_unref(grabber->keymap);

	gdk_window_remove_filter(grabber->root, alt_grabber_event_filter, grabber);
	g_object_unref(grabber->root);

	G_OBJECT_CLASS(alt_grabber_parent_class)->finalize(object);
}

static void alt_grabber_init(AltGrabber *grabber)
{
	grabber->grabs = g_hash_table_new(NULL, NULL);
}

static void alt_grabber_class_init(AltGrabberClass *class)
{
	class->finalize = alt_grabber_finalize;
}

static AltGrabber *alt_grabber_new(GdkScreen *screen)
{
	AltGrabber *grabber;
	GdkDisplay *display;

	display = gdk_screen_get_display(screen);

	grabber         = g_object_new(ALT_GRABBER_TYPE, NULL);
	grabber->keymap = g_object_ref(gdk_keymap_get_for_display(display));
	grabber->root   = g_object_ref(gdk_screen_get_root_window(screen));
	grabber->screen = g_object_ref(screen);
	gdk_window_add_filter(grabber->root, alt_grabber_event_filter, grabber);

	/* We can not safely reverse this... */
	XSelectInput(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XID(grabber->root), KeyPressMask);

	return grabber;
}

/**
 * alt_grabber_get_for_screen:
 * @screen: a #GdkScreen
 *
 * Gets the #AltGrabber for @screen.
 *
 * If an #AltGrabber already exists for @screen, it is returned.
 * Otherwise, one is created.
 *
 * In any case, you get a reference.  Use g_object_unref() when you're
 * done.
 *
 * Returns: (transfer full): the #AltGrabber for @screen
 **/
AltGrabber *alt_grabber_get_for_screen(GdkScreen *screen)
{
	AltGrabber *grabber;
	GSList *node;

	for (node = alt_grabbers; node; node = node->next)
	{
		grabber = node->data;

		if (grabber->screen == screen)
			return g_object_ref(grabber);
	}

	grabber      = alt_grabber_new(screen);
	alt_grabbers = g_slist_prepend(alt_grabbers, grabber);

	return grabber;
}

/**
 * alt_grabber_clear:
 * @grabber: an #AltGrabber
 *
 * Clears all grab requests on @grabber.
 **/
void alt_grabber_clear(AltGrabber *grabber)
{
	gboolean clear_needed = FALSE;
	GHashTableIter iter;
	gpointer value;

	g_hash_table_iter_init(&iter, grabber->grabs);
	while (g_hash_table_iter_next(&iter, NULL, &value))
	{
		GSList *list = value;

		while (list)
		{
			KeyGrab *grab = list->data;

			if (0 == --grab->ref_count)
			{
				if (grab->notify)
					(*grab->notify)(grab->user_data);
				g_slice_free(KeyGrab, grab);
			}

			list = g_slist_remove_link(list, list);
		}

		g_hash_table_iter_replace(&iter, NULL);
		clear_needed = TRUE;
	}

	if (clear_needed && grabber->finish_clear_id == 0)
		grabber->finish_clear_id = g_idle_add(alt_grabber_finish_clear, grabber);
}

/**
 * alt_grabber_add_unichar:
 * @grabber: an #AltGrabber
 * @c: the #gunichar to grab
 * @callback: the callback to call when the character is pressed
 * @user_data: the user_data parameter for @callback
 * @notify: the #GDestroyNotift for @user_data, or %NULL
 *
 * Adds a new grab to @grabber.
 *
 * When Alt+@c is pressed, @callback will be called with @user_data.
 *
 * The effect of this call can be reversed with alt_grabber_clear().
 **/
void alt_grabber_add_unichar(AltGrabber *grabber, gunichar c, AltGrabberCallback callback,
                             gpointer user_data, GDestroyNotify notify)
{
	GdkKeymapKey *codes;
	gint n_codes;
	guint keyval;

	keyval = gdk_unicode_to_keyval(c);
	keyval = gdk_keyval_to_lower(keyval);

	if (keyval != 0 &&
	    gdk_keymap_get_entries_for_keyval(grabber->keymap, keyval, &codes, &n_codes))
	{
		KeyGrab *grab;
		Display *dpy;
		Window root;
		gint i;

		g_assert(n_codes > 0);

		grab            = g_slice_new(KeyGrab);
		grab->c         = c;
		grab->ref_count = n_codes;
		grab->callback  = callback;
		grab->user_data = user_data;
		grab->notify    = notify;

		dpy  = GDK_DISPLAY_XDISPLAY(gdk_screen_get_display(grabber->screen));
		root = GDK_WINDOW_XID(grabber->root);

		gdk_error_trap_push();
		for (i = 0; i < n_codes; i++)
		{
			gpointer list = NULL;

			if (!g_hash_table_lookup_extended(grabber->grabs,
			                                  GINT_TO_POINTER(codes[i].keycode),
			                                  NULL,
			                                  &list))
			{
				guint mask;

				for (mask = 0; mask < 0x100; mask++)
					if ((mask & (ShiftMask | ControlMask | Mod1Mask)) ==
					    Mod1Mask)
						XGrabKey(dpy,
						         codes[i].keycode,
						         mask,
						         root,
						         False,
						         GrabModeAsync,
						         GrabModeAsync);
			}

			list = g_slist_prepend(list, grab);
			g_hash_table_insert(grabber->grabs,
			                    GINT_TO_POINTER(codes[i].keycode),
			                    list);
		}

		if (gdk_error_trap_pop())
		{
			gchar outbuf[8];
			gint s;

			s         = g_unichar_to_utf8(c, outbuf);
			outbuf[s] = '\0';

			g_warning("failed to acquire (some) keyboard grabs for '%s'", outbuf);
		}
	}
	else
	{
		if (notify)
			(*notify)(user_data);
	}
}
