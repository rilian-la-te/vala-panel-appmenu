using GLib;
using Gtk;
using DBusMenu;

namespace Appmenu
{
    public class MenuWidgetDbusmenu: MenuWidget
    {
        public string object_name {get; private set construct;}
        public ObjectPath object_path {get; private set construct;}
        private DBusMenu.GtkClient client;
        private Gtk.MenuBar menubar;
        public MenuWidgetDbusmenu(uint window_id, string name, ObjectPath path)
        {
            this.window_id = window_id;
            this.object_name = name;
            this.object_path = path;
            client = new DBusMenu.GtkClient(name,(string)path);
            menubar = new Gtk.MenuBar();
            client.attach_to_menu(menubar);
            this.add(menubar);
            this.show_all();
        }
    }
}
