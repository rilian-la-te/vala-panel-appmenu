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
        private Bamf.Application app;
        private GLib.Menu menu;
        private GLib.Menu window_section;
        private SimpleActionGroup configurator;
        private ulong adding_handler;
        private ulong removing_handler;
        private static const GLib.ActionEntry[] entries =
        {
            {"new-window", activate_new, null, null, null},
            {"activate-action", activate_action, "s", null, null},
            {"activate-unity-desktop-shortcut",activate_unity,"s",null,null},
            {"active-window", activate_window, "u", null,null},
            {"close-this",activate_close_this,null,null,null},
            {"close-all",activate_close_all,null,null,null}
        };
        public BamfAppmenu(Bamf.Application app)
        {
            this.app = app;
            configurator = new SimpleActionGroup();
            configurator.add_action_entries(entries,this);
            this.insert_action_group("conf",configurator);
            var desktop_file = app.get_desktop_file();
            var submenu = new GLib.Menu();
            var section = new GLib.Menu();
            section.append(_("_New Window..."),"conf.new-window");
            section.append(_("Close _This"),"conf.close-this");
            section.append(_("Close _All"),"conf.close-all");
            submenu.append_section(null,section);
            section = new GLib.Menu();
            if (desktop_file != null)
            {
                var info = new DesktopAppInfo.from_filename(desktop_file);
                foreach(var action in info.list_actions())
                    section.append(info.get_action_name(action),"conf.activate-action('%s')".printf(action));
                submenu.append_section(null,section);
                try{
                    section = new GLib.Menu();
                    var keyfile = new KeyFile();
                    keyfile.load_from_file(desktop_file,KeyFileFlags.NONE);
                    var unity_list = keyfile.get_string_list(KeyFileDesktop.GROUP,UNITY_QUICKLISTS_KEY);
                    foreach(var action in unity_list)
                    {
                        var action_name = keyfile.get_locale_string(UNITY_QUICKLISTS_SHORTCUT_GROUP_NAME.printf(action),KeyFileDesktop.KEY_NAME);
                        section.append(action_name,"conf.activate-unity-desktop-shortcut('%s')".printf(action));
                    }
                    submenu.append_section(null,section);
                } catch (Error e) {
                    debug("%s\n",e.message);
                }
            }
            adding_handler = app.window_added.connect(on_window_added);
            removing_handler = app.window_removed.connect(on_window_removed);
            window_section = new GLib.Menu();
            foreach(var window in app.get_windows())
                on_window_added(window);
            submenu.append_section(null,window_section);
            menu = new GLib.Menu();
            menu.append_submenu(app.get_name(),submenu);
            this.bind_model(menu,null,true);
            this.show_all();
        }
        ~BamfAppmenu()
        {
            if (SignalHandler.is_connected(app,adding_handler))
                app.disconnect(adding_handler);
            if (SignalHandler.is_connected(app,removing_handler))
                app.disconnect(removing_handler);
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
            var desktop_file = app.get_desktop_file();
            if (desktop_file != null)
            {
                try {
                    var info = new DesktopAppInfo.from_filename(desktop_file);
                    info.launch(null,this.get_display().get_app_launch_context());
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
            var action_name = param.get_string();
            var desktop_file = app.get_desktop_file();
            if (desktop_file != null)
            {
                try {
                    var keyfile = new KeyFile();
                    keyfile.load_from_file(desktop_file,KeyFileFlags.NONE);
                    var exec = keyfile.get_string(UNITY_QUICKLISTS_SHORTCUT_GROUP_NAME.printf(action_name),KeyFileDesktop.KEY_EXEC);
                    var info  = AppInfo.create_from_commandline(exec,null,0);
                    info.launch(null,this.get_display().get_app_launch_context());
                } catch (Error e) {
                    stderr.printf("%s\n",e.message);
                }
            }
        }
        /* Taken from Plank */
        private void activate_window(GLib.SimpleAction action, Variant? param)
        {
            var xid = param.get_uint32();
            Wnck.Screen.get_default ();
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
            Wnck.Screen.get_default ();
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
            Wnck.Screen.get_default ();
            var widget = this.get_parent() as MenuWidget;
            unowned Wnck.Window window = Wnck.Window.@get ((ulong)widget.window_id);
            if (window == null)
            {
                Wnck.Screen.get_default().force_update();
                window = Wnck.Window.@get ((ulong)widget.window_id);
            }
            if (window != null && !window.is_skip_tasklist ())
                window.close (Gtk.get_current_event_time ());
        }
    }
}
