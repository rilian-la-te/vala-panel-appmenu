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
    public class BamfAppmenu : Gtk.MenuBar
    {
        private static const string UNITY_QUICKLISTS_KEY = "X-Ayatana-Desktop-Shortcuts";
        private static const string UNITY_QUICKLISTS_SHORTCUT_GROUP_NAME = "%s Shortcut Group";
        private static const string UNITY_QUICKLISTS_TARGET_KEY = "TargetEnvironment";
        private static const string UNITY_QUICKLISTS_TARGET_VALUE = "Unity";
        private unowned Bamf.Application app;
        private GLib.Menu window_section;
        private static const GLib.ActionEntry[] entries =
        {
            {"new-window", activate_new, null, null, null},
#if WNCK
            {"active-window", activate_window, "u", null,null},
            {"close-this",activate_close_this,null,null,null},
            {"close-all",activate_close_all,null,null,null},
#endif
            {"activate-action", activate_action, "s", null, null},
            {"activate-unity-desktop-shortcut",activate_unity,"s",null,null}
        };
        public BamfAppmenu(Bamf.Application app)
        {
            this.app = app;
            var configurator = new SimpleActionGroup();
            configurator.add_action_entries(entries,this);
            this.insert_action_group("conf",configurator);
            var desktop_file = app.get_desktop_file();
            var builder = new Builder.from_resource("/org/vala-panel/appmenu/desktop-menus.ui");
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
            app.window_added.connect(on_window_added);
            app.window_removed.connect(on_window_removed);
            window_section = builder.get_object("active-windows") as GLib.Menu;
            foreach(unowned Bamf.Window window in app.get_windows())
                on_window_added(window);
            var gmenu = new GLib.Menu();
            menu.freeze();
            gmenu.append_submenu(app.get_name(),menu);
            this.bind_model(gmenu,null,true);
            this.show_all();
        }
        ~BamfAppmenu()
        {
            SignalHandler.disconnect_by_data(app,this);
            app = null;
            window_section = null;
        }
        private void on_window_added(Bamf.Window window)
        {
            var menuitem = new GLib.MenuItem(window.get_name(),null);
            menuitem.set_action_and_target_value("conf.active-window",new Variant.uint32(window.get_xid()));
            window_section.append_item(menuitem);
        }
        private void on_window_removed(Bamf.Window win)
        {
            for(var i = 0; i< window_section.get_n_items(); i++)
            {
                uint xid = window_section.get_item_attribute_value(i,GLib.Menu.ATTRIBUTE_TARGET,VariantType.UINT32).get_uint32();
                if (xid == win.get_xid())
                {
                    window_section.remove(i);
                    return;
                }
            }
        }
        private void activate_new(GLib.SimpleAction action, Variant? param)
        {
            unowned string desktop_file = app.get_desktop_file();
            var data = new SpawnData();
            if (desktop_file != null)
            {
                try {
                    var info = new DesktopAppInfo.from_filename(desktop_file);
                    info.launch_uris_as_manager(null,
                                                this.get_display().get_app_launch_context(),
                                                SpawnFlags.SEARCH_PATH,
                                                data.child_spawn_func,(a,b)=>{});
                } catch (Error e) {
                    stderr.printf("%s\n",e.message);
                }
            }
        }
        private void activate_action(GLib.SimpleAction action, Variant? param)
        {
            var action_name = param.get_string();
            var desktop_file = app.get_desktop_file();
            if (desktop_file != null)
            {
                var info = new DesktopAppInfo.from_filename(desktop_file);
                info.launch_action(action_name,this.get_display().get_app_launch_context());
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
                    info.launch_uris_as_manager(null,
                                                this.get_display().get_app_launch_context(),
                                                SpawnFlags.SEARCH_PATH,
                                                data.child_spawn_func,(a,b)=>{});
                } catch (Error e) {
                    stderr.printf("%s\n",e.message);
                }
            }
        }
#if WNCK
        /* Taken from Plank */
        private void activate_window(GLib.SimpleAction action, Variant? param)
        {
            var xid = param.get_uint32();
            unowned Wnck.Window w = Wnck.Window.@get (xid);
            if (w == null)
            {
                Wnck.Screen.get_default().force_update();
                w = Wnck.Window.@get (xid);
            }
            var time = Gtk.get_current_event_time ();
            unowned Wnck.Workspace? workspace = w.get_workspace ();

            if (workspace != null && workspace != w.get_screen ().get_active_workspace ())
                workspace.activate (time);

            if (w.is_minimized ())
                w.unminimize (time);

            w.activate_transient (time);
        }
        /* Taken from Plank */
        private void activate_close_all(SimpleAction action, Variant? param)
        {
            Array<uint32>? xids = app.get_xids ();

            warn_if_fail (xids != null);

            for (var i = 0; xids != null && i < xids.length; i++)
            {
                unowned Wnck.Window window = Wnck.Window.@get (xids.index (i));
                if (window != null && !window.is_skip_tasklist ())
                    window.close (Gtk.get_current_event_time ());
            }
        }
        private void activate_close_this(SimpleAction action, Variant? param)
        {
            unowned MenuWidget widget = this.get_parent() as MenuWidget;
            unowned Wnck.Window window = Wnck.Window.@get ((ulong)widget.window_id);
            if (window == null)
            {
                Wnck.Screen.get_default().force_update();
                window = Wnck.Window.@get ((ulong)widget.window_id);
            }
            if (window != null && !window.is_skip_tasklist ())
                window.close (Gtk.get_current_event_time ());
        }
#endif
    }
}
