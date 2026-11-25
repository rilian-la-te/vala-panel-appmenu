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

namespace Appmenu
{
    internal class DBusMenuHelper: Helper
    {
        private DBusMenu.Importer importer = null;
        private Helper dbus_helper = null;
        private ulong connect_handler = 0;
        public DBusMenuHelper(GLib.AppLaunchContext ctx, string name, ObjectPath path, string? title, DesktopAppInfo? info)
        {
            dbus_helper = new DBusAppMenu.with_connection(ctx, title, name, info);
            importer = new DBusMenu.Importer(name,(string)path);
            connect_handler = Signal.connect(importer,"notify::model",(GLib.Callback)on_model_changed_cb,this);
        }
        private static void on_model_changed_cb(DBusMenu.Importer importer, GLib.ParamSpec pspec, DBusMenuHelper self)
        {
            self.actions.insert("dbusmenu",importer.action_group);
            self.menu = create_model(self.dbus_helper.menu, importer.model, false);
            merge_actions(self.dbus_helper.actions, self.actions);
            self.content_updated();
        }
        ~DBusMenuHelper()
        {
            importer.disconnect(connect_handler);
        }
    }
}
