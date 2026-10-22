#include "collect.h"
#include "collect-dbusmenu.h"
#include "collect-menumodel.h"
#include <gio/gio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>

#define REGISTRAR_BUS    "com.canonical.AppMenu.Registrar"
#define REGISTRAR_OBJECT "/com/canonical/AppMenu/Registrar"
#define REGISTRAR_IFACE  "com.canonical.AppMenu.Registrar"

/* Check the registrar for a DBusMenu entry for the window.
 * Returns TRUE and sets both out params if found (non-trivial).
 * Caller frees *service and *path. */
static gboolean query_registrar(gulong xid, char **service, char **path)
{
    g_autoptr(GError) err = NULL;
    GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &err);
    if (!conn) {
        g_message("mate-hud: registrar lookup unavailable for xid=%lu: %s",
                  xid, err ? err->message : "no session bus");
        return FALSE;
    }

    GDBusProxy *reg = g_dbus_proxy_new_sync(conn,
        G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
        G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
        NULL, REGISTRAR_BUS, REGISTRAR_OBJECT, REGISTRAR_IFACE,
        NULL, &err);
    g_object_unref(conn);
    if (!reg) {
        g_message("mate-hud: registrar proxy unavailable for xid=%lu: %s",
                  xid, err ? err->message : "proxy creation failed");
        return FALSE;
    }

    GVariant *reply = g_dbus_proxy_call_sync(reg, "GetMenuForWindow",
        g_variant_new("(u)", (guint32)xid),
        G_DBUS_CALL_FLAGS_NONE, 1000, NULL, &err);
    g_object_unref(reg);
    if (!reply) {
        g_message("mate-hud: registrar lookup failed for xid=%lu: %s",
                  xid, err ? err->message : "GetMenuForWindow failed");
        return FALSE;
    }

    if (!g_variant_is_of_type(reply, G_VARIANT_TYPE("(so)"))) {
        g_message("mate-hud: registrar returned unexpected type %s for xid=%lu",
                  g_variant_get_type_string(reply), xid);
        g_variant_unref(reply);
        return FALSE;
    }

    const char *svc, *pth;
    g_variant_get(reply, "(&s&o)", &svc, &pth);

    /* Non-trivial means service is non-empty and path is not just "/" */
    gboolean found = (svc && *svc && pth && g_strcmp0(pth, "/") != 0);
    if (found) {
        *service = g_strdup(svc);
        *path    = g_strdup(pth);
        g_message("mate-hud: xid=%lu using DBusMenu backend %s %s",
                  xid, svc, pth);
    }
    g_variant_unref(reply);
    return found;
}

/* Check if the window has GTK GMenuModel properties set. */
static gboolean has_menumodel(gulong xid)
{
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return FALSE;

    Atom prop = XInternAtom(dpy, "_GTK_UNIQUE_BUS_NAME", True);
    gboolean found = FALSE;
    if (prop != None) {
        Atom type; int fmt;
        unsigned long n, after;
        unsigned char *data = NULL;
        if (XGetWindowProperty(dpy, (Window)xid, prop,
                               0, 1, False, AnyPropertyType,
                               &type, &fmt, &n, &after, &data) == Success
            && data && n > 0) {
            found = TRUE;
            XFree(data);
        }
    }
    XCloseDisplay(dpy);
    return found;
}

GPtrArray *collect_menu_for_window(gulong xid)
{
    if (!xid) return NULL;

    /* 1. Try DBusMenu via registrar */
    char *service = NULL, *path = NULL;
    if (query_registrar(xid, &service, &path)) {
        GPtrArray *entries = collect_dbusmenu(service, path);
        g_free(service);
        service = NULL;
        g_free(path);
        path = NULL;
        if (entries && entries->len > 0) {
            g_message("mate-hud: xid=%lu collected %u DBusMenu entries",
                      xid, entries->len);
            return entries;
        }
        g_message("mate-hud: xid=%lu DBusMenu backend returned no actionable entries",
                  xid);
        if (entries)
            g_ptr_array_unref(entries);
    }
    g_free(service);
    g_free(path);

    /* 2. Try GMenuModel via X11 window properties */
    if (has_menumodel(xid)) {
        g_message("mate-hud: xid=%lu using GMenuModel backend", xid);
        GPtrArray *entries = collect_menumodel(xid);
        if (entries && entries->len > 0) {
            g_message("mate-hud: xid=%lu collected %u GMenuModel entries",
                      xid, entries->len);
            return entries;
        }
        g_message("mate-hud: xid=%lu GMenuModel backend returned no actionable entries",
                  xid);
        if (entries)
            g_ptr_array_unref(entries);
    }

    g_message("mate-hud: xid=%lu has no usable exported menu", xid);
    return NULL;
}
