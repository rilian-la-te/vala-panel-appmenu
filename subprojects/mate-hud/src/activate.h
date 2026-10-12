#pragma once
#include <glib.h>

/* Apply placement changes to an already open HUD without stealing focus. */
void hud_update_placement(const char *attach_to, const char *location);

/*
 * Attempt to show the HUD for the given window.
 *
 * This function:
 *   1. Collects menu items for xid.
 *   2. Loads history.
 *   3. Shows the HUD dialog.
 *   4. Runs a nested gtk_main() until the dialog closes.
 *
 * If no menu is available or it is empty, returns immediately without showing
 * any window.
 *
 * wm_class: obtained from XGetClassHint; may be NULL.
 * history_max: from GSettings.
 */
void hud_activate(gulong xid,
                  guint32 timestamp,
                  const char *wm_class,
                  int history_max,
                  const char *attach_to,
                  const char *location);

/*
 * Data passed to the idle callback that defers activation to the main loop.
 */
typedef struct {
    gulong   xid;
    guint32  timestamp;
    char    *wm_class;
    int      history_max;
    char    *attach_to;
    char    *location;
    GDestroyNotify started;
    gpointer       started_data;
    GDestroyNotify finished;
    gpointer       finished_data;
} HudActivateData;

/* Free a HudActivateData and its owned fields. */
void hud_activate_data_free(HudActivateData *data);

/* GLib idle source callback; pair with hud_activate_data_free as destroy notify. */
gboolean hud_activate_idle(gpointer data);
