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
using Appmenu;

namespace Appmenu
{
    internal class MenuWidgetDesktop: MenuWidgetMenumodel
    {
        private GLib.Menu files_menu;
        private const GLib.ActionEntry[] menu_entries =
        {
            {"launch-id", activate_menu_id, "s", null, null},
            {"launch-uri", activate_menu_uri, "s", null, null},
            {"launch-command", activate_menu_command, "s", null, null},
            {"launch-type", activate_menu_launch_type, "s", null, null},
            {"desktop-settings", activate_desktop, null, null, null},
            {"control-center", activate_control, null, null, null},
            {"populate-files", null, "b", "false", state_populate_files}
        };
        public MenuWidgetDesktop(Bamf.Application? app, Bamf.Window? window)
        {
            base(null,window);
            var group = new SimpleActionGroup();
            group.add_action_entries(menu_entries,this);
            this.insert_action_group("menu",group);
            var builder = new Builder.from_resource("/org/vala-panel/appmenu/desktop-menus.ui");
            if ((completed_menus & MenuWidgetCompletionFlags.APPMENU) == 0)
            {
                unowned GLib.Menu gmenu = builder.get_object("appmenu-desktop") as GLib.Menu;
                var menu = new GLib.Menu();
                string? name = null;
                if (app != null)
                {
                    var desktop_file = app.get_desktop_file();
                    if (desktop_file != null)
                        name = new DesktopAppInfo.from_filename(desktop_file).get_name();
                }
                if (name == null)
                    name = _("_Desktop");
                menu.append_submenu(name,gmenu);
                menu.freeze();
                var appmenu = new Gtk.MenuBar.from_model(menu);
                this.add(appmenu);
                completed_menus |= MenuWidgetCompletionFlags.APPMENU;
            }
            if ((completed_menus & MenuWidgetCompletionFlags.MENUBAR) == 0)
            {
                files_menu = builder.get_object("files") as GLib.Menu;
                unowned GLib.Menu gmenu = builder.get_object("menubar") as GLib.Menu;
                var menubar = new Gtk.MenuBar.from_model(gmenu);
                completed_menus |= MenuWidgetCompletionFlags.MENUBAR;
                this.add(menubar);
            }
            this.show_all();
        }
        internal void activate_menu_id(SimpleAction action, Variant? param)
        {
            MenuMaker.activate_menu_launch_id(action,param,this);
        }
        internal void activate_menu_uri(SimpleAction action, Variant? param)
        {
            MenuMaker.activate_menu_launch_uri(action,param,this);
        }
        internal void activate_menu_command(SimpleAction action, Variant? param)
        {
            MenuMaker.activate_menu_launch_command(action,param,this);
        }
        public void activate_menu_launch_type(SimpleAction action, Variant? param)
        {
            unowned string type = param.get_string();
            var info = GLib.AppInfo.get_default_for_type(type,false) as DesktopAppInfo;
			MenuMaker.launch(info,null,this);
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
				MenuMaker.launch(info,null,this);
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
				MenuMaker.launch(info,null,this);
            } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
        }
        public void state_populate_files(SimpleAction action, Variant? param)
        {
            var desktop_dir = Environment.get_user_special_dir(UserDirectory.DESKTOP);
            files_menu.remove_all();
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
                        files_menu.append(info.get_display_name(),"menu.launch-uri('%s')".printf(file.get_uri()));
                }
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
            }
            if (files_menu.get_n_items() == 0)
                files_menu.append(_("No files"),"ls.this-should-not-be-reached");
            action.set_state(new Variant.boolean(true));
        }
        ~MenuWidgetDesktop()
        {
            files_menu = null;
        }
    }
}

