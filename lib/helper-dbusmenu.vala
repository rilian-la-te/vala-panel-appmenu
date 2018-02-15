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
        private ulong connect_handler = 0;
        public DBusMenuHelper(MenuWidget w, string name, ObjectPath path, string? title, DesktopAppInfo? info)
        {
            dbus_helper = new DBusAppMenu(w, title, name, info);
            importer = new DBusMenu.Importer(name,(string)path);
            connect_handler = Signal.connect(importer,"notify::model",(GLib.Callback)on_model_changed_cb,w);
        }
        private static void on_model_changed_cb(DBusMenu.Importer importer, GLib.ParamSpec pspec, MenuWidget w)
        {
            w.insert_action_group("dbusmenu",importer.action_group);
            w.set_menubar(importer.model);
        }
        ~DBusMenuHelper()
        {
            importer.disconnect(connect_handler);
        }
    }
}
