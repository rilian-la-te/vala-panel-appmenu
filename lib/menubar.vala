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
        private unowned MenuWidget? menu
        {
            get {return this.get_child() as MenuWidget;}
            set {replace_menu(value);}
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
            if (window_id != matcher.get_active_window().get_xid() || window_id == menu.window_id)
                return;
            menu = create_dbusmenu(window_id,sender,menu_object_path);
        }
        private MenuWidget create_dbusmenu(uint window_id, string sender, ObjectPath menu_object_path)
        {
            unowned Bamf.Application app = matcher.get_application_for_xid(window_id);
            MenuWidget dbusmenu = new MenuWidgetDbusmenu(window_id,sender,menu_object_path,app);
            return dbusmenu;
        }
        public void unregister_menu_window(uint window_id)
        {
            if (menu.window_id == window_id)
            {
                this.menu.destroy();
                menu = show_dummy_menu();
            }
            desktop_menus.remove(window_id);
        }
        private void replace_menu(MenuWidget? menu)
        {
            if (this.menu != null)
                this.menu.destroy();
            if (menu != null)
                this.add(menu);
        }
        private MenuWidget? show_dummy_menu()
        {
            MenuWidget? menu = null;
            if (desktop_menus.length > 0)
            {
                desktop_menus.foreach((k,v)=>{
                    menu = new MenuWidgetDesktop(null,v);;
                    if (menu != null)
                        return;
                });
            }
            return menu;
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
            if (menu != null)
                menu.destroy();
            menu = lookup_menu(win);
            if (menu != null)
                menu.show();
        }
        private MenuWidget lookup_menu(Bamf.Window? window)
        {
            MenuWidget? menu = null;
            uint xid = 0;
            while (window != null && menu == null)
            {
                xid = window.get_xid();
                unowned Bamf.Application app = matcher.get_application_for_window(window);
                /* First look to see if we can get these from the
                   GMenuModel access */
                if (menu == null)
                {
                    var uniquename = window.get_utf8_prop ("_GTK_UNIQUE_BUS_NAME");
                    if (uniquename != null)
                    {
                        if (window.get_window_type() == Bamf.WindowType.DESKTOP)
                            menu = new MenuWidgetDesktop(app,window);
                        else
                            menu = new MenuWidgetMenumodel(app,window);
                        return menu;
                    }
                }
                if (menu == null)
                {
                    string name;
                    ObjectPath path;
                    proxy.get_menu_for_window(xid,out name, out path);
                    /* Check DBusMenu sanity to differ it from MenuModel*/
                    if (!(name.length <= 0 && path == "/"))
                        menu = create_dbusmenu(xid,name,path);
                }
                /* Appmenu hack, because BAMF does not always send a correct Application
                * DBusMenu registration always happened BEFORE a BAMF register application.
                */
                if (menu != null && (menu.completed_menus & MenuWidgetCompletionFlags.APPMENU) == 0 && app != null)
                {
                    var appmenu = new BamfAppmenu(app);
                    menu.add(appmenu);
                    menu.reorder_child(appmenu,0);
                    appmenu.show();
                    menu.completed_menus |= MenuWidgetCompletionFlags.APPMENU;
                }
                if (menu == null)
                {
                    debug("Looking for parent window on XID %u", xid);
                    window = window.get_transient();
                    if (window == null && app != null)
                        menu = new MenuWidgetAny(app);
                }
            }
            if (menu == null)
                menu = show_dummy_menu();
            return menu;
        }
    }
}
