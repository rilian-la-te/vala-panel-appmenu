using GLib;
using Gtk;

namespace Appmenu
{
    public class MenuWidgetMenumodel : MenuWidget
    {
        private Gtk.MenuBar? appmenu = null;
        private Gtk.MenuBar? menubar = null;
        private GLib.ActionGroup? appmenu_actions = null;
        private GLib.ActionGroup? menubar_actions = null;
        private GLib.ActionGroup? unity_actions = null;
        public MenuWidgetMenumodel(uint xid) throws IOError
        {
            this.window_id = xid;
            var window = (Xcb.Window) this.window_id;
            var disp = Gdk.Display.get_default() as Gdk.X11.Display;
            unowned Xcb.Connection conn = ((X.xcb.Display)disp.get_xdisplay()).connection;
            var gtk_unique_atom_cookie = conn.intern_atom(false,(char[])"_GTK_UNIQUE_BUS_NAME".data);
            var app_menu_atom_cookie = conn.intern_atom(false,(char[]) "_GTK_APP_MENU_OBJECT_PATH".data);
            var menubar_atom_cookie = conn.intern_atom(false,(char[]) "_GTK_MENUBAR_OBJECT_PATH".data);
            var application_atom_cookie = conn.intern_atom(false,(char[]) "_GTK_APPLICATION_OBJECT_PATH".data);
            var window_atom_cookie = conn.intern_atom(false,(char[]) "_GTK_WINDOW_OBJECT_PATH".data);
            var unity_atom_cookie = conn.intern_atom(false,(char[]) "_UNITY_OBJECT_PATH".data);
            var gtk_unique_atom = gtk_unique_atom_cookie.reply(conn).atom;
            var app_menu_atom = app_menu_atom_cookie.reply(conn).atom;
            var menubar_atom = menubar_atom_cookie.reply(conn).atom;
            var application_atom = application_atom_cookie.reply(conn).atom;
            var window_atom = window_atom_cookie.reply(conn).atom;
            var unity_atom = unity_atom_cookie.reply(conn).atom;
            var gtk_unique_cookie = window.get_property(conn,false,gtk_unique_atom,Xcb.AtomType.STRING,0,0);
            var app_menu_cookie = window.get_property(conn,false,app_menu_atom,Xcb.AtomType.STRING,0,0);
            var menubar_cookie = window.get_property(conn,false,menubar_atom,Xcb.AtomType.STRING,0,0);
            var application_cookie = window.get_property(conn,false,application_atom,Xcb.AtomType.STRING,0,0);
            var window_cookie = window.get_property(conn,false,window_atom,Xcb.AtomType.STRING,0,0);
            var unity_cookie = window.get_property(conn,false,unity_atom,Xcb.AtomType.STRING,0,0);
            var gtk_unique_bus_name = (string)gtk_unique_cookie.reply(conn).value;
            var app_menu_path = (string)app_menu_cookie.reply(conn).value;
            var menubar_path = (string)menubar_cookie.reply(conn).value;
            var application_path = (string)application_cookie.reply(conn).value;
            var window_path = (string)window_cookie.reply(conn).value;
            var unity_path = (string)unity_cookie.reply(conn).value;
            var dbusconn = Bus.get_sync(BusType.SESSION);
            if (application_path != null)
                appmenu_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,application_path);
            if (unity_path != null)
                unity_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,unity_path);
            if (window_path != null)
                menubar_actions = DBusActionGroup.get(dbusconn,gtk_unique_bus_name,window_path);
            if (app_menu_path != null)
                appmenu = new Gtk.MenuBar.from_model(DBusMenuModel.get(dbusconn,gtk_unique_bus_name,app_menu_path));
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
