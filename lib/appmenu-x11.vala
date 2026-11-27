/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* X11 backend built on the xcb window manager helper (see xcb-aux.c) instead of
 * libwnck. It keeps the same public surface as the wnck based backend did: an
 * Appmenu subclass that resolves a menu model for the focused window.
 *
 * Where wnck handed out a Wnck.Window object, this backend works with plain XID
 * values, which is all the helper exposes and all the lookups below need. */

using GLib;

namespace Appmenu
{
    internal class BackendX11 : Backend
    {
        /* XIDs of the windows the helper reported as desktop windows. wnck kept
         * the window objects here; an XID is enough, since a window that closes
         * stops being a desktop window anyway. */
        private HashTable<uint,bool> desktop_windows;
        private ValaPanel.Matcher matcher = ValaPanel.Matcher.get();
        /* The xcb helper connection. It is refcounted by the helper itself, and
         * this instance holds one of those references until it finalizes. */
        private unowned Xcb.Aux? wm;
        private uint active_window = 0;
        private int menu_update_delay = 500; // should be close enough to avoid flickering
        private uint delayed_menu_update_id = 0;

        /* The window manager activity the helper reports. Declared here so that
         * this instance, registered as a listener, is the object they live on and
         * the C helper can emit them by name (see ensure_signals() in xcb-aux.c). */
        public signal void wm_window_opened(uint xid);
        public signal void wm_window_closed(uint xid);
        public signal void wm_active_window_changed();

        public static BackendX11 get_backend()
        {
            return new BackendX11();
        }
        public static bool is_applicable()
        {
            return Xcb.is_applicable();
        }
        construct
        {
            desktop_windows = new HashTable<uint,bool>(direct_hash,direct_equal);
            wm = Xcb.Aux.connect();
            if (wm == null)
                return;
            this.wm_window_opened.connect(on_window_opened);
            this.wm_window_closed.connect(on_window_closed);
            this.wm_active_window_changed.connect(on_active_window_changed);
            proxy.window_registered.connect((window_id,menu_name,menu_object_path) => {
                register_menu_window(window_id);
            });
            proxy.window_unregistered.connect(unregister_menu_window);
            proxy.registrar_changed.connect((h) => {
                on_active_window_changed();
            });
            /* Registering as a listener refreshes the window list and reports the
             * windows that already exist, then watch_root() lets the main loop see
             * any later changes. */
            wm.add_listener(this);
            wm.watch_root();
        }
        ~BackendX11()
        {
            reset_menu_update_timeout();
            if (wm != null)
            {
                Xcb.Aux.remove_listener(this);
                wm.unref();
                wm = null; // drops this instance's reference to the shared connection
            }
        }

        /* wnck reported no window at all rather than a NULL one once focus left, so
         * both mean "nothing focused" here. */
        private uint current_window()
        {
            return wm != null ? wm.get_active_window() : 0;
        }

        public override void update_helper()
        {
            var ctx = new GLib.AppLaunchContext();
            helper = null;
            if(active_window == 0)
                return;
            if(type == ModelType.MENUMODEL)
                helper = get_menu_model_helper(active_window,ctx);
            else if(type == ModelType.DBUSMENU)
                create_dbusmenu_for_window(active_window,ctx);
            else if(type == ModelType.DESKTOP)
                helper = new DesktopHelper(ctx);
            else if(type == ModelType.STUB)
                helper = get_stub_helper(active_window,ctx);
            if(helper != null)
                helper.content_updated.connect(()=>{
                    active_model_changed();
                });
        }
        DBusMenuHelper get_dbus_menu_helper(uint xid,string name,ObjectPath path,GLib.AppLaunchContext ctx)
        {
            DesktopAppInfo? info = wm.match_app(matcher,xid);
            return new DBusMenuHelper(ctx,name,path,window_title(xid),info);
        }
        MenuModelHelper get_menu_model_helper(uint xid,GLib.AppLaunchContext ctx)
        {
            var gtk_unique_bus_name = wm.get_utf8_prop(xid,"_GTK_UNIQUE_BUS_NAME");
            var app_menu_path = wm.get_utf8_prop(xid,"_GTK_APP_MENU_OBJECT_PATH");
            var menubar_path = wm.get_utf8_prop(xid,"_GTK_MENUBAR_OBJECT_PATH");
            var application_path = wm.get_utf8_prop(xid,"_GTK_APPLICATION_OBJECT_PATH");
            var window_path = wm.get_utf8_prop(xid,"_GTK_WINDOW_OBJECT_PATH");
            var unity_path = wm.get_utf8_prop(xid,"_UNITY_OBJECT_PATH");
            DesktopAppInfo? info = wm.match_app(matcher,xid);
            return new MenuModelHelper(ctx,gtk_unique_bus_name,app_menu_path,menubar_path,application_path,window_path,unity_path,window_title(xid),info);
        }
        DBusAppMenu get_stub_helper(uint xid,GLib.AppLaunchContext ctx)
        {
            string title = window_title(xid);
            DesktopAppInfo? info = wm.match_app(matcher,xid);
            if (title_filter != null)
            {
                string? inst = null;
                string? grp = null;
                wm.get_wm_class(xid,out inst,out grp);
                string? filtered = title_filter(wm.get_pid(xid),inst ?? "",title);
                if (filtered != null)
                    title = filtered;
            }
            return new DBusAppMenu(ctx,title,(Posix.pid_t)wm.get_pid(xid),info);
        }
        /* wnck fell back from a matched .desktop name to the application name and
         * then to the window title; without Wnck.Application the window title is
         * the closest equivalent. */
        private string window_title(uint xid)
        {
            string? name = wm.get_name(xid);
            return name != null ? name : "";
        }
        private void register_menu_window(uint window_id)
        {
            uint win = current_window();
            if (window_id != win)
                return;
            this.active_window = win;
            this.type = ModelType.DBUSMENU;
            active_model_changed();
        }
        private void unregister_menu_window(uint window_id)
        {
            desktop_windows.remove(window_id);
        }
        private void create_dbusmenu_for_window(uint xid,GLib.AppLaunchContext ctx)
        {
            string name;
            ObjectPath path;
            proxy.get_menu_for_window(xid,out name, out path);
            helper = get_dbus_menu_helper(xid,name,path,ctx);
        }
        private void on_window_opened(uint xid)
        {
            if (wm.is_desktop_window(xid))
                desktop_windows.insert(xid,true);
        }
        private void reset_menu_update_timeout()
        {
            if (delayed_menu_update_id > 0)
                Source.remove(delayed_menu_update_id);
            delayed_menu_update_id = 0;
        }
        private void on_window_closed(uint xid)
        {
            unregister_menu_window(xid);
            delayed_menu_update_id = Timeout.add(menu_update_delay,menu_update_timeout);
        }
        private bool menu_update_timeout()
        {
            uint win = current_window();
            type = ModelType.NONE;
            lookup_menu(win);
            active_model_changed();
            delayed_menu_update_id = 0;
            return false;
        }
        /* The helper reports the activity it notices rather than a previous window
         * like wnck did, so the current focus is read here. */
        private void on_active_window_changed()
        {
            reset_menu_update_timeout();
            uint win = current_window();
            type = ModelType.NONE;
            lookup_menu(win);
            active_model_changed();
        }
        private void lookup_menu(uint xid)
        {
            uint win = xid;
            while (win != 0 && type == ModelType.NONE)
            {
                if (type == ModelType.NONE)
                {
                    string name;
                    ObjectPath path;
                    proxy.get_menu_for_window(win,out name, out path);
                    /* Check DBusMenu sanity to differ it from MenuModel*/
                    if (!(name.length <= 0 && path == "/"))
                    {
                        this.active_window = win;
                        this.type = ModelType.DBUSMENU;
                    }
                }
                /* First look to see if we can get these from the
                   GMenuModel access */
                if (type == ModelType.NONE)
                {
                    var uniquename = wm.get_utf8_prop(win,"_GTK_UNIQUE_BUS_NAME");
                    if (uniquename != null)
                    {
                        this.active_window = win;
                        if (wm.is_desktop_window(win)
                            && wm.get_utf8_prop(win,"_GTK_MENUBAR_OBJECT_PATH") == null
                            && wm.get_utf8_prop(win,"_GTK_APP_MENU_OBJECT_PATH") == null)
                            this.type = ModelType.DESKTOP;
                        else
                            this.type = ModelType.MENUMODEL;
                    }
                }
                if (type == ModelType.NONE && wm.is_desktop_window(win))
                {
                    this.active_window = win;
                    this.type = ModelType.DESKTOP;
                }
                if (type == ModelType.NONE)
                {
                    debug("Looking to see if we can get menu from window owner on XID %u",win);
                    /* wnck stopped at the first window whose application had no
                     * further window to offer. The helper exposes no applications,
                     * so the equivalent stopping point is a window that belongs to
                     * the window manager itself (a dock or the desktop): anything
                     * above it is not an application window worth walking to. */
                    uint next = wm.get_transient_for(win);
                    if (next == 0)
                        next = wm.get_parent(win);
                    if (next == 0 || !wm.is_desktop_window(next))
                    {
                        this.active_window = xid;
                        type = ModelType.STUB;
                    }
                    win = next;
                }
            }
            if (type == ModelType.NONE)
            {
                this.active_window = 0;
                type = ModelType.DESKTOP;
            }
            return;
        }
    }
}
