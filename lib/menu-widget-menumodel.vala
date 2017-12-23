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
    internal class MenuWidgetMenumodel : MenuWidget
    {
        private GLib.ActionGroup? appmenu_actions = null;
        private GLib.ActionGroup? menubar_actions = null;
        private GLib.ActionGroup? unity_actions = null;
        private Gtk.Adjustment? scroll_adj = null;
        protected Gtk.MenuBar? appmenu = null;
        protected Gtk.MenuBar? menubar = null;
        public MenuWidgetMenumodel(Bamf.Application? app,Bamf.Window window)
        {
            this.window_id = window.get_xid();
            var gtk_unique_bus_name = window.get_utf8_prop("_GTK_UNIQUE_BUS_NAME");
            var app_menu_path = window.get_utf8_prop("_GTK_APP_MENU_OBJECT_PATH");
            var menubar_path = window.get_utf8_prop("_GTK_MENUBAR_OBJECT_PATH");
            var application_path = window.get_utf8_prop("_GTK_APPLICATION_OBJECT_PATH");
            var window_path = window.get_utf8_prop("_GTK_WINDOW_OBJECT_PATH");
            var unity_path = window.get_utf8_prop("_UNITY_OBJECT_PATH");
            DBusConnection? dbusconn = null;
            try {
                dbusconn = Bus.get_sync(BusType.SESSION);
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
                return;
            }
            if (application_path != null)
                appmenu_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,application_path);
            if (unity_path != null)
                unity_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,unity_path);
            if (window_path != null)
                menubar_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,window_path);
            string? name = null;
            if (app != null)
            {
                var desktop_file = app.get_desktop_file();
                if (desktop_file != null)
                    name = new DesktopAppInfo.from_filename(desktop_file).get_name();
            }
            if (name == null && app != null)
                name = app.get_name();
            if (name == null)
                name = window.get_name();
            if (name == null)
                name = _("_Application");

            // Scroller setup
            scroll_adj = new Gtk.Adjustment(0, 0, 0, 20, 20, 0);
            var scroller = new Gtk.ScrolledWindow(scroll_adj, null);
            scroller.set_hexpand(true);
            scroller.set_policy(Gtk.PolicyType.EXTERNAL, Gtk.PolicyType.NEVER);
            scroller.set_shadow_type(Gtk.ShadowType.NONE);
            scroller.scroll_event.connect(on_scroll_event);

            if (app_menu_path != null)
            {
                var menu = new GLib.Menu();
                menu.append_submenu(name,DBusMenuModel.get(dbusconn,gtk_unique_bus_name,app_menu_path));
                appmenu = new Gtk.MenuBar.from_model(menu);
            }
            else if (app != null)
                appmenu = new BamfAppmenu(app);
            if (appmenu != null)
                completed_menus |= MenuWidgetCompletionFlags.APPMENU;
            else
                appmenu = new Gtk.MenuBar();
            this.add(appmenu);
            if (menubar_path != null)
            {
                menubar = new Gtk.MenuBar.from_model(DBusMenuModel.get(dbusconn,gtk_unique_bus_name,menubar_path));
                this.add(scroller);
                scroller.add(menubar);
                if (menubar.get_children().length() > 0)
                    completed_menus |= MenuWidgetCompletionFlags.MENUBAR;
            } else
                menubar = new Gtk.MenuBar();

            var style_provider = new Gtk.CssProvider();
            style_provider.load_from_resource("/org/vala-panel/appmenu/appmenu.css");
            var style_context = menubar.get_style_context();
            style_context.add_provider(style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
            style_context.add_class("-vala-panel-appmenu-private");
            style_context = appmenu.get_style_context();
            style_context.add_provider(style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
#if BOLD
            style_context.add_class("-vala-panel-appmenu-bold");
#endif
            style_context.add_class("-vala-panel-appmenu-private");
            if (appmenu_actions != null)
                this.insert_action_group("app",appmenu_actions);
            if (menubar_actions != null)
                this.insert_action_group("win",menubar_actions);
            if (unity_actions != null)
                this.insert_action_group("unity",unity_actions);
            this.show_all();
        }
        public bool on_scroll_event(Gdk.EventScroll event)
        {
            print("scroll event: ");
            var val = scroll_adj.get_value();
            var incr = scroll_adj.get_step_increment();
            if (event.direction == Gdk.ScrollDirection.UP)
            {
                print ("up\n");
                scroll_adj.set_value(val - incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.DOWN)
            {
                print ("down\n");
                scroll_adj.set_value(val + incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.LEFT)
            {
                scroll_adj.set_value(val - incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.RIGHT)
            {
                scroll_adj.set_value(val + incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.SMOOTH)
            {
                print("smooth %f %f\n", event.delta_x, event.delta_y);
                scroll_adj.set_value(val + incr * (event.delta_y + event.delta_x));
                return true;
            }
            return false;
        }
    }
}
