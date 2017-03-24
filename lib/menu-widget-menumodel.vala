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
        public MenuWidgetMenumodel(Bamf.Application? app,Bamf.Window? window)
        {
            if (window != null)
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
                Gtk.MenuBar? appmenu = null;
                if (app_menu_path != null)
                {
                    var menu = new GLib.Menu();
                    menu.append_submenu(name,DBusMenuModel.get(dbusconn,gtk_unique_bus_name,app_menu_path));
                    appmenu = new Gtk.MenuBar.from_model(menu);
                }
                else if (app != null)
                    appmenu = new BamfAppmenu(app);
                if (appmenu != null)
                {
                    this.add(appmenu);
                    completed_menus |= MenuWidgetCompletionFlags.APPMENU;
                }
                if (menubar_path != null)
                {
                    var menubar = new Gtk.MenuBar.from_model(DBusMenuModel.get(dbusconn,gtk_unique_bus_name,menubar_path));
                    this.add(menubar);
                    if (menubar.get_children().length() > 0)
                        completed_menus |= MenuWidgetCompletionFlags.MENUBAR;
                }
                if (appmenu_actions != null)
                    this.insert_action_group("app",appmenu_actions);
                if (menubar_actions != null)
                    this.insert_action_group("win",menubar_actions);
                if (unity_actions != null)
                    this.insert_action_group("unity",unity_actions);
            }
            this.show_all();
        }
    }
}
