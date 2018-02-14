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
    internal class MenuModelHelper: Helper
    {
        private Helper dbus_helper = null;
        public MenuModelHelper(MenuWidget w,
                               string? gtk_unique_bus_name,
                               string? app_menu_path,
                               string? menubar_path,
                               string? application_path,
                               string? window_path,
                               string? unity_path,
                               string? title,
                               DesktopAppInfo? info)
        {
            GLib.ActionGroup? appmenu_actions = null;
            GLib.ActionGroup? menubar_actions = null;
            GLib.ActionGroup? unity_actions = null;
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
            GLib.MenuModel? appmenu = null;
            if (app_menu_path != null)
            {
                appmenu = new GLib.Menu();
                (appmenu as GLib.Menu).append_submenu(title,DBusMenuModel.get(dbusconn,gtk_unique_bus_name,app_menu_path));
                w.set_appmenu(appmenu);
            }
            else
                dbus_helper = new DBusAppMenu(w, title, gtk_unique_bus_name, info);
            if (menubar_path != null)
            {
                var menubar = DBusMenuModel.get(dbusconn,gtk_unique_bus_name,menubar_path);
                w.set_menubar(menubar);
            }
            else
                w.set_menubar(null);
            if (appmenu_actions != null)
                w.insert_action_group("app",appmenu_actions);
            if (menubar_actions != null)
                w.insert_action_group("win",menubar_actions);
            if (unity_actions != null)
                w.insert_action_group("unity",unity_actions);
        }
    }
}
