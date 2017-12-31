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
    public static int main(string[] args)
    {
        var app = new App();
        return app.run(args);
    }
    public class App: GLib.Application
    {
        private DBusMenuRegistrar registrar = new DBusMenuRegistrar();
        private uint dbusmenu_binding = 0;
        private const OptionEntry[] options =
        {
            { "version", 'v', 0, OptionArg.NONE, null, N_("Print version and exit"), null },
            { null }
        };
        public App()
        {
            Object(application_id: "org.valapanel.appmenu.registrar",
                    flags: GLib.ApplicationFlags.HANDLES_COMMAND_LINE,
                    resource_base_path: "/org/valapanel/registrar");
        }
        construct
        {
            add_main_option_entries(options);
        }

        protected override int handle_local_options(VariantDict opts)
        {
            if (opts.contains("version"))
            {
                stdout.printf(_("%s - Version %s\n"),GLib.Environment.get_application_name(),
                                                    Config.VERSION);
                return 0;
            }
            return -1;
        }
        public override bool dbus_register (DBusConnection connection, string object_path) throws Error {
            bool ret = base.dbus_register (connection, object_path);
            dbusmenu_binding = connection.register_object (DBUSMENU_REG_OBJECT, registrar);
            return ret && dbusmenu_binding > 0;
        }

        public override void dbus_unregister (DBusConnection connection, string object_path) {
            // Do our own stuff here, e.g. unexport any D-Bus objects we exported in the dbus_register
            //  hook above. Be sure to check that we actually did export them, since the hook
            //  above might have returned early due to the parent class' hook returning false!
            connection.unregister_object (dbusmenu_binding);
            base.dbus_unregister (connection, object_path);
        }
    }
}
