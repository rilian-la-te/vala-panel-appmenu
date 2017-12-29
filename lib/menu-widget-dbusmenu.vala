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
    internal class MenuWidgetDbusmenu: MenuWidget
    {
        public string object_name {get; private set construct;}
        public ObjectPath object_path {get; private set construct;}
        private DBusMenu.GtkClient client;
        public MenuWidgetDbusmenu(uint window_id, string name, ObjectPath path, Bamf.Application? app)
        {
            this.window_id = window_id;
            this.object_name = name;
            this.object_path = path;
            if (app != null)
            {
                appmenu = new BamfAppmenu(app);
                this.add(appmenu);
                appmenu.show();
                completed_menus |= MenuWidgetCompletionFlags.APPMENU;
            }
            if (DBusMenu.GtkClient.check(name,(string)path))
            {
                client = new DBusMenu.GtkClient(name,(string)path);
                menubar = new Gtk.MenuBar();
                client.attach_to_menu(menubar);

                scroller.add(menubar);
                this.add(scroller);
                menubar.show();
                scroller.show();

                completed_menus |= MenuWidgetCompletionFlags.MENUBAR;
            }
            conf_menus();
            this.show();
        }
    }
}
