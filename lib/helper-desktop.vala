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
            unowned GLib.Menu gmenu = builder.get_object("appmenu-desktop") as GLib.Menu;
            var menu = new GLib.Menu();
            string? name = GLib.dgettext(Config.GETTEXT_PACKAGE,"_Desktop");
            menu.append_submenu(name,gmenu);
            menu.freeze();
            widget.set_appmenu(menu);
            GLib.Menu append_menu = new GLib.Menu();
            widget.set_menubar(append_menu);
            files_menu = builder.get_object("files") as GLib.Menu;
            documents_menu = builder.get_object("docs") as GLib.Menu;
            music_menu = builder.get_object("music") as GLib.Menu;
            pictures_menu = builder.get_object("picts") as GLib.Menu;
            videos_menu = builder.get_object("video") as GLib.Menu;
            gmenu = builder.get_object("menubar") as GLib.Menu;
            append_menu.append_section(null,gmenu);
            append_menu.freeze();
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
                    case "Budgie:GNOME":
                        info = new DesktopAppInfo("budgie-desktop-settings.desktop");
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
                    case "Budgie:GNOME":
                        string control_center = "gnome-control-center";
                        if (Environment.find_program_in_path("budgie-control-center") != null) {
                            control_center = "budgie-control-center";
                        }
                        info = AppInfo.create_from_commandline(control_center,null,
                        AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION) as DesktopAppInfo;
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
