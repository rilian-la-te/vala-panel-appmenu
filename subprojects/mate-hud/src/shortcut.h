#pragma once
#include "activate.h"
#include <glib.h>
#include <X11/Xlib.h>

/*
 * Shortcut grab state.  Opaque; create with shortcut_new().
 */
typedef struct ShortcutGrab ShortcutGrab;

/*
 * Create a new ShortcutGrab.
 * xdpy: persistent X display connection (owned by caller; must outlive grab).
 * When the shortcut fires, the grab schedules hud_activate_idle() with a
 * heap-allocated HudActivateData that is cleaned up by
 * hud_activate_data_free().
 */
ShortcutGrab *shortcut_new(Display *xdpy);

/*
 * Install a grab for the given GDK accelerator string (for example
 * "<Alt>space"). Modifier-less shortcuts are rejected for safety.
 * If a grab is already active it is released first.
 * Returns FALSE if the accelerator string is invalid or the grab fails.
 */
gboolean shortcut_grab(ShortcutGrab *sg, const char *accelerator,
                       const char *wm_class, int history_max);

/*
 * Release all active grabs.
 */
void shortcut_ungrab(ShortcutGrab *sg);

/*
 * Free the ShortcutGrab (calls shortcut_ungrab first).
 */
void shortcut_free(ShortcutGrab *sg);
