using GLib;
using Gtk;

namespace Appmenu
{
    public class BamfAppmenu : Gtk.MenuBar
    {
        private Bamf.Application app;
        private GLib.Menu menu;
        private SimpleActionGroup configurator;
        private static const GLib.ActionEntry[] entries =
        {
            {"new-window", activate_new, null, null, null},
            {"activate-action", activate_action, "s", null, null},
            {"active-window", null, "i", "0", state_window},
        };
        public BamfAppmenu(Bamf.Application app)
        {
            this.app = app;
            configurator = new SimpleActionGroup();
            configurator.add_action_entries(entries,this);
            this.insert_action_group("conf",configurator);
            var desktop_file = app.get_desktop_file();
            var info = new DesktopAppInfo.from_filename(desktop_file);
            var submenu = new GLib.Menu();
            var section = new GLib.Menu();
            section.append(_("New window"),"conf.new-window");
            submenu.append_section(null,section);
            section = new GLib.Menu();
            foreach(var action in info.list_actions())
                section.append(info.get_action_name(action),"conf.activate-action('%s')".printf(action));
            menu = new GLib.Menu();
            menu.append_submenu(info.get_name(),submenu);
            this.bind_model(menu,null,true);
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
        private void state_window(GLib.SimpleAction action, Variant? param)
        {
        }
    }
}
