using GLib;
using Wnck;

namespace Appmenu
{
    public const string DBUS_NAME = "com.canonical.AppMenu.Registrar";
    public const string REG_IFACE = "com.canonical.AppMenu.Registrar";
    public const string REG_OBJECT = "/com/canonical/AppMenu/Registrar";

    public abstract class MenuWidget: Gtk.Box
    {
        public uint window_id {get; protected set construct;}
    }
    [DBus (name = "com.canonical.AppMenu.Registrar")]
    public class Registrar : Object
    {
        private HashTable<uint,MenuWidget> menus;
        private Wnck.Screen screen;
        private ulong active_handler;
        private ulong open_handler;
        private ulong close_handler;
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        [DBus (visible = false)]
        public Gtk.Widget? active_menu {get; private set;}
        public Registrar()
        {
            Object();
        }
        construct
        {
            menus = new HashTable<uint,MenuWidget>(direct_hash,direct_equal);
            screen = Wnck.Screen.get_default();
            active_handler = screen.active_window_changed.connect(on_active_window_changed);
            open_handler = screen.window_opened.connect(on_window_opened);
            close_handler = screen.window_closed.connect(on_window_closed);
        }
        ~Registrar()
        {
            screen.disconnect(active_handler);
            screen.disconnect(open_handler);
            screen.disconnect(close_handler);
        }
        public void register_window(uint window_id, ObjectPath menu_object_path, BusName sender)
        {
            MenuWidget menu = new MenuWidgetDbusmenu(window_id,sender,menu_object_path);
            if (menus.contains(window_id))
                unregister_window(window_id);
            menus.insert(window_id,menu);
            window_registered(window_id,sender,menu_object_path);
        }
        public void unregister_window(uint window_id)
        {
            var menu = menus.lookup(window_id);
            if (menu == null)
                return;
            if (this.active_menu==menu)
                this.active_menu = show_dummy_menu();
            menus.remove(window_id);
            window_unregistered(window_id);
        }
        public void get_menu_for_window(uint window, out string service, out ObjectPath path) throws DBusError
        {
            var menu = menus.lookup(window);
            if (menu == null)
                throw new DBusError.INVALID_ARGS("Window not found!");
            if (menu != null && menu is MenuWidgetDbusmenu)
            {
                var menu_dbus = menu as MenuWidgetDbusmenu;
                service = menu_dbus.object_name;
                path = menu_dbus.object_path;
            }
            else
            {
                service = "";
                path = new ObjectPath("/");
            }
        }
        public void get_menus([DBus (signature="a(uso)")] out Variant menus)
        {
            VariantBuilder builder = new VariantBuilder(new VariantType("a(uso)"));
            this.menus.foreach((k,v)=>{
                try {
                    string service;
                    ObjectPath path;
                    get_menu_for_window(k,out service,out path);
                    builder.add("(uso)",k,service,path);
                } catch (Error e) {
                    stderr.printf("%s\n",e.message);
                }
            });
            menus = builder.end();
        }
        private Gtk.MenuBar? show_dummy_menu()
        {
            return null;
        }
        private void on_window_opened(Window window)
        {

        }
        private void on_window_closed(.Window window)
        {

        }
        private void on_active_window_changed(Window prev)
        {
            this.active_menu = menus.lookup((uint)screen.get_active_window().get_xid());
        }
    }
}
