using GLib;

namespace Appmenu
{
    public const string DBUS_NAME = "com.canonical.AppMenu.Registrar";
    public const string REG_IFACE = "com.canonical.AppMenu.Registrar";
    public const string REG_OBJECT = "/com/canonical/AppMenu/Registrar";
    public const string DEBUG_IFACE = "com.canonical.AppMenu.Renderer";
    public const string DEBUG_OBJECT = "/com/canonical/AppMenu/Renderer";

    [DBus (name = "com.canonical.AppMenu.Registrar")]
    public interface Registrar
    {
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        public void register_window(uint window_id, ObjectPath menu_object_path);
        public void unregister_window(uint window_id);
        public void get_menu_for_window(uint window, out string service, out ObjectPath path);
        public void get_menus([DBus (signature="a(uso)")] out Variant menus);
    }
    [DBus (name = "com.canonical.AppMenu.Renderer")]
    public interface Renderer
    {
        public void get_current_menu(out string name, out ObjectPath path);
        public void activate_menu_item(int[] menu_item_path);
        public void dump_current_menu(out string json);
        public void dump_menu(uint window_id, out string json);
    }

}
