/*
 * vala-panel-appmenu
 * Copyright (C) 2015 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using GLib;
using Gtk;

namespace Appmenu
{
    internal class BackendImpl : Backend
    {
        private HashTable<uint,unowned Wnck.Window> desktop_menus;
        private ValaPanel.Matcher matcher = ValaPanel.Matcher.get();
        private Helper helper;
        private Wnck.Window active_window;
        private int menu_update_delay = 500; // should be close enough to avoid flickering
        private uint delayed_menu_update_id = 0;
        private unowned Wnck.Screen screen;
        construct
        {
            desktop_menus = new HashTable<uint,unowned Wnck.Window>(direct_hash,direct_equal);
            screen = Wnck.Screen.get_default();
            proxy.window_registered.connect(register_menu_window);
            proxy.window_unregistered.connect(unregister_menu_window);
            proxy.registrar_changed.connect((h)=>{
                on_active_window_changed(this.active_window);
            });
            screen.active_window_changed.connect(on_active_window_changed);
            screen.window_opened.connect(on_window_opened);
            screen.window_closed.connect(on_window_closed);
            foreach (unowned Wnck.Window window in screen.get_windows())
                on_window_opened(window);
            on_active_window_changed(screen.get_active_window());
        }
        public BackendImpl()
        {
            Object();
        }
        ~BackendImpl()
        {
            SignalHandler.disconnect_by_data(proxy,this);
            SignalHandler.disconnect_by_data(screen,this);
        }
        public override void set_active_window_menu(MenuWidget widget)
        {
            helper = null;
            if(type == ModelType.MENUMODEL)
                helper = get_menu_model_helper_with_wnck(widget, active_window);
            else if(type == ModelType.DBUSMENU)
               create_dbusmenu_for_wnck_window(widget,active_window);
            else if(type == ModelType.DESKTOP)
                helper = new DesktopHelper(widget);
            else if(type == ModelType.STUB)
            {
                helper = get_stub_helper_with_wnck(widget,active_window);
                widget.set_menubar(null);
            }
        }
        DBusMenuHelper get_dbus_menu_helper_with_wnck(MenuWidget w, string name, ObjectPath path, Wnck.Window win)
        {
            string? title = null;
            DesktopAppInfo? info = libwnck_aux_match_wnck_window(matcher, win);
            if (info != null)
                title = info.get_name();
            if (title == null)
            {
                Wnck.Application app = win.get_application();
                if(app != null)
                    title = app.get_name();
                else
                    title = win.get_name();
            }
            return new DBusMenuHelper(w,name,path,title,info);
        }
        MenuModelHelper get_menu_model_helper_with_wnck(MenuWidget w, Wnck.Window win)
        {
            ulong xid = win.get_xid();
            var gtk_unique_bus_name = libwnck_aux_get_utf8_prop(xid,"_GTK_UNIQUE_BUS_NAME");
            var app_menu_path = libwnck_aux_get_utf8_prop(xid,"_GTK_APP_MENU_OBJECT_PATH");
            var menubar_path = libwnck_aux_get_utf8_prop(xid,"_GTK_MENUBAR_OBJECT_PATH");
            var application_path = libwnck_aux_get_utf8_prop(xid,"_GTK_APPLICATION_OBJECT_PATH");
            var window_path = libwnck_aux_get_utf8_prop(xid,"_GTK_WINDOW_OBJECT_PATH");
            var unity_path = libwnck_aux_get_utf8_prop(xid,"_UNITY_OBJECT_PATH");
            DesktopAppInfo? info = libwnck_aux_match_wnck_window(matcher, win);
            string? title = null;
            if (info != null)
                title = info.get_name();
            if (title == null)
            {
                Wnck.Application app = win.get_application();
                if(app != null)
                    title = app.get_name();
                else
                    title = win.get_name();
            }
            return new MenuModelHelper(w,gtk_unique_bus_name,app_menu_path,menubar_path,application_path,window_path,unity_path,title,info);
        }
        DBusAppMenu get_stub_helper_with_wnck(MenuWidget w, Wnck.Window win)
        {
            string? title = null;
            DesktopAppInfo? info = libwnck_aux_match_wnck_window(ValaPanel.Matcher.get(), win);
            if (info != null)
                title = info.get_name();
            if (title == null)
            {
                Wnck.Application app = win.get_application();
                if(app != null)
                    title = app.get_name();
                else
                    title = win.get_name();
            }
            return new DBusAppMenu(w,title,null,info);
        }
        private void register_menu_window(uint window_id, string sender, ObjectPath menu_object_path)
        {
            if (window_id != screen.get_active_window().get_xid())
                return;
            this.active_window = screen.get_active_window();
            this.type = ModelType.DBUSMENU;
            active_model_changed();
        }
        private void unregister_menu_window(uint window_id)
        {
            desktop_menus.remove(window_id);
        }
        private void create_dbusmenu_for_wnck_window(MenuWidget menu,Wnck.Window window)
        {
            ulong xid = window.get_xid();
            string name;
            ObjectPath path;
            proxy.get_menu_for_window((uint)xid,out name, out path);
            helper = get_dbus_menu_helper_with_wnck(menu,name,path, window);
        }
        private void on_window_opened(Wnck.Window window)
        {
            if (window.get_window_type() == Wnck.WindowType.DESKTOP)
                desktop_menus.insert((uint)window.get_xid(),window);
        }
        private void reset_menu_update_timeout() {
            if (delayed_menu_update_id > 0) {
                Source.remove(delayed_menu_update_id);
            }
            delayed_menu_update_id = 0;
        }
        private void on_window_closed(Wnck.Window window)
        {
            unregister_menu_window((uint)window.get_xid());
            delayed_menu_update_id = Timeout.add(menu_update_delay, menu_update_timeout);
        }
        private bool menu_update_timeout() {
            unowned Wnck.Window win = screen.get_active_window();
            type = ModelType.NONE;
            lookup_menu(win);
            active_model_changed();
            delayed_menu_update_id = 0;
            return false;
        }
        private void on_active_window_changed(Wnck.Window? prev)
        {
            reset_menu_update_timeout();
            unowned Wnck.Window win = screen.get_active_window();
            type = ModelType.NONE;
            lookup_menu(win);
            active_model_changed();
        }
        private void lookup_menu(Wnck.Window? window)
        {
            ulong xid = 0;
            Wnck.Window? win = window;
            while (win != null && type == ModelType.NONE)
            {
                xid = window.get_xid();
                unowned Wnck.Application app = window.get_application();
                if (type == ModelType.NONE)
                {
                    string name;
                    ObjectPath path;
                    proxy.get_menu_for_window((uint)xid,out name, out path);
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
                    var uniquename = libwnck_aux_get_utf8_prop (win.get_xid(), "_GTK_UNIQUE_BUS_NAME");
                    if (uniquename != null)
                    {
                        this.active_window = win;
                        this.type = ModelType.MENUMODEL;
                    }
                }
                if (type == ModelType.NONE && window.get_window_type() == Wnck.WindowType.DESKTOP)
                {
                    this.active_window = win;
                    this.type = ModelType.DESKTOP;
                }
                if (type == ModelType.NONE)
                {
                    debug("Looking for parent window on XID %lu", xid);
                    win = win.get_transient();
                    if (win == null && app != null)
                    {
                        this.active_window = window;
                        type = ModelType.STUB;
                    }
                }
            }
            if (type == ModelType.NONE)
            {
                this.active_window = null;
                type = ModelType.DESKTOP;
            }
            return;
        }
    }
}
