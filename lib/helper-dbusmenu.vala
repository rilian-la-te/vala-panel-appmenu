/*
 * vala-panel-appmenu
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
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
    internal class DBusMenuHelper: Helper
    {
        private DBusMenu.Importer importer = null;
        private Helper dbus_helper = null;
        public DBusMenuHelper(MenuWidget w, string name, ObjectPath path, string? title, DesktopAppInfo? info)
        {
            dbus_helper = new DBusAppMenu(w, title, name, info);
            importer = new DBusMenu.Importer(name,(string)path);
            bool is_model_set = false;
            ulong connect_handler = 0;
            connect_handler = importer.notify["model"].connect((s, p)=>{
                if(!is_model_set)
                {
                    w.insert_action_group("dbusmenu",importer.action_group);
                    w.set_menubar(importer.model);
                    is_model_set = true;
                    importer.disconnect(connect_handler);
                }
            });
        }
    }
}
