#pragma once
#include "model.h"
#include <gtk/gtk.h>

typedef struct HudWindow HudWindow;

/*
 * Create the HUD dialog window.
 * entries: immutable snapshot, must outlive the window.
 * wm_class: for history; may be NULL.
 * history_max: from GSettings hud-history-max.
 * on_done: called when the window is about to be destroyed (success or cancel).
 */
HudWindow *hud_window_new(GPtrArray *entries,
                          const char *wm_class,
                          int history_max,
                          const char *attach_to,
                          const char *location);

/* Replace the displayed menu snapshot in an existing HUD window. */
void hud_window_update(HudWindow *w,
                       GPtrArray *entries,
                       const char *wm_class,
                       int history_max,
                       const char *attach_to,
                       const char *location);

/* Update only placement preferences; keeps current search and menu snapshot. */
void hud_window_set_placement(HudWindow *w,
                              const char *attach_to,
                              const char *location);

/* Update the global-menu applet rectangle used by "applet" placement. */
void hud_window_set_applet_geometry(int x, int y, int width, int height);

/*
 * Present the window: realize → set X11 user time → show → present_with_time
 * → grab focus to search entry.
 */
void hud_window_present(HudWindow *w, guint32 timestamp, gulong target_xid);

/* Destroy the window and free the HudWindow struct. */
void hud_window_destroy(HudWindow *w);

/* Returns the underlying GtkWidget for the window. */
GtkWidget *hud_window_widget(HudWindow *w);

/* Returns the X11 window id for the HUD window, or 0 before realization. */
gulong hud_window_xid(HudWindow *w);

/* X server time at which focus loss dismissed the window, or 0. */
guint32 hud_window_dismiss_time(HudWindow *w);
