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
    internal abstract class Helper: Object
    {

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
            var data = new SpawnData();
            if (info != null)
            {
                try {
                    info.launch_uris_as_manager(new List<string>(),
                                                widget.get_display().get_app_launch_context(),
                                                SpawnFlags.SEARCH_PATH,
                                                data.child_spawn_func,(a,b)=>{});
                } catch (Error e) {
                    stderr.printf("%s\n",e.message);
                }
            }
            else if (connection != null)
            {
                //FIXME: Now using only first part, not parameters
                try {
                    string str = "/proc/%u/cmdline".printf(dbus.get_connection_unix_process_id(this.connection));
                    string exec = Launcher.posix_get_cmdline_string(str);
                    var appinfo  = AppInfo.create_from_commandline(exec,null,0) as DesktopAppInfo;
                    appinfo.launch_uris_as_manager(new List<string>(),
                                                widget.get_display().get_app_launch_context(),
                                                SpawnFlags.SEARCH_PATH,
                                                data.child_spawn_func,(a,b)=>{});
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
            var data = new SpawnData();
            try {
                var keyfile = new KeyFile();
                keyfile.load_from_file(info.get_filename(),KeyFileFlags.NONE);
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
    internal class MenuModelHelper: Helper
    {
        private Helper dbus_helper = null;
        public MenuModelHelper(MenuWidget w,
                               string? gtk_unique_bus_name,
                               string? app_menu_path,
                               string? menubar_path,
                               string? application_path,
                               string? window_path,
                               string? unity_path,
                               string? title,
                               DesktopAppInfo? info)
        {
            GLib.ActionGroup? appmenu_actions = null;
            GLib.ActionGroup? menubar_actions = null;
            GLib.ActionGroup? unity_actions = null;
            DBusConnection? dbusconn = null;
            try {
                dbusconn = Bus.get_sync(BusType.SESSION);
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
                return;
            }
            if (application_path != null)
                appmenu_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,application_path);
            if (unity_path != null)
                unity_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,unity_path);
            if (window_path != null)
                menubar_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,window_path);
            GLib.MenuModel? appmenu = null;
            if (app_menu_path != null)
            {
                appmenu = new GLib.Menu();
                (appmenu as GLib.Menu).append_submenu(title,DBusMenuModel.get(dbusconn,gtk_unique_bus_name,app_menu_path));
                w.set_appmenu(appmenu);
            }
            else
                dbus_helper = new DBusAppMenu(w, title, gtk_unique_bus_name, info);
            if (menubar_path != null)
            {
                var menubar = DBusMenuModel.get(dbusconn,gtk_unique_bus_name,menubar_path);
                w.set_menubar(menubar);
            }
            else
                w.set_menubar(null);
            if (appmenu_actions != null)
                w.insert_action_group("app",appmenu_actions);
            if (menubar_actions != null)
                w.insert_action_group("win",menubar_actions);
            if (unity_actions != null)
                w.insert_action_group("unity",unity_actions);
        }
    }
    internal class DBusMenuHelper: Helper
    {
        private DBusMenu.Importer importer = null;
        private Helper dbus_helper = null;
        public DBusMenuHelper(MenuWidget w, string name, ObjectPath path, string? title, DesktopAppInfo? info)
        {
            dbus_helper = new DBusAppMenu(w, title, name, info);
            importer = new DBusMenu.Importer(name,(string)path);
            bool is_model_set = false;
            ulong connect_handler = 0;
            connect_handler = importer.notify["model"].connect((s, p)=>{
                if(!is_model_set)
                {
                    w.insert_action_group("dbusmenu",importer.action_group);
                    w.set_menubar(importer.model);
                    is_model_set = true;
                    importer.disconnect(connect_handler);
                }
            });
        }
    }
    internal class DesktopHelper: Helper
    {
        private GLib.Menu files_menu;
        private GLib.Menu documents_menu;
        private GLib.Menu pictures_menu;
        private GLib.Menu music_menu;
        private GLib.Menu videos_menu;
        private unowned MenuWidget widget;
        private const GLib.ActionEntry[] menu_entries =
        {
            {"launch-id", activate_menu_id, "s", null, null},
            {"launch-uri", activate_menu_uri, "s", null, null},
            {"launch-command", activate_menu_command, "s", null, null},
            {"launch-type", activate_menu_launch_type, "s", null, null},
            {"desktop-settings", activate_desktop, null, null, null},
            {"control-center", activate_control, null, null, null},
            {"populate-files", null, "b", "false", state_populate_files},
            {"populate-docs", null, "b", "false", state_populate_docs},
            {"populate-music", null, "b", "false", state_populate_music},
            {"populate-picts", null, "b", "false", state_populate_picts},
            {"populate-video", null, "b", "false", state_populate_video}
        };
        public DesktopHelper(MenuWidget w)
        {
//            if(window != null)
//                base(w,null,window);
            this.widget = w;
            var group = new SimpleActionGroup();
            group.add_action_entries(menu_entries,this);
            w.insert_action_group("menu",group);
            var builder = new Builder.from_resource("/org/vala-panel/appmenu/desktop-menus.ui");
            builder.set_translation_domain(Config.GETTEXT_PACKAGE);
            if ((widget.completed_menus & MenuWidgetCompletionFlags.APPMENU) == 0)
            {
                unowned GLib.Menu gmenu = builder.get_object("appmenu-desktop") as GLib.Menu;
                var menu = new GLib.Menu();
                string? name = GLib.dgettext(Config.GETTEXT_PACKAGE,"_Desktop");
                menu.append_submenu(name,gmenu);
                menu.freeze();
                widget.set_appmenu(menu);
            }
            if ((widget.completed_menus & MenuWidgetCompletionFlags.MENUBAR) == 0)
            {
                GLib.Menu append_menu = new GLib.Menu();
                widget.set_menubar(append_menu);
                files_menu = builder.get_object("files") as GLib.Menu;
                documents_menu = builder.get_object("docs") as GLib.Menu;
                music_menu = builder.get_object("music") as GLib.Menu;
                pictures_menu = builder.get_object("picts") as GLib.Menu;
                videos_menu = builder.get_object("video") as GLib.Menu;
                unowned GLib.Menu gmenu = builder.get_object("menubar") as GLib.Menu;
                append_menu.append_section(null,gmenu);
                append_menu.freeze();
            }
        }
        internal void activate_menu_id(SimpleAction action, Variant? param)
        {
            Launcher.activate_menu_launch_id(action,param,this);
        }
        internal void activate_menu_uri(SimpleAction action, Variant? param)
        {
            Launcher.activate_menu_launch_uri(action,param,this);
        }
        internal void activate_menu_command(SimpleAction action, Variant? param)
        {
            Launcher.activate_menu_launch_command(action,param,this);
        }
        public void activate_menu_launch_type(SimpleAction action, Variant? param)
        {
            unowned string type = param.get_string();
            var info = GLib.AppInfo.get_default_for_type(type,false) as DesktopAppInfo;
            MenuMaker.launch(info,null,widget);
        }
        public void activate_desktop(SimpleAction action, Variant? param)
        {
            try{
                unowned string desktop = Environment.get_variable("XDG_CURRENT_DESKTOP");
                DesktopAppInfo? info = null;
                switch(desktop)
                {
                    case "MATE":
                        info = new DesktopAppInfo("mate-appearance-properties.desktop");
                        break;
                    case "XFCE":
                        info = new DesktopAppInfo("xfce-backdrop-settings.desktop");
                        break;
                    case "LXDE":
                        info = AppInfo.create_from_commandline("pcmanfm --desktop-pref",null,
                        AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION) as DesktopAppInfo;
                        break;
                    default:
                        warning("Unknown desktop environment\n");
                        info = AppInfo.create_from_commandline("gnome-control-center backgrounds",null,
                        AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION) as DesktopAppInfo;
                        break;
                }
                MenuMaker.launch(info,null,widget);
            } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
        }
        public void activate_control(SimpleAction action, Variant? param)
        {
            try{
                unowned string desktop = Environment.get_variable("XDG_CURRENT_DESKTOP");
                DesktopAppInfo? info = null;
                switch(desktop)
                {
                    case "XFCE":
                        info = new DesktopAppInfo("xfce-settings-manager.desktop");
                        break;
                    case "MATE":
                        info = AppInfo.create_from_commandline("mate-control-center",null,
                        AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION) as DesktopAppInfo;
                        break;
                    case "LXDE":
                        info = new DesktopAppInfo("lxappearance.desktop");
                        break;
                    default:
                        warning("Unknown desktop environment\n");
                        info = AppInfo.create_from_commandline("gnome-control-center",null,
                        AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION) as DesktopAppInfo;
                        break;
                }
                MenuMaker.launch(info,null,widget);
            } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
        }
        public void state_populate_files(SimpleAction action, Variant? param)
        {
            populate_menu(files_menu,UserDirectory.DOWNLOAD);
            action.set_state(new Variant.boolean(true));
        }
        public void state_populate_docs(SimpleAction action, Variant? param)
        {
            populate_menu(documents_menu,UserDirectory.DOCUMENTS);
            action.set_state(new Variant.boolean(true));
        }
        public void state_populate_music(SimpleAction action, Variant? param)
        {
            populate_menu(music_menu,UserDirectory.MUSIC);
            action.set_state(new Variant.boolean(true));
        }
        public void state_populate_picts(SimpleAction action, Variant? param)
        {
            populate_menu(pictures_menu,UserDirectory.PICTURES);
            action.set_state(new Variant.boolean(true));
        }
        public void state_populate_video(SimpleAction action, Variant? param)
        {
            populate_menu(videos_menu,UserDirectory.VIDEOS);
            action.set_state(new Variant.boolean(true));
        }
        private void populate_menu(GLib.Menu menu, GLib.UserDirectory udir)
        {
            var desktop_dir = Environment.get_user_special_dir(udir);
            menu.remove_all();
            try
            {
                var dir = Dir.open(desktop_dir);
                for(var name = dir.read_name();name != null; name = dir.read_name())
                {
                    var file = File.new_for_path(desktop_dir + "/" + name);
                    var info = file.query_info(FileAttribute.STANDARD_DISPLAY_NAME+","
                                               +FileAttribute.STANDARD_IS_HIDDEN,
                                               FileQueryInfoFlags.NOFOLLOW_SYMLINKS);
                    if (!info.get_is_hidden())
                    {
                        var item = new GLib.MenuItem(info.get_display_name(),null);
                        item.set_action_and_target("menu.launch-uri", "s", file.get_uri() );
                        menu.append_item(item);
                    }
                }
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
            }
            if (menu.get_n_items() == 0)
                menu.append(GLib.dgettext(Config.GETTEXT_PACKAGE,"No files"),"ls.this-should-not-be-reached");
        }
        ~DesktopHelper()
        {
            files_menu = null;
            documents_menu = null;
            pictures_menu = null;
            music_menu = null;
            videos_menu = null;
        }
    }
}
