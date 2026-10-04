#include <gio/gio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#define HUD_BUS    "org.mate.HUD"
#define HUD_OBJECT "/org/mate/HUD"
#define TAP_TIMEOUT_MS 250

static const guint IGNORED_MASKS[] = {
    0,
    Mod2Mask,
    LockMask,
    Mod2Mask | LockMask,
    Mod5Mask,
    Mod2Mask | Mod5Mask,
    LockMask | Mod5Mask,
    Mod2Mask | LockMask | Mod5Mask,
};
#define N_IGNORED (sizeof(IGNORED_MASKS) / sizeof(IGNORED_MASKS[0]))
#define KNOWN_MODS (ControlMask | ShiftMask | Mod1Mask | Mod2Mask | \
                    Mod3Mask | Mod4Mask | Mod5Mask)

typedef struct {
    Display *dpy;
    Window   root;
    KeyCode  keycode;
    guint    modifiers;
    gboolean bare_tap;
    gboolean possible_tap;
    guint32  tap_start_time;
} GrabState;

static gboolean helper_running = TRUE;

static void handle_term(int signo G_GNUC_UNUSED)
{
    helper_running = FALSE;
}

static const char *current_wm_name(Display *dpy, Window root)
{
    static char name_buf[128];
    Atom wm_check = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", True);
    Atom wm_name  = XInternAtom(dpy, "_NET_WM_NAME", True);
    Atom utf8     = XInternAtom(dpy, "UTF8_STRING", True);
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char *data = NULL;
    Window *wm_window = NULL;

    name_buf[0] = '\0';
    if (wm_check == None || wm_name == None)
        return name_buf;

    if (XGetWindowProperty(dpy, root, wm_check, 0, 1, False, XA_WINDOW,
                           &type, &format, &nitems, &after, &data) != Success || !data)
        return name_buf;

    wm_window = (Window *)data;
    Window actual = wm_window[0];
    XFree(data);
    data = NULL;

    if (XGetWindowProperty(dpy, actual, wm_name, 0, 64, False,
                           utf8 != None ? utf8 : AnyPropertyType,
                           &type, &format, &nitems, &after, &data) != Success || !data)
        return name_buf;

    g_strlcpy(name_buf, (const char *)data, sizeof(name_buf));
    XFree(data);
    return name_buf;
}

static gboolean keyval_is_modifier(guint keyval)
{
    switch (keyval) {
    case GDK_KEY_Alt_L:
    case GDK_KEY_Alt_R:
    case GDK_KEY_Super_L:
    case GDK_KEY_Super_R:
    case GDK_KEY_Control_L:
    case GDK_KEY_Control_R:
    case GDK_KEY_Shift_L:
    case GDK_KEY_Shift_R:
    case GDK_KEY_Meta_L:
    case GDK_KEY_Meta_R:
    case GDK_KEY_Hyper_L:
    case GDK_KEY_Hyper_R:
    case GDK_KEY_ISO_Level3_Shift:
        return TRUE;
    default:
        return FALSE;
    }
}

static gboolean keycode_in_replay_whitelist(Display *dpy, KeyCode kc)
{
    static const KeySym navkeys[] = {
        XK_Return, XK_Up, XK_Down, XK_Left, XK_Right, XK_Home, XK_BackSpace, 0
    };

    for (int i = 0; navkeys[i]; i++) {
        if (XKeysymToKeycode(dpy, navkeys[i]) == kc)
            return TRUE;
    }

    KeySym sym = XKeycodeToKeysym(dpy, kc, 0);
    return (sym >= 8 && sym <= 255);
}

static gboolean send_activate(guint32 timestamp)
{
    g_autoptr(GError) err = NULL;
    g_autoptr(GDBusConnection) conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &err);
    if (!conn) {
        g_warning("mate-hud-shortcut-helper: no session bus: %s", err->message);
        return FALSE;
    }

    g_autoptr(GVariant) reply = g_dbus_connection_call_sync(
        conn,
        HUD_BUS,
        HUD_OBJECT,
        HUD_BUS,
        "Activate",
        g_variant_new("(u)", timestamp),
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        2000,
        NULL,
        &err);
    if (!reply) {
        g_warning("mate-hud-shortcut-helper: Activate call failed: %s", err->message);
        return FALSE;
    }
    return TRUE;
}

static void ungrab_all(GrabState *gs)
{
    for (guint i = 0; i < N_IGNORED; i++)
        XUngrabKey(gs->dpy, gs->keycode, (gs->modifiers & KNOWN_MODS) | IGNORED_MASKS[i], gs->root);

    if (gs->bare_tap) {
        for (guint i = 0; i < N_IGNORED; i++)
            XUngrabButton(gs->dpy, AnyButton, Mod1Mask | IGNORED_MASKS[i], gs->root);
    }
    XFlush(gs->dpy);
}

static gboolean grab_all(GrabState *gs)
{
    int keyboard_mode = gs->bare_tap ? GrabModeSync : GrabModeAsync;
    guint mods = gs->modifiers & KNOWN_MODS;

    for (guint i = 0; i < N_IGNORED; i++) {
        XGrabKey(gs->dpy, gs->keycode, mods | IGNORED_MASKS[i],
                 gs->root, False, GrabModeAsync, keyboard_mode);
    }

    if (gs->bare_tap) {
        for (guint i = 0; i < N_IGNORED; i++) {
            XGrabButton(gs->dpy, AnyButton, Mod1Mask | IGNORED_MASKS[i],
                        gs->root, True, ButtonPressMask,
                        GrabModeSync, GrabModeAsync, None, None);
        }
    }

    XSync(gs->dpy, False);
    return TRUE;
}

static gboolean parse_shortcut(Display *dpy, const char *accelerator, GrabState *gs)
{
    guint keyval = 0;
    GdkModifierType mods = 0;
    GdkKeymapKey *keys = NULL;
    gint n_keys = 0;

    gtk_accelerator_parse(accelerator, &keyval, &mods);
    if (!keyval) {
        keyval = gdk_keyval_from_name(accelerator);
        mods = 0;
    }
    if (!keyval || !keyval_is_modifier(keyval))
        return FALSE;

    if (!gdk_keymap_get_entries_for_keyval(
            gdk_keymap_get_for_display(gdk_display_get_default()),
            keyval, &keys, &n_keys) || n_keys == 0) {
        g_free(keys);
        return FALSE;
    }

    gs->dpy = dpy;
    gs->root = DefaultRootWindow(dpy);
    gs->keycode = (KeyCode)keys[0].keycode;
    gs->modifiers = (guint)mods;
    gs->bare_tap = (mods == 0);
    gs->possible_tap = FALSE;
    gs->tap_start_time = 0;
    g_free(keys);
    return TRUE;
}

static void release_keyboard_if_stuck(GrabState *gs)
{
    if (!gs->possible_tap)
        return;

    gs->possible_tap = FALSE;
    XAllowEvents(gs->dpy, AsyncKeyboard, CurrentTime);
    XFlush(gs->dpy);
    g_warning("mate-hud-shortcut-helper: released stuck sync keyboard grab");
}

static void forward_button_event(GrabState *gs, XEvent *ev)
{
    XAllowEvents(gs->dpy, ReplayPointer, CurrentTime);

    const char *wm = current_wm_name(gs->dpy, gs->root);
    if (wm[0] != '\0' && g_ascii_strcasecmp(wm, "compiz") == 0) {
        XFlush(gs->dpy);
        return;
    }

    XUngrabKeyboard(gs->dpy, CurrentTime);
    XUngrabPointer(gs->dpy, CurrentTime);

    Window root_ret, child_ret;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    if (XQueryPointer(gs->dpy, gs->root, &root_ret, &child_ret,
                      &root_x, &root_y, &win_x, &win_y, &mask) && child_ret != None) {
        XSendEvent(gs->dpy, child_ret, True, ButtonPressMask, ev);
    }
    XFlush(gs->dpy);
}

static void forward_key_event(GrabState *gs, XEvent *ev)
{
    if (keycode_in_replay_whitelist(gs->dpy, ev->xkey.keycode)) {
        XAllowEvents(gs->dpy, ReplayKeyboard, CurrentTime);
    } else {
        XAllowEvents(gs->dpy, AsyncKeyboard, CurrentTime);
    }

    XUngrabKeyboard(gs->dpy, CurrentTime);
    XSendEvent(gs->dpy, ev->xkey.window,
               True, KeyPressMask | KeyReleaseMask, ev);
    XFlush(gs->dpy);
}

static void run_loop(GrabState *gs)
{
    int xfd = ConnectionNumber(gs->dpy);

    while (helper_running) {
        int timeout_ms = -1;
        struct pollfd pfd = { .fd = xfd, .events = POLLIN, .revents = 0 };

        if (gs->bare_tap && gs->possible_tap) {
            gint64 now_ms = g_get_monotonic_time() / 1000;
            gint64 deadline_ms = (gint64)gs->tap_start_time + TAP_TIMEOUT_MS;
            timeout_ms = (deadline_ms > now_ms) ? (int)(deadline_ms - now_ms) : 0;
        }

        if (XPending(gs->dpy) == 0) {
            int rv = poll(&pfd, 1, timeout_ms);
            if (rv == 0) {
                release_keyboard_if_stuck(gs);
                continue;
            }
            if (rv < 0)
                continue;
        }

        while (XPending(gs->dpy) > 0) {
            XEvent ev;
            XNextEvent(gs->dpy, &ev);

            if (!gs->bare_tap) {
                if (ev.type == KeyPress) {
                    guint state = ev.xkey.state & KNOWN_MODS & ~(Mod2Mask | LockMask | Mod5Mask);
                    guint want  = gs->modifiers & KNOWN_MODS & ~(Mod2Mask | LockMask | Mod5Mask);
                    if (ev.xkey.keycode == gs->keycode && state == want)
                        send_activate(ev.xkey.time);
                }
                continue;
            }

            guint32 event_time = 0;
            if (ev.type == KeyPress || ev.type == KeyRelease)
                event_time = ev.xkey.time;
            else if (ev.type == ButtonPress)
                event_time = ev.xbutton.time;

            if (gs->possible_tap && event_time &&
                (event_time - gs->tap_start_time) > TAP_TIMEOUT_MS) {
                release_keyboard_if_stuck(gs);
            }

            if (ev.type == KeyPress && ev.xkey.keycode == gs->keycode && !gs->possible_tap) {
                guint state = ev.xkey.state & KNOWN_MODS & ~(Mod2Mask | LockMask | Mod5Mask);
                guint want  = gs->modifiers & KNOWN_MODS & ~(Mod2Mask | LockMask | Mod5Mask);
                if (state == want) {
                    gs->tap_start_time = ev.xkey.time;
                    gs->possible_tap = TRUE;
                }
                XAllowEvents(gs->dpy, SyncKeyboard, CurrentTime);
            } else if (ev.type == KeyRelease &&
                       ev.xkey.keycode == gs->keycode && gs->possible_tap) {
                gs->possible_tap = FALSE;
                send_activate(ev.xkey.time);
                XAllowEvents(gs->dpy, AsyncKeyboard, CurrentTime);
            } else if (ev.type == ButtonPress) {
                gs->possible_tap = FALSE;
                forward_button_event(gs, &ev);
            } else if (ev.type == KeyPress || ev.type == KeyRelease) {
                gs->possible_tap = FALSE;
                forward_key_event(gs, &ev);
            } else {
                gs->possible_tap = FALSE;
                XAllowEvents(gs->dpy, AsyncKeyboard, CurrentTime);
            }

            XFlush(gs->dpy);
        }
    }
}

int main(int argc, char **argv)
{
    Display *dpy;
    GrabState gs = {0};

    if (argc < 2) {
        g_printerr("mate-hud-shortcut-helper: missing shortcut argument\n");
        return 1;
    }

    gtk_init(&argc, &argv);

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        g_printerr("mate-hud-shortcut-helper: cannot open X display\n");
        return 1;
    }

    if (!parse_shortcut(dpy, argv[1], &gs)) {
        g_printerr("mate-hud-shortcut-helper: invalid internal shortcut '%s'\n", argv[1]);
        XCloseDisplay(dpy);
        return 1;
    }

    signal(SIGTERM, handle_term);
    signal(SIGINT, handle_term);

    g_message("mate-hud-shortcut-helper: grabbed '%s' in %s mode",
              argv[1], gs.bare_tap ? "modifier-tap" : "modifier-chord");
    grab_all(&gs);
    run_loop(&gs);
    ungrab_all(&gs);
    XCloseDisplay(dpy);
    return 0;
}
