#pragma once
#include "model.h"
#include <glib.h>

/*
 * Harvest menu items from an application exposing a GMenuModel menubar via
 * the standard GTK D-Bus protocol (org.gtk.Menus + org.gtk.Actions).
 *
 * xid: the active window's X11 window ID.
 * The function reads the required X11 window properties to find the bus name
 * and object paths.
 *
 * Returns a GPtrArray of MenuEntry* or NULL on failure/empty.
 * Caller takes ownership.
 */
GPtrArray *collect_menumodel(gulong xid);
