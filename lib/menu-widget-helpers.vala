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
    internal abstract class Helper: Object
    {

    }
    internal class MenuModelHelper: Helper
    {
        private Helper bamf_helper = null;
        public MenuModelHelper(MenuWidget w,Bamf.Application? app,Bamf.Window window)
        {
//            this.window_id = window.get_xid();
//            this.widget = w;
            GLib.ActionGroup? appmenu_actions = null;
            GLib.ActionGroup? menubar_actions = null;
            GLib.ActionGroup? unity_actions = null;
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
            if (name == null && window != null)
                name = window.get_name();
            if (name == null)
                name = Environment.get_prgname();
            GLib.MenuModel? appmenu = null;
            if (app_menu_path != null)
            {
                appmenu = new GLib.Menu();
                (appmenu as GLib.Menu).append_submenu(name,DBusMenuModel.get(dbusconn,gtk_unique_bus_name,app_menu_path));
                w.set_appmenu(appmenu);
            }
            else if (app != null)
                bamf_helper = new BamfAppmenu(w,app);
            if (menubar_path != null)
            {
                var menubar = DBusMenuModel.get(dbusconn,gtk_unique_bus_name,menubar_path);
                w.set_menubar(menubar);
            }
            if (appmenu_actions != null)
                w.insert_action_group("app",appmenu_actions);
            if (menubar_actions != null)
                w.insert_action_group("win",menubar_actions);
            if (unity_actions != null)
                w.insert_action_group("unity",unity_actions);
        }
    }
    internal class DBusMenuHelper: Helper
    {
        private DBusMenu.Importer importer = null;
        private Helper bamf_helper = null;
        public DBusMenuHelper(MenuWidget w, uint window_id, string name, ObjectPath path, Bamf.Application? app)
        {
            if (app != null)
                bamf_helper = new BamfAppmenu(w,app);
            importer = new DBusMenu.Importer(name,(string)path);
            importer.notify["model"].connect((s, p)=>{
                if((w.completed_menus & MenuWidgetCompletionFlags.MENUBAR) == 0)
                    w.set_menubar(importer.model);
                w.insert_action_group("dbusmenu",importer.action_group);
            });
        }
    }
}
