using GLib;
using Gtk;

namespace Appmenu
{
    internal class MenuWidgetMenumodel : MenuWidget
    {
        private GLib.ActionGroup? appmenu_actions = null;
        private GLib.ActionGroup? menubar_actions = null;
        private GLib.ActionGroup? unity_actions = null;
        public MenuWidgetMenumodel(Bamf.Application? app,Bamf.Window window)
        {
            this.window_id = window.get_xid();
            var gtk_unique_bus_name = window.get_utf8_prop("_GTK_UNIQUE_BUS_NAME");
            var app_menu_path = window.get_utf8_prop("_GTK_APP_MENU_OBJECT_PATH");
            var menubar_path = window.get_utf8_prop("_GTK_MENUBAR_OBJECT_PATH");
            var application_path = window.get_utf8_prop("_GTK_APPLICATION_OBJECT_PATH");
            var window_path = window.get_utf8_prop("_GTK_WINDOW_OBJECT_PATH");
            var unity_path = window.get_utf8_prop("_UNITY_OBJECT_PATH");
            DBusConnection? dbusconn = null;
            try {
                dbusconn = Bus.get_sync(BusType.SESSION);
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
            }
            if (application_path != null)
                appmenu_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,application_path);
            if (unity_path != null)
                unity_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,unity_path);
            if (window_path != null)
                menubar_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,window_path);
            string? name = null;
            if (app != null)
            {
                var desktop_file = app.get_desktop_file();
                if (desktop_file != null)
                    name = new DesktopAppInfo.from_filename(desktop_file).get_name();
            }
            if (name == null)
                name = window.get_name();
            if (name == null)
                name = _("_Application");
            if (app_menu_path != null)
            {
                var menu = new GLib.Menu();
                menu.append_submenu(name,DBusMenuModel.get(dbusconn,gtk_unique_bus_name,app_menu_path));
                this.appmenu = new Gtk.MenuBar.from_model(menu);
            }
            else if (app != null)
                this.appmenu = new BamfAppmenu(app);
            if (menubar_path != null)
                menubar = new Gtk.MenuBar.from_model(DBusMenuModel.get(dbusconn,gtk_unique_bus_name,menubar_path));
            if (appmenu_actions != null)
                this.insert_action_group("app",appmenu_actions);
            if (menubar_actions != null)
                this.insert_action_group("win",menubar_actions);
            if (unity_actions != null)
                this.insert_action_group("unity",unity_actions);
            if (appmenu != null)
                this.add(appmenu);
            if (menubar != null)
                this.add(menubar);
            this.show_all();
        }
    }
}
