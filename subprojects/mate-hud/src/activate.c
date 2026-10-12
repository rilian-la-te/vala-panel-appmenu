#include "activate.h"
#include "collect.h"
#include "history.h"
#include "window.h"
#include <glib.h>

static gboolean hud_activation_in_progress = FALSE;
static HudWindow *current_window = NULL;
static GPtrArray *current_entries = NULL;
static gulong current_target_xid = 0;
static gulong dismissed_target_xid = 0;
static guint32 dismissed_time = 0;

/* Upper bound on how long a shortcut tap can be held, matching the helper's
 * TAP_TIMEOUT_MS. */
#define DISMISS_TOGGLE_WINDOW_MS 250

void hud_update_placement(const char *attach_to, const char *location,
                          int applet_x, int applet_y,
                          int applet_width, int applet_height)
{
    hud_window_set_applet_geometry(applet_x, applet_y,
                                   applet_width, applet_height);
    if (current_window)
        hud_window_set_placement(current_window, attach_to, location);
}

void hud_activate_data_free(HudActivateData *data)
{
    if (!data) return;
    g_free(data->wm_class);
    g_free(data->attach_to);
    g_free(data->location);
    g_free(data);
}

void hud_activate(gulong xid,
                  guint32 timestamp,
                  const char *wm_class,
                  int history_max,
                  const char *attach_to,
                  const char *location)
{
    if (!xid) {
        g_message("mate-hud: activation ignored because there is no active X window");
        return;
    }

    if (current_window) {
        gulong hud_xid = hud_window_xid(current_window);
        if (hud_xid != 0 && xid == hud_xid)
            xid = current_target_xid;
    }

    /* Pressing a bare-modifier shortcut while the HUD is open takes a
     * keyboard grab, which dismisses the HUD on focus-out. The helper only
     * sends Activate on release, by which time the WM has handed focus back
     * to the transient parent. Treat that activation as the closing half of
     * a toggle instead of reopening the HUD. */
    if (!current_window && timestamp && dismissed_time &&
        xid == dismissed_target_xid &&
        (guint32)(timestamp - dismissed_time) <= DISMISS_TOGGLE_WINDOW_MS) {
        g_message("mate-hud: activation ignored because it dismissed the HUD for xid=%lu",
                  xid);
        dismissed_time = 0;
        return;
    }
    dismissed_time = 0;

    if (hud_activation_in_progress) {
        g_message("mate-hud: activation ignored because another HUD session is in progress");
        return;
    }

    if (current_window && xid == current_target_xid) {
        g_message("mate-hud: re-presenting existing HUD for xid=%lu", xid);
        hud_window_set_placement(current_window, attach_to, location);
        hud_window_present(current_window, timestamp, xid);
        return;
    }

    hud_activation_in_progress = TRUE;
    g_message("mate-hud: activation started for xid=%lu wm_class=%s",
              xid, wm_class ? wm_class : "(unknown)");

    GPtrArray *entries = collect_menu_for_window(xid);
    if (!entries) {
        g_message("mate-hud: activation ended without showing a dialog for xid=%lu",
                  xid);
        hud_activation_in_progress = FALSE;
        return;
    }
    if (entries->len == 0) {
        g_ptr_array_unref(entries);
        g_message("mate-hud: collected zero entries for xid=%lu", xid);
        hud_activation_in_progress = FALSE;
        return;
    }

    history_load(entries, wm_class);
    if (current_window) {
        g_message("mate-hud: refreshing HUD with %u entries for xid=%lu",
                  entries->len, xid);
        hud_window_update(current_window, entries, wm_class, history_max,
                          attach_to, location);
        if (current_entries)
            g_ptr_array_unref(current_entries);
        current_entries = entries;
        current_target_xid = xid;
        hud_window_present(current_window, timestamp, xid);
        hud_activation_in_progress = FALSE;
        return;
    }

    g_message("mate-hud: presenting HUD with %u entries for xid=%lu",
              entries->len, xid);

    current_entries = entries;
    current_target_xid = xid;
    current_window = hud_window_new(current_entries, wm_class, history_max,
                                    attach_to, location);
    hud_window_present(current_window, timestamp, xid);
    hud_activation_in_progress = FALSE;

    /* Nested main loop: blocks until on_window_destroy calls gtk_main_quit() */
    gtk_main();

    dismissed_time = hud_window_dismiss_time(current_window);
    dismissed_target_xid = current_target_xid;

    /* Window was already destroyed by GTK; free our wrapper */
    hud_window_destroy(current_window);
    current_window = NULL;
    if (current_entries)
        g_ptr_array_unref(current_entries);
    current_entries = NULL;
    g_message("mate-hud: HUD session finished for xid=%lu", current_target_xid);
    current_target_xid = 0;
}

gboolean hud_activate_idle(gpointer data)
{
    HudActivateData *d = data;
    if (d->started)
        d->started(d->started_data);
    hud_activate(d->xid, d->timestamp, d->wm_class, d->history_max,
                 d->attach_to, d->location);
    if (d->finished)
        d->finished(d->finished_data);
    return G_SOURCE_REMOVE;
}
