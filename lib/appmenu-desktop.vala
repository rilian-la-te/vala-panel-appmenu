using GLib;
using Gtk;
using Appmenu;

namespace Appmenu
{
    internal class MenuWidgetDesktop: MenuWidgetMenumodel
    {
        private GLib.Menu files_menu;
        private static const GLib.ActionEntry[] menu_entries =
        {
            {"launch-id", activate_menu_launch_id, "s", null, null},
            {"launch-uri", activate_menu_launch_uri, "s", null, null},
            {"launch-command", activate_menu_launch_command, "s", null, null},
            {"launch-type", activate_menu_launch_type, "s", null, null},
            {"desktop-settings", activate_desktop, null, null, null},
            {"control-center", activate_control, null, null, null},
            {"populate-files", null, "b", "false", state_populate_files}
        };
        public MenuWidgetDesktop(Bamf.Application? app, Bamf.Window window)
        {
            base(app,window);
            var group = new SimpleActionGroup();
            group.add_action_entries(menu_entries,this);
            this.insert_action_group("menu",group);
            if (appmenu == null || appmenu is BamfAppmenu)
            {
                var builder = new Builder.from_resource("/org/vala-panel/appmenu/desktop-menus.ui");
                var gmenu = builder.get_object("appmenu") as GLib.Menu;
                var menu = new GLib.Menu();
                string? name = null;
                if (app != null)
                {
                    var desktop_file = app.get_desktop_file();
                    if (desktop_file != null)
                        name = new DesktopAppInfo.from_filename(desktop_file).get_name();
                }
                if (name == null)
                    name = _("Desktop");
                menu.append_submenu(name,gmenu);
                appmenu = new Gtk.MenuBar.from_model(menu);
            }
            this.add(appmenu);
            if ((menubar == null || menubar.get_children() == null))
            {
                this.remove(menubar);
                var builder = new Builder.from_resource("/org/vala-panel/appmenu/desktop-menus.ui");
                var gmenu = builder.get_object("menubar") as GLib.Menu;
                files_menu = builder.get_object("files") as GLib.Menu;
                menubar = new Gtk.MenuBar.from_model(gmenu);
            }
            this.add(menubar);
            this.show_all();
        }
        public void activate_menu_launch_id(SimpleAction action, Variant? param)
        {
            var id = param.get_string();
            var info = new DesktopAppInfo(id);
            try{
                info.launch(null,this.get_display().get_app_launch_context());
            } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
        }

        public void activate_menu_launch_command(SimpleAction? action, Variant? param)
        {
            var command = param.get_string();
            try{
                GLib.AppInfo info = AppInfo.create_from_commandline(command,null,
                                    AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION);
                info.launch(null,this.get_display().get_app_launch_context());
            } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
        }

        public void activate_menu_launch_uri(SimpleAction action, Variant? param)
        {
            var uri = param.get_string();
            try{
                GLib.AppInfo.launch_default_for_uri(uri,this.get_display().get_app_launch_context());
            } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
        }
        public void activate_menu_launch_type(SimpleAction action, Variant? param)
        {
            var type = param.get_string();
            try{
                var info = GLib.AppInfo.get_default_for_type(type,false);
                info.launch(null,this.get_display().get_app_launch_context());
            } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
        }
        public void activate_desktop(SimpleAction action, Variant? param)
        {
            try{
                var desktop = Environment.get_variable("XDG_CURRENT_DESKTOP");
                AppInfo? info = null;
                switch(desktop)
                {
                    case "XFCE":
                        info = new DesktopAppInfo("xfce-backdrop-settings.desktop");
                        break;
                    case "LXDE":
                        info = AppInfo.create_from_commandline("pcmanfm --desktop-pref",null,
                        AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION);
                        break;
                    default:
                        warning("Unknown desktop environment\n");
                        info = AppInfo.create_from_commandline("gnome-control-center backgrounds",null,
                        AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION);
                        break;
                }
                info.launch(null,this.get_display().get_app_launch_context());
            } catch (GLib.Error e){stderr.printf("%s\n",e.message);}
        }
        public void activate_control(SimpleAction action, Variant? param)
        {
            try{
                var desktop = Environment.get_variable("XDG_CURRENT_DESKTOP");
                AppInfo? info = null;
                switch(desktop)
                {
                    case "XFCE":
                        info = new DesktopAppInfo("xfce-settings-manager.desktop");
                        break;
                    case "LXDE":
                        info = new DesktopAppInfo("lxappearance.desktop");
                        break;
                    default:
                        warning("Unknown desktop environment\n");
                        info = new DesktopAppInfo("gnome-control-center.desktop");
                        break;
                }
                info.launch(null,this.get_display().get_app_launch_context());
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
                files_menu.append(_("No files"),"ls.this-shold-not-be-reached");
            action.set_state(new Variant.boolean(true));
        }
    }
}
