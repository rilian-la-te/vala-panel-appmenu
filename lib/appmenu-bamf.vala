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
    DBusMenuHelper get_dbus_menu_helper_with_bamf(MenuWidget w, string name, ObjectPath path, Bamf.Application? app)
    {
        string? title = null;
        DesktopAppInfo? info = null;
        if (app != null)
        {
            var desktop_file = app.get_desktop_file();
            if (desktop_file != null)
            {
                info = new DesktopAppInfo.from_filename(desktop_file);
                title = info.get_name();
            }
        }
        if (title == null && app != null)
            title = app.get_name();
        return new DBusMenuHelper(w,name,path,title,info);
    }
    MenuModelHelper get_menu_model_helper_with_bamf(MenuWidget w, Bamf.Window window, Bamf.Application? app)
    {
        var gtk_unique_bus_name = window.get_utf8_prop("_GTK_UNIQUE_BUS_NAME");
        var app_menu_path = window.get_utf8_prop("_GTK_APP_MENU_OBJECT_PATH");
        var menubar_path = window.get_utf8_prop("_GTK_MENUBAR_OBJECT_PATH");
        var application_path = window.get_utf8_prop("_GTK_APPLICATION_OBJECT_PATH");
        var window_path = window.get_utf8_prop("_GTK_WINDOW_OBJECT_PATH");
        var unity_path = window.get_utf8_prop("_UNITY_OBJECT_PATH");
        DesktopAppInfo? info = null;
        string? title = null;
        if (app != null)
        {
            var desktop_file = app.get_desktop_file();
            if (desktop_file != null)
            {
                info = new DesktopAppInfo.from_filename(desktop_file);
                title = info.get_name();
            }
        }
        if (title == null && app != null)
            title = app.get_name();
        if (title == null)
            title = window.get_name();
        return new MenuModelHelper(w,gtk_unique_bus_name,app_menu_path,menubar_path,application_path,window_path,unity_path,title,info);
    }
    DBusAppMenu get_stub_helper_with_bamf(MenuWidget w, Bamf.Application app)
    {
        DesktopAppInfo? info = null;
        string? title = null;
        var desktop_file = app.get_desktop_file();
        if (desktop_file != null)
        {
            info = new DesktopAppInfo.from_filename(desktop_file);
            title = info.get_name();
        }
        if (title == null)
            title = app.get_name();
        return new DBusAppMenu(w,title,null,info);
    }
    internal class BackendBAMF : Backend
    {
        private HashTable<uint,unowned Bamf.Window> desktop_menus;
        private Bamf.Matcher matcher;
        private Helper helper;
        private Bamf.Application active_application;
        private Bamf.Window active_window;
        construct
        {
            desktop_menus = new HashTable<uint,unowned Bamf.Window>(direct_hash,direct_equal);
            matcher = Bamf.Matcher.get_default();
            proxy.window_registered.connect(register_menu_window);
            proxy.window_unregistered.connect(unregister_menu_window);
            proxy.registrar_changed.connect((h)=>{
                on_active_window_changed(this.active_window,this.active_window);
            });
            matcher.active_window_changed.connect(on_active_window_changed);
            matcher.view_opened.connect(on_window_opened);
            matcher.view_closed.connect(on_window_closed);
            foreach (unowned Bamf.Window window in matcher.get_windows())
                on_window_opened(window);
            foreach (unowned Bamf.Application app in matcher.get_running_applications())
                on_window_opened(app);
            on_active_window_changed(null,matcher.get_active_window());
        }
        public BackendBAMF()
        {
            Object();
        }
        ~BackendBAMF()
        {
            SignalHandler.disconnect_by_data(proxy,this);
            SignalHandler.disconnect_by_data(matcher,this);
        }
        public override void set_active_window_menu(MenuWidget widget)
        {
            helper = null;
            if(type == ModelType.MENUMODEL)
                helper = get_menu_model_helper_with_bamf(widget, active_window, active_application);
            else if(type == ModelType.DBUSMENU)
                create_dbusmenu_for_bamf_window(widget,active_window);
            else if(type == ModelType.DESKTOP)
                helper = new DesktopHelper(widget);
            else if(type == ModelType.STUB)
            {
                helper = get_stub_helper_with_bamf(widget,active_application);
                widget.set_menubar(null);
            }
        }
        private void register_menu_window(uint window_id, string sender, ObjectPath menu_object_path)
        {
            if (window_id != matcher.get_active_window().get_xid())
                return;
            this.active_window = matcher.get_active_window();
            this.type = ModelType.DBUSMENU;
            active_model_changed();
        }
        private void create_dbusmenu_for_bamf_window(MenuWidget menu,Bamf.Window window)
        {
            uint xid = window.get_xid();
            string name;
            ObjectPath path;
            proxy.get_menu_for_window(xid,out name, out path);
            active_application = matcher.get_application_for_xid(xid);
            helper = get_dbus_menu_helper_with_bamf(menu,name,path,active_application);
        }
        private void unregister_menu_window(uint window_id)
        {
            desktop_menus.remove(window_id);
        }
        private void on_window_opened(Bamf.View view)
        {
            if(view is Bamf.Window)
            {
                unowned Bamf.Window window = view as Bamf.Window;
                if (window.get_window_type() == Bamf.WindowType.DESKTOP)
                    desktop_menus.insert(window.get_xid(),window);
            }
        }
        private void on_window_closed(Bamf.View view)
        {
            if (view is Bamf.Window)
                unregister_menu_window((view as Bamf.Window).get_xid());
        }
        private void on_active_window_changed(Bamf.Window? prev, Bamf.Window? next)
        {
            unowned Bamf.Window win = next != null ? next : matcher.get_active_window();
            type = ModelType.NONE;
            lookup_menu(win);
            active_model_changed();
        }
        private void lookup_menu(Bamf.Window? window)
        {
            uint xid = 0;
            Bamf.Window? win = window;
            while (win != null && type == ModelType.NONE)
            {
                xid = window.get_xid();
                unowned Bamf.Application app = matcher.get_application_for_window(win);
                if (type == ModelType.NONE)
                {
                    string name;
                    ObjectPath path;
                    proxy.get_menu_for_window(xid,out name, out path);
                    /* Check DBusMenu sanity to differ it from MenuModel*/
                    if (!(name.length <= 0 && path == "/"))
                    {
                        this.active_application = app;
                        this.active_window = win;
                        this.type = ModelType.DBUSMENU;
                    }
                }
                /* First look to see if we can get these from the
                   GMenuModel access */
                if (type == ModelType.NONE)
                {
                    var uniquename = window.get_utf8_prop ("_GTK_UNIQUE_BUS_NAME");
                    if (uniquename != null)
                    {
                        this.active_window = win;
                        this.active_application = app;
                        if (window.get_window_type() == Bamf.WindowType.DESKTOP)
                            this.type = ModelType.DESKTOP;
                        else
                            this.type = ModelType.MENUMODEL;
                    }
                }
                if (type == ModelType.NONE)
                {
                    debug("Looking for parent window on XID %u", xid);
                    win = win.get_transient();
                    if (win == null && app != null)
                    {
                        this.active_window = win;
                        this.active_application = app;
                        type = ModelType.STUB;
                    }
                }
            }
            if (type == ModelType.NONE)
            {
                this.active_window = null;
                this.active_application = null;
                type = ModelType.DESKTOP;
            }
            return;
        }
    }
}
