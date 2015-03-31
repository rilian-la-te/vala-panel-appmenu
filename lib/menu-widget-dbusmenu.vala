using GLib;
using Gtk;
using DBusMenu;

namespace Appmenu
{
    internal class MenuWidgetDbusmenu: MenuWidget
    {
        public string object_name {get; private set construct;}
        public ObjectPath object_path {get; private set construct;}
        private DBusMenu.GtkClient client;
        public MenuWidgetDbusmenu(uint window_id, string name, ObjectPath path, Bamf.Application? app)
        {
            this.window_id = window_id;
            this.object_name = name;
            this.object_path = path;
            if (app != null)
                this.appmenu = new BamfAppmenu(app);
            client = new DBusMenu.GtkClient(name,(string)path);
            menubar = new Gtk.MenuBar();
            client.attach_to_menu(menubar);
            if (this.appmenu != null)
                this.add(appmenu);
            this.add(menubar);
            menubar.show();
            this.show();
        }
    }
}
