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
    internal class BamfAppmenu : Helper
    {
        private const string UNITY_QUICKLISTS_KEY = "X-Ayatana-Desktop-Shortcuts";
        private const string UNITY_QUICKLISTS_SHORTCUT_GROUP_NAME = "%s Shortcut Group";
        private const string UNITY_QUICKLISTS_TARGET_KEY = "TargetEnvironment";
        private const string UNITY_QUICKLISTS_TARGET_VALUE = "Unity";
        private unowned Bamf.Application app;
        private unowned MenuWidget widget;
        private GLib.Menu all_menu = new GLib.Menu();
        private const GLib.ActionEntry[] entries =
        {
            {"new-instance", activate_new, null, null, null},
            {"activate-action", activate_action, "s", null, null},
            {"activate-unity-desktop-shortcut",activate_unity,"s",null,null},
            {"quit-all-instances", activate_quit_all, null, null, null},
            {"kill-all-instances", activate_kill_all, null, null, null}
        };
        public BamfAppmenu(MenuWidget w, Bamf.Application app)
        {
            this.app = app;
            this.widget = w;
            var configurator = new SimpleActionGroup();
            configurator.add_action_entries(entries,this);
            var desktop_file = app.get_desktop_file();
            var builder = new Builder.from_resource("/org/vala-panel/appmenu/desktop-menus.ui");
            builder.set_translation_domain(Config.GETTEXT_PACKAGE);
            unowned GLib.Menu menu = builder.get_object("appmenu-bamf") as GLib.Menu;
            if (desktop_file != null)
            {
                unowned GLib.Menu section = builder.get_object("desktop-actions") as GLib.Menu;
                var info = new DesktopAppInfo.from_filename(desktop_file);
                foreach(unowned string action in info.list_actions())
                    section.append(info.get_action_name(action),"conf.activate-action('%s')".printf(action));
                section.freeze();
                try{
                    section = builder.get_object("unity-actions") as GLib.Menu;
                    var keyfile = new KeyFile();
                    keyfile.load_from_file(desktop_file,KeyFileFlags.NONE);
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
            else
            {
                (configurator.lookup_action("new-instance") as SimpleAction).set_enabled(false);
                (configurator.lookup_action("quit-all-instances") as SimpleAction).set_enabled(false);
                (configurator.lookup_action("kill-all-instances") as SimpleAction).set_enabled(false);
            }
            var name = app.get_name();
            if (desktop_file == null && name.length >= 28)
                name = name[0:25]+"...";
            all_menu.append_submenu(name,menu);
            all_menu.freeze();
            widget.insert_action_group("conf",configurator);
            widget.set_appmenu(all_menu);
        }
        private void activate_new(GLib.SimpleAction action, Variant? param)
        {
            unowned string desktop_file = app.get_desktop_file();
            var data = new SpawnData();
            if (desktop_file != null)
            {
                try {
                    var info = new DesktopAppInfo.from_filename(desktop_file);
                    info.launch_uris_as_manager(new List<string>(),
                                                widget.get_display().get_app_launch_context(),
                                                SpawnFlags.SEARCH_PATH,
                                                data.child_spawn_func,(a,b)=>{});
                } catch (Error e) {
                    stderr.printf("%s\n",e.message);
                }
            }
        }
        private void activate_killall(bool force_kill)
        {
            var desktop_file = app.get_desktop_file();
            var info = new DesktopAppInfo.from_filename(desktop_file);
            var exec_str = info.get_executable();
            var signal_name = force_kill ? "KILL" : "QUIT";
            try {
                string[] spawn_args = {"killall", "-s", signal_name, exec_str};
                string[] spawn_env = Environ.get ();
                Pid child_pid;

                Process.spawn_async ("/",
                    spawn_args,
                    spawn_env,
                    SpawnFlags.SEARCH_PATH | SpawnFlags.DO_NOT_REAP_CHILD | SpawnFlags.STDOUT_TO_DEV_NULL | SpawnFlags.STDERR_TO_DEV_NULL,
                    null,
                    out child_pid);

                ChildWatch.add (child_pid, (pid, status) => {
                    // Triggered when the child indicated by child_pid exits
                    Process.close_pid (pid);
                });
            } catch (SpawnError e) {
                stderr.printf ("Error: %s\n", e.message);
            }
        }
        private void activate_quit_all(GLib.SimpleAction action, Variant? param)
        {
            activate_killall(false);
        }
        private void activate_kill_all(GLib.SimpleAction action, Variant? param)
        {
            activate_killall(false);
        }
        private void activate_action(GLib.SimpleAction action, Variant? param)
        {
            var action_name = param.get_string();
            var desktop_file = app.get_desktop_file();
            if (desktop_file != null)
            {
                var info = new DesktopAppInfo.from_filename(desktop_file);
                info.launch_action(action_name,widget.get_display().get_app_launch_context());
            }
        }
        private void activate_unity(GLib.SimpleAction action, Variant? param)
        {
            unowned string action_name = param.get_string();
            var desktop_file = app.get_desktop_file();
            var data = new SpawnData();
            if (desktop_file != null)
            {
                try {
                    var keyfile = new KeyFile();
                    keyfile.load_from_file(desktop_file,KeyFileFlags.NONE);
                    var exec = keyfile.get_string(UNITY_QUICKLISTS_SHORTCUT_GROUP_NAME.printf(action_name),KeyFileDesktop.KEY_EXEC);
                    var info  = AppInfo.create_from_commandline(exec,null,0) as DesktopAppInfo;
                    info.launch_uris_as_manager(new List<string>(),
                                                widget.get_display().get_app_launch_context(),
                                                SpawnFlags.SEARCH_PATH,
                                                data.child_spawn_func,(a,b)=>{});
                } catch (Error e) {
                    stderr.printf("%s\n",e.message);
                }
            }
        }
    }
}
