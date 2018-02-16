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
    public enum ModelType
    {
        NONE,
        STUB,
        DESKTOP,
        MENUMODEL,
        DBUSMENU
    }
    internal abstract class Helper: Object
    {

    }
    public abstract class Backend : Object
    {
        protected ModelType type = ModelType.NONE;
        protected static DBusMenuRegistrarProxy proxy;
        static construct
        {
            proxy = new DBusMenuRegistrarProxy();
        }
        construct
        {
            try
            {
                var con = Bus.get_sync(BusType.SESSION);
                con.call.begin(
                    "org.valapanel.AppMenu.Registrar",
                    "/Registrar",
                    "org.valapanel.AppMenu.Registrar",
                    "Reference",
                    null,null,
                    DBusCallFlags.NONE, -1);
            }
            catch(Error e)
            {
                stderr.printf("%s\n",e.message);
            }
        }
        ~Backend()
        {
            try
            {
                var con = Bus.get_sync(BusType.SESSION,null);
                con.call.begin(
                    "org.valapanel.AppMenu.Registrar",
                    "/Registrar",
                    "org.valapanel.AppMenu.Registrar",
                    "UnReference",
                    null,null,
                    DBusCallFlags.NO_AUTO_START, -1);
            }
            catch(Error e)
            {
                stderr.printf("%s\n",e.message);
            }
        }
        public signal void active_model_changed();
        public abstract void set_active_window_menu(MenuWidget widget);
    }
}
