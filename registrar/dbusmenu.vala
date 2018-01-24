/*
 * vala-panel-appmenu
 * Copyright (C) 2017 Konstantin Pugin <ria.freelander@gmail.com>
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
    public const string DBUSMENU_DBUS_NAME = "com.canonical.AppMenu.Registrar";
    public const string DBUSMENU_REG_IFACE = "com.canonical.AppMenu.Registrar";
    public const string DBUSMENU_REG_OBJECT = "/com/canonical/AppMenu/Registrar";
    internal struct DBusAddress
    {
        string name;
        ObjectPath path;
    }

    [DBus (name = "com.canonical.AppMenu.Registrar")]
    public class DBusMenuRegistrar : Object
    {
        private HashTable<uint,DBusAddress?> menus;
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        public DBusMenuRegistrar()
        {
            Object();
        }
        construct
        {
            menus = new HashTable<uint,DBusAddress?>(direct_hash,direct_equal);
        }
        public void register_window(uint window_id, ObjectPath menu_object_path, BusName sender)
        {
            DBusAddress addr = DBusAddress();
            addr.name = (string)sender;
            addr.path = menu_object_path;
            menus.insert(window_id,addr);
            window_registered(window_id,sender,menu_object_path);
        }
        public void unregister_window(uint window_id)
        {
            if (!(window_id in menus))
                return;
            menus.remove(window_id);
            window_unregistered(window_id);
        }
        public void get_menu_for_window(uint window, out string service, out ObjectPath path)
        {
            unowned DBusAddress? menu = menus.lookup(window);
            if (menu != null)
            {
                service = menu.name;
                path = menu.path;
            }
            else
            {
                service = "";
                path = new ObjectPath("/");
            }
//            if (menu != null && !DBusMenu.GtkClient.check(service,path))
//            {
//                service = "";
//                path = new ObjectPath("/");
//                unregister_window(window);
//            }
        }
        public void get_menus([DBus (signature="a(uso)")] out Variant menus)
        {
            VariantBuilder builder = new VariantBuilder(new VariantType("a(uso)"));
            this.menus.foreach((k,v)=>{
                string service;
                ObjectPath path;
                get_menu_for_window(k,out service,out path);
                builder.add("(uso)",k,service,path);
            });
            menus = builder.end();
        }
    }
}
