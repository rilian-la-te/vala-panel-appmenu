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
using DBusMenu;

namespace Appmenu
{
    internal class MenuWidgetImporter: MenuWidget
    {
        public string object_name {get; private set construct;}
        public ObjectPath object_path {get; private set construct;}
        private DBusMenu.Importer importer;
        public MenuWidgetImporter(uint window_id, string name, ObjectPath path, Bamf.Application? app)
        {
            this.window_id = window_id;
            this.object_name = name;
            this.object_path = path;
            this.set_appmenu(null);
            this.set_menubar(null);
            if (app != null)
            {
                var appmenu = new BamfAppmenu(app);
                this.set_appmenu(appmenu);
                appmenu.show();
            }
//            if (DBusMenu.GtkClient.check(name,(string)path))
//            {
                importer = new DBusMenu.Importer(name,(string)path);
                importer.notify["model"].connect((s, p)=>{
                    var menubar = new Gtk.MenuBar.from_model(importer.model);
                    this.set_menubar(menubar);
                    this.insert_action_group("dbusmenu",importer.action_group);
                    menubar.show();
                });
//            }
            this.show();
        }
    }
}
