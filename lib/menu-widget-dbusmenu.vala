using GLib;
using Gtk;
using DBusMenu;

namespace Appmenu
{
    public class MenuWidgetDbusmenu: Gtk.Box
    {
        public string name {get; private set construct;}
        public ObjectPath path {get; private set construct;}
        public uint window_id {get; private set construct;}
        private DBusMenu.GtkClient client;
        private Gtk.MenuBar menubar;
        public MenuWidgetDbusmenu(uint window_id, string name, ObjectPath path)
        {
            this.window_id = window_id;
            this.name = name;
            this.path = path;
            client = new DBusMenu.GtkClient(name,(string)path);
            menubar = new Gtk.MenuBar();
            client.attach_to_menu(menubar);
            this.add(menubar);
            menubar.show_all();
        }
    }
}
