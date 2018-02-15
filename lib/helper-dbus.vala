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
    public const string DBUS_DEFAULT_PATH = "/org/freedesktop/DBus";
    public const string DBUS_DEFAULT_NAME= "org.freedesktop.DBus";
    [DBus (name = "org.freedesktop.DBus")]
    public interface DBusMain : DBusProxy
    {
        [DBus (name = "GetConnectionUnixProcessID")]
        public abstract uint get_connection_unix_process_id(string id) throws Error;
        public abstract int start_service_by_name(string service, int flags) throws Error;
        public abstract string[] list_activatable_names() throws Error;
    }
    internal class DBusAppMenu : Helper
    {
        private const string UNITY_QUICKLISTS_KEY = "X-Ayatana-Desktop-Shortcuts";
        private const string UNITY_QUICKLISTS_SHORTCUT_GROUP_NAME = "%s Shortcut Group";
        private const string UNITY_QUICKLISTS_TARGET_KEY = "TargetEnvironment";
        private const string UNITY_QUICKLISTS_TARGET_VALUE = "Unity";


        private DBusMain dbus;
        private DesktopAppInfo? info = null;
        private string? connection = null;
        private unowned MenuWidget widget;
        private GLib.Menu all_menu = new GLib.Menu();

        private const GLib.ActionEntry[] entries =
        {
            {"new", activate_new, null, null, null},
            {"activate-action", activate_action, "s", null, null},
            {"activate-unity-desktop-shortcut",activate_unity,"s",null,null},
            {"quit", activate_quit, null, null, null},
        };
        construct
        {
            try {
                dbus = Bus.get_proxy_sync(BusType.SESSION, DBUS_DEFAULT_NAME, DBUS_DEFAULT_PATH);
            } catch (Error e) {
                debug("%s\n",e.message);
            }
        }
        public DBusAppMenu(MenuWidget w, string? name, string? connection, DesktopAppInfo? info)
        {
            this.widget = w;
            var configurator = new SimpleActionGroup();
            configurator.add_action_entries(entries,this);
            var builder = new Builder.from_resource("/org/vala-panel/appmenu/desktop-menus.ui");
            builder.set_translation_domain(Config.GETTEXT_PACKAGE);
            unowned GLib.Menu menu = builder.get_object("appmenu-stub") as GLib.Menu;
            if(connection != null)
                this.connection = connection;
            else
                (configurator.lookup_action("quit") as SimpleAction).set_enabled(false);
            if (info != null)
            {
                this.info = info;
                unowned GLib.Menu section = builder.get_object("desktop-actions") as GLib.Menu;
                foreach(unowned string action in info.list_actions())
                    section.append(info.get_action_name(action),"conf.activate-action('%s')".printf(action));
                section.freeze();
                try{
                    section = builder.get_object("unity-actions") as GLib.Menu;
                    var keyfile = new KeyFile();
                    keyfile.load_from_file(info.get_filename(),KeyFileFlags.NONE);
                    var unity_list = keyfile.get_string_list(KeyFileDesktop.GROUP,UNITY_QUICKLISTS_KEY);
                    foreach(unowned string action in unity_list)
                    {
                        var action_name = keyfile.get_locale_string(UNITY_QUICKLISTS_SHORTCUT_GROUP_NAME.printf(action),KeyFileDesktop.KEY_NAME);
                        section.append(action_name,"conf.activate-unity-desktop-shortcut('%s')".printf(action));
                    }
                    section.freeze();
                } catch (Error e) {
                    debug("%s\n",e.message);
                }
            }
            else if (connection == null)
            {
                (configurator.lookup_action("new") as SimpleAction).set_enabled(false);
            }
            string res_name = name ?? _("Application");
            if (name.length >= 28)
                res_name = name[0:25]+"...";
            all_menu.append_submenu(res_name,menu);
            all_menu.freeze();
            widget.insert_action_group("conf",configurator);
            widget.set_appmenu(all_menu);
        }
        private void activate_new(GLib.SimpleAction action, Variant? param)
        {
            if (info != null)
            {
                MenuMaker.launch(info,new List<string>(),widget);
            }
            else if (connection != null)
            {
                //FIXME: Now using only first part, not parameters
                try {
                    string str = "/proc/%u/cmdline".printf(dbus.get_connection_unix_process_id(this.connection));
                    string exec = Launcher.posix_get_cmdline_string(str);
                    var appinfo  = AppInfo.create_from_commandline(exec,null,0) as DesktopAppInfo;
                    MenuMaker.launch(appinfo,new List<string>(),widget);
                } catch (Error e) {
                    stderr.printf("%s\n",e.message);
                }
            }
        }
        private void activate_quit(GLib.SimpleAction action, Variant? param)
        {
            try {
                Posix.kill((Posix.pid_t)dbus.get_connection_unix_process_id(this.connection), Posix.SIGQUIT);
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
            }
        }
        private void activate_action(GLib.SimpleAction action, Variant? param)
        {
            var action_name = param.get_string();
            info.launch_action(action_name,widget.get_display().get_app_launch_context());
        }
        private void activate_unity(GLib.SimpleAction action, Variant? param)
        {
            unowned string action_name = param.get_string();
            try {
                var keyfile = new KeyFile();
                keyfile.load_from_file(info.get_filename(),KeyFileFlags.NONE);
                var exec = keyfile.get_string(UNITY_QUICKLISTS_SHORTCUT_GROUP_NAME.printf(action_name),KeyFileDesktop.KEY_EXEC);
                var appinfo  = AppInfo.create_from_commandline(exec,null,0) as DesktopAppInfo;
                MenuMaker.launch(appinfo,new List<string>(),widget);
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
            }
        }
    }
}
