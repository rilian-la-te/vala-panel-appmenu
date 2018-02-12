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

namespace Appmenu
{
    public class AppMenuBar : Gtk.Bin
    {
        private static DBusMenuRegistrarProxy proxy;
        private HashTable<uint,unowned Bamf.Window> desktop_menus;
        private Bamf.Matcher matcher;
        private Helper helper;
        private Helper bamf_helper;
        private unowned MenuWidget? menu
        {
            get {return this.get_child() as MenuWidget;}
        }
        static construct
        {
            proxy = new DBusMenuRegistrarProxy();
        }
        public AppMenuBar()
        {
            Object();
        }
        construct
        {
            desktop_menus = new HashTable<uint,unowned Bamf.Window>(direct_hash,direct_equal);
            matcher = Bamf.Matcher.get_default();
            var mw = new MenuWidget();
            this.add(mw);
            proxy.window_registered.connect(register_menu_window);
            proxy.window_unregistered.connect(unregister_menu_window);
            matcher.active_window_changed.connect(on_active_window_changed);
            matcher.view_opened.connect(on_window_opened);
            matcher.view_closed.connect(on_window_closed);
            foreach (unowned Bamf.Window window in matcher.get_windows())
                on_window_opened(window);
            foreach (unowned Bamf.Application app in matcher.get_running_applications())
                on_window_opened(app);
            on_active_window_changed(null,matcher.get_active_window());
        }
        protected override void destroy()
        {
            SignalHandler.disconnect_by_data(proxy,this);
            SignalHandler.disconnect_by_data(matcher,this);
            base.destroy();
        }
        protected override void map()
        {
            base.map();
            unowned Gtk.Settings gtksettings = this.get_settings();
            gtksettings.gtk_shell_shows_app_menu = false;
            gtksettings.gtk_shell_shows_menubar = false;
        }
        public void register_menu_window(uint window_id, string sender, ObjectPath menu_object_path)
        {
            if (window_id != matcher.get_active_window().get_xid())
                return;
            create_dbusmenu(window_id,sender,menu_object_path);
        }
        private void create_dbusmenu(uint window_id, string sender, ObjectPath menu_object_path)
        {
            unowned Bamf.Application app = matcher.get_application_for_xid(window_id);
            helper = null;
            helper = get_dbus_menu_helper_with_bamf(menu,sender,menu_object_path,app);
        }
        public void unregister_menu_window(uint window_id)
        {
            if (menu.window_id == window_id)
            {
                helper = null;
                helper = new DesktopHelper(menu);
            }
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
            lookup_menu(win);
        }
        private void lookup_menu(Bamf.Window? window)
        {
            uint xid = 0;
            bool found = false;
            Bamf.Window? win = window;
            menu.completed_menus = 0;
            while (win != null && found == false)
            {
                xid = window.get_xid();
                unowned Bamf.Application app = matcher.get_application_for_window(win);
                if (!found)
                {
                    string name;
                    ObjectPath path;
                    proxy.get_menu_for_window(xid,out name, out path);
                    /* Check DBusMenu sanity to differ it from MenuModel*/
                    if (!(name.length <= 0 && path == "/"))
                    {
                        create_dbusmenu(xid,name,path);
                        found = true;
                    }
                }
                /* First look to see if we can get these from the
                   GMenuModel access */
                if (!found)
                {
                    var uniquename = window.get_utf8_prop ("_GTK_UNIQUE_BUS_NAME");
                    if (uniquename != null)
                    {
                        helper = null;
                        if (window.get_window_type() == Bamf.WindowType.DESKTOP)
                            helper = new DesktopHelper(menu);
                        else
                            helper = get_menu_model_helper_with_bamf(menu,win,app);
                        found = true;
                    }
                }
                /* Appmenu hack, because BAMF does not always send a correct Application
                * DBusMenu registration always happened BEFORE a BAMF register application.
                */
//                if (found && (menu.completed_menus & MenuWidgetCompletionFlags.APPMENU) == 0 && app != null)
//                {
//                    bamf_helper = new BamfAppmenu(menu,app);
//                }
                if ((menu.completed_menus & MenuWidgetCompletionFlags.APPMENU) == 0 && (menu.completed_menus & MenuWidgetCompletionFlags.MENUBAR) == 0)
                {
                    debug("Looking for parent window on XID %u", xid);
                    win = win.get_transient();
                    if (win == null && app != null)
                    {
                        bamf_helper = get_stub_helper_with_bamf(menu,app);
                        menu.set_menubar(null);
                        found = true;
                    }
                }
            }
            if (found == false)
            {
                helper = null;
                helper = new DesktopHelper(menu);
            }
            return;
        }
    }
}
