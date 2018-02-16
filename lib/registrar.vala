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

namespace Appmenu
{
    public const string DBUS_NAME = "com.canonical.AppMenu.Registrar";
    public const string REG_IFACE = "com.canonical.AppMenu.Registrar";
    public const string REG_OBJECT = "/com/canonical/AppMenu/Registrar";
    public const string KDE_APPMENU_NAME = "org.kde.kappmenu";
    public const string KDE_APPMENU_VIEW_NAME = "org.kde.kappmenuview";
    public const string KDE_APPMENU_OBJECT = "/KAppMenu";

    [DBus (name = "com.canonical.AppMenu.Registrar")]
    public interface OuterRegistrar : DBusProxy
    {
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        public abstract void register_window(uint window, ObjectPath menu_object_path) throws Error;
        public abstract void unregister_window(uint window) throws Error;
        public abstract void get_menu_for_window(uint window, out string service, out ObjectPath path) throws Error;
        public abstract void get_menus([DBus (signature="a(uso)")] out Variant menus) throws Error;
    }
    [DBus (name = "org.kde.kappmenu")]
    public interface KDEAppMenu : DBusProxy
    {
        public abstract void show_menu(int x,int y,string service, ObjectPath path,int actionId) throws Error;
        public abstract void reconfigure() throws Error;
        public signal void reconfigured();
        public signal void show_request(string service, ObjectPath path,int actionId);
        public signal void menu_shown(string service, ObjectPath path);
        public signal void menu_hidden(string service, ObjectPath path);
    }
    public class DBusMenuRegistrarProxy: Object
    {
        public bool have_registrar {get; private set;}
        private OuterRegistrar outer_registrar;
        private uint watched_name;
        public DBusMenuRegistrarProxy()
        {
            Object();
        }
        public signal void registrar_changed(bool have_registrar);
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        private void create_outer_registrar()
        {
            watched_name = Bus.watch_name(BusType.SESSION,REG_IFACE,GLib.BusNameWatcherFlags.NONE,
                                                    () => {
                                                        try{
                                                            outer_registrar = Bus.get_proxy_sync(BusType.SESSION,REG_IFACE,REG_OBJECT);
                                                            outer_registrar.window_registered.connect((w,s,p)=>{this.window_registered(w,s,p);});
                                                            outer_registrar.window_unregistered.connect((w)=>{this.window_unregistered(w);});
                                                            have_registrar = true;
                                                            registrar_changed(true);
                                                        } catch (Error e) {stderr.printf("%s\n",e.message);}
                                                        },
                                                    () => {
                                                        have_registrar = false;
                                                        outer_registrar = null;
                                                        registrar_changed(false);
                                                        }
                                                    );
        }
        construct
        {
            have_registrar = false;
            try{
                var con = Bus.get_sync(BusType.SESSION);
                con.call_sync(
                    DBUS_NAME,
                    REG_OBJECT,
                    REG_IFACE,
                    "GetMenus",
                    null,null,
                    DBusCallFlags.NONE, -1);
            }
            catch(Error e)
            {
                stderr.printf("%s\n",e.message);
            }
            create_outer_registrar();
        }
        public void get_menu_for_window(uint window, out string name, out ObjectPath path)
        {
            name = "";
            path = new ObjectPath("/");
            if (!have_registrar)
                return;
            try{
                outer_registrar.get_menu_for_window(window,out name, out path);
            } catch (Error e) {stderr.printf("%s\n",e.message);}
        }
        ~DBusMenuRegistrarProxy()
        {
            Bus.unwatch_name(watched_name);
        }
    }
}
