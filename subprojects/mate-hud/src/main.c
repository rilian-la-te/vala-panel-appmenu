#include "activate.h"
#include "shortcut.h"
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <locale.h>
#include <string.h>

#define SCHEMA_ID   "org.valapanel.appmenu"
#define PANEL_SCHEMA_ID "org.mate.panel"
#define PANEL_OBJECT_SCHEMA_ID "org.mate.panel.object"
#define HUD_BUS     "org.mate.HUD"
#define HUD_OBJECT  "/org/mate/HUD"
#define PANEL_OBJECT_PATH_FMT "/org/mate/panel/objects/%s/"
#define PANEL_OBJECT_PREFS_PATH_FMT "/org/mate/panel/objects/%s/prefs/"

/* D-Bus introspection XML for org.mate.HUD */
static const char HUD_INTROSPECT[] =
"<node>"
"  <interface name='org.mate.HUD'>"
"    <method name='Activate'>"
"      <arg name='timestamp' direction='in' type='u'/>"
"    </method>"
"    <method name='Quit'/>"
"  </interface>"
"</node>";

typedef struct {
    GMainLoop    *loop;
    ShortcutGrab *grab;
    Display      *xdpy;
    GSettings    *settings;
    guint         bus_owner_id;
    gboolean      dbus_object_registered;
    /* cached prefs */
    gboolean      use_system_shortcut;
    char         *shortcut_str;
    int           history_max;
    char         *attach_to;
    char         *location;
    /* Activations in progress. Nested ones run inside an open HUD's main
     * loop; the helper stays stopped until the outermost one finishes. */
    guint         shortcut_suspend_depth;
} Daemon;

static void apply_shortcut_prefs(Daemon *d);

static void suspend_internal_shortcut(gpointer user_data)
{
    Daemon *d = user_data;
    if (d->shortcut_suspend_depth++ == 0)
        shortcut_ungrab(d->grab);
}

static void restore_internal_shortcut(gpointer user_data)
{
    Daemon *d = user_data;

    g_return_if_fail(d->shortcut_suspend_depth > 0);
    if (--d->shortcut_suspend_depth == 0 && !d->use_system_shortcut)
        apply_shortcut_prefs(d);
}

/* ── Helpers ────────────────────────────────────────────────────────────── */

static gboolean is_appmenu_applet_iid(const char *iid)
{
    if (!iid || !*iid) return FALSE;
    return g_strcmp0(iid, "AppmenuAppletFactory::AppmenuApplet") == 0 ||
           g_strcmp0(iid, "AppmenuApplet") == 0 ||
           strstr(iid, "AppmenuAppletFactory") != NULL;
}

static char *find_appmenu_settings_path(void)
{
    g_autofree char *override = g_strdup(g_getenv("MATE_HUD_SETTINGS_PATH"));
    if (override && *override) return g_steal_pointer(&override);

    g_autoptr(GSettings) panel_settings = g_settings_new(PANEL_SCHEMA_ID);
    g_auto(GStrv) object_ids = g_settings_get_strv(panel_settings, "object-id-list");
    if (!object_ids) return NULL;

    for (guint i = 0; object_ids[i] != NULL; i++) {
        const char *object_id = object_ids[i];
        g_autofree char *object_path =
            g_strdup_printf(PANEL_OBJECT_PATH_FMT, object_id);
        g_autoptr(GSettings) object_settings =
            g_settings_new_with_path(PANEL_OBJECT_SCHEMA_ID, object_path);
        g_autofree char *object_type =
            g_settings_get_string(object_settings, "object-type");
        g_autofree char *applet_iid =
            g_settings_get_string(object_settings, "applet-iid");

        if (g_strcmp0(object_type, "applet") == 0 &&
            is_appmenu_applet_iid(applet_iid)) {
            return g_strdup_printf(PANEL_OBJECT_PREFS_PATH_FMT, object_id);
        }
    }

    return NULL;
}

static Window read_active_xid(Display *dpy)
{
    Atom prop = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    if (prop == None) return 0;
    Atom type; int fmt; unsigned long n, after;
    unsigned char *data = NULL;
    Window xid = 0;
    if (XGetWindowProperty(dpy, DefaultRootWindow(dpy), prop,
                           0, 1, False, XA_WINDOW,
                           &type, &fmt, &n, &after, &data) == Success && data) {
        /* A zero-length property still returns a (1-byte) buffer. */
        if (n >= 1 && fmt == 32)
            xid = *(Window *)data;
        XFree(data);
    }
    return xid;
}

/* ── D-Bus method handler ────────────────────────────────────────────────── */

static void handle_dbus_method(GDBusConnection  *conn G_GNUC_UNUSED,
                                const char       *sender G_GNUC_UNUSED,
                                const char       *object_path G_GNUC_UNUSED,
                                const char       *iface G_GNUC_UNUSED,
                                const char       *method,
                                GVariant         *params,
                                GDBusMethodInvocation *invocation,
                                gpointer          user_data)
{
    Daemon *d = user_data;
    g_dbus_method_invocation_return_value(invocation, NULL);

    if (g_strcmp0(method, "Quit") == 0) {
        g_main_loop_quit(d->loop);
        return;
    }

    if (g_strcmp0(method, "Activate") == 0) {
        guint32 timestamp = 0;
        g_variant_get(params, "(u)", &timestamp);

        /* Snapshot active window IMMEDIATELY before any GTK window is created */
        Window xid = read_active_xid(d->xdpy);
        char *wm_class = NULL;
        if (xid) {
            XClassHint hint = {NULL, NULL};
            if (XGetClassHint(d->xdpy, xid, &hint)) {
                wm_class = g_strdup(hint.res_class ? hint.res_class : hint.res_name);
                if (hint.res_name)  XFree(hint.res_name);
                if (hint.res_class) XFree(hint.res_class);
            }
        }

        HudActivateData *data = g_new0(HudActivateData, 1);
        data->xid         = (gulong)xid;
        data->timestamp   = timestamp;
        data->wm_class    = wm_class;
        data->history_max = d->history_max;
        data->attach_to   = g_strdup(d->attach_to);
        data->location    = g_strdup(d->location);
        /* Tracked in either shortcut mode, so a switch to the internal one
         * while a HUD is open cannot start the helper underneath it. */
        data->started       = suspend_internal_shortcut;
        data->started_data  = d;
        data->finished      = restore_internal_shortcut;
        data->finished_data = d;
        g_idle_add_full(G_PRIORITY_HIGH, hud_activate_idle, data,
                        (GDestroyNotify)hud_activate_data_free);
    }
}

static const GDBusInterfaceVTable DBUS_VTABLE = {
    handle_dbus_method,
    NULL,
    NULL,
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

/* ── GSettings change handler ─────────────────────────────────────────────── */

static void apply_hud_prefs(Daemon *d)
{
    d->history_max = g_settings_get_int(d->settings, "hud-history-max");

    g_free(d->attach_to);
    g_free(d->location);
    d->attach_to = g_settings_get_string(d->settings, "hud-attach-to");
    d->location  = g_settings_get_string(d->settings, "hud-location");
    hud_update_placement(d->attach_to, d->location,
                         g_settings_get_int(d->settings, "hud-applet-x"),
                         g_settings_get_int(d->settings, "hud-applet-y"),
                         g_settings_get_int(d->settings, "hud-applet-width"),
                         g_settings_get_int(d->settings, "hud-applet-height"));
}

static void apply_shortcut_prefs(Daemon *d)
{
    gboolean use_sys = g_settings_get_boolean(d->settings, "hud-use-system-shortcut");
    char    *accel   = g_settings_get_string(d->settings, "hud-shortcut");

    apply_hud_prefs(d);

    if (use_sys) {
        shortcut_ungrab(d->grab);
        g_message("mate-hud: system shortcut mode enabled; internal key grabbing disabled");
    } else if (d->shortcut_suspend_depth > 0) {
        /* A HUD session is open; restore_internal_shortcut() grabs with
         * these settings once it ends. */
        g_message("mate-hud: internal shortcut '%s' will be grabbed after the HUD closes",
                  accel);
    } else {
        if (!shortcut_grab(d->grab, accel, NULL, d->history_max)) {
            g_warning("mate-hud: failed to grab shortcut '%s'", accel);
            shortcut_ungrab(d->grab);
        } else {
            g_message("mate-hud: grabbed internal shortcut '%s'", accel);
        }
    }

    g_free(d->shortcut_str);
    d->shortcut_str        = accel;
    d->use_system_shortcut = use_sys;
}

static void on_settings_changed(GSettings   *settings G_GNUC_UNUSED,
                                  const char  *key,
                                  gpointer     user_data)
{
    Daemon *d = user_data;

    if (g_strcmp0(key, "hud-enabled") == 0) {
        if (!g_settings_get_boolean(d->settings, "hud-enabled")) {
            g_main_loop_quit(d->loop);
        }
        return;
    }

    if (g_strcmp0(key, "hud-shortcut") == 0 ||
        g_strcmp0(key, "hud-use-system-shortcut") == 0) {
        apply_shortcut_prefs(d);
        return;
    }

    if (g_strcmp0(key, "hud-attach-to") == 0 ||
        g_strcmp0(key, "hud-location") == 0 ||
        g_str_has_prefix(key, "hud-applet-") ||
        g_strcmp0(key, "hud-history-max") == 0) {
        apply_hud_prefs(d);
    }
}

/* ── D-Bus name acquired ──────────────────────────────────────────────────── */

static void on_bus_acquired(GDBusConnection *conn, const char *name G_GNUC_UNUSED,
                             gpointer user_data)
{
    Daemon *d = user_data;
    g_autoptr(GError) err = NULL;
    GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(HUD_INTROSPECT, &err);
    if (!info) {
        g_warning("mate-hud: failed to build D-Bus introspection data: %s",
                  err->message);
        g_main_loop_quit(d->loop);
        return;
    }

    g_autoptr(GError) reg_err = NULL;
    guint reg_id = g_dbus_connection_register_object(conn, HUD_OBJECT,
        info->interfaces[0], &DBUS_VTABLE, d, NULL, &reg_err);
    g_dbus_node_info_unref(info);
    if (reg_id == 0) {
        g_warning("mate-hud: failed to register D-Bus object %s: %s",
                  HUD_OBJECT, reg_err ? reg_err->message : "unknown error");
        g_main_loop_quit(d->loop);
        return;
    }

    d->dbus_object_registered = TRUE;
    g_message("mate-hud: D-Bus service ready on %s %s", HUD_BUS, HUD_OBJECT);
}

static void on_name_lost(GDBusConnection *conn G_GNUC_UNUSED,
                          const char *name G_GNUC_UNUSED,
                          gpointer user_data)
{
    Daemon *d = user_data;
    shortcut_ungrab(d->grab);
    if (!d->dbus_object_registered) {
        g_warning("mate-hud: failed to acquire D-Bus name %s; exiting", HUD_BUS);
    } else {
        g_warning("mate-hud: lost D-Bus name %s; exiting", HUD_BUS);
    }
    g_main_loop_quit(d->loop);
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    g_set_application_name(_("Search for menus"));

    /* Open X display as early as possible */
    Display *xdpy = XOpenDisplay(NULL);
    if (!xdpy) {
        g_printerr("mate-hud-daemon: cannot open X display; exiting\n");
        return 1;
    }

    gtk_init(&argc, &argv);
    gtk_window_set_default_icon_name("system-search");

    /* Check if HUD is enabled in the applet's relocatable GSettings path */
    g_autofree char *settings_path = find_appmenu_settings_path();
    if (!settings_path) {
        g_printerr("mate-hud-daemon: could not locate AppMenu panel settings path; exiting\n");
        XCloseDisplay(xdpy);
        return 1;
    }

    GSettings *settings = g_settings_new_with_path(SCHEMA_ID, settings_path);
    if (!g_settings_get_boolean(settings, "hud-enabled")) {
        g_message("mate-hud: HUD disabled in settings at %s; exiting",
                  settings_path);
        g_object_unref(settings);
        XCloseDisplay(xdpy);
        return 0;
    }

    Daemon d = {0};
    d.xdpy     = xdpy;
    d.settings = settings;
    d.loop     = g_main_loop_new(NULL, FALSE);
    d.grab     = shortcut_new(xdpy);
    if (!d.grab) {
        g_warning("mate-hud: failed to initialize shortcut handling; exiting");
        g_main_loop_unref(d.loop);
        g_object_unref(settings);
        XCloseDisplay(xdpy);
        return 1;
    }

    g_message("mate-hud: starting with settings path %s", settings_path);

    /* Connect settings change handler */
    g_signal_connect(settings, "changed", G_CALLBACK(on_settings_changed), &d);

    /* Apply initial shortcut configuration */
    apply_shortcut_prefs(&d);

    /* Register D-Bus service */
    d.bus_owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
        HUD_BUS,
        G_BUS_NAME_OWNER_FLAGS_NONE,
        on_bus_acquired, NULL, on_name_lost,
        &d, NULL);
    if (d.bus_owner_id == 0) {
        g_warning("mate-hud: g_bus_own_name failed for %s; exiting", HUD_BUS);
        shortcut_free(d.grab);
        g_main_loop_unref(d.loop);
        g_free(d.shortcut_str);
        g_free(d.attach_to);
        g_free(d.location);
        g_object_unref(settings);
        XCloseDisplay(xdpy);
        return 1;
    }

    /* Run until quit */
    g_main_loop_run(d.loop);

    /* Cleanup */
    g_bus_unown_name(d.bus_owner_id);
    shortcut_free(d.grab);
    g_main_loop_unref(d.loop);
    g_free(d.shortcut_str);
    g_free(d.attach_to);
    g_free(d.location);
    g_object_unref(settings);
    XCloseDisplay(xdpy);
    return 0;
}
