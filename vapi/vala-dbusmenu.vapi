namespace DBusMenu {
    [CCode (cheader_filename = "vala-panel.h")]
    public class Client : GLib.Object {
        public Client (string object_name, string object_path);
        public DBusMenu.Item? get_item (int id);
        public DBusMenu.Item? get_root_item ();
        public void request_about_to_show (int id);
        public DBusMenu.Iface iface { get; private set; }
    }
    [CCode (cheader_filename = "vala-panel.h")]
    public class GtkClient : DBusMenu.Client {
        public GtkClient (string object_name, string object_path);
        public void attach_to_menu (Gtk.MenuShell menu);
        public static bool check (string bus_name, string object_path);
        public static Gtk.MenuItem new_item (DBusMenu.Item item);
    }
    [CCode (cheader_filename = "vala-panel.h")]
    public class GtkMainItem : Gtk.CheckMenuItem, DBusMenu.GtkItemIface {
        public GtkMainItem (DBusMenu.Item item);
        protected override void draw_indicator (Cairo.Context cr);
    }
    [CCode (cheader_filename = "vala-panel.h")]
    public class GtkMenuBarItem : Gtk.MenuItem, DBusMenu.GtkItemIface {
        public GtkMenuBarItem (DBusMenu.Item item);
    }
    [CCode (cheader_filename = "vala-panel.h")]
    public class GtkSeparatorItem : Gtk.SeparatorMenuItem, DBusMenu.GtkItemIface {
        public GtkSeparatorItem (DBusMenu.Item item);
    }
    [CCode (cheader_filename = "vala-panel.h")]
    public class GtkSliderItem : Gtk.MenuItem, DBusMenu.GtkItemIface {
        public GtkSliderItem (DBusMenu.Item item);
        protected override bool button_press_event (Gdk.EventButton event);
        protected override bool button_release_event (Gdk.EventButton event);
        protected override bool motion_notify_event (Gdk.EventMotion event);
        protected override bool scroll_event (Gdk.EventScroll event);
    }
    [CCode (cheader_filename = "vala-panel.h")]
    public class Item : GLib.Object {
        public Item (int id, DBusMenu.Client iface, GLib.Variant props, GLib.List<int> children_ids);
        public void add_child (int id, int pos);
        public bool get_bool_property (string name);
        public int get_child_position (int child_id);
        public GLib.List<DBusMenu.Item> get_children ();
        public GLib.List<int> get_children_ids ();
        public int get_int_property (string name);
        public string get_string_property (string name);
        public GLib.Variant get_variant_property (string name);
        public void handle_event (string event_id, GLib.Variant? data, uint timestamp);
        public void move_child (int id, int newpos);
        public void remove_child (int id);
        public void request_about_to_show ();
        public void set_variant_property (string name, GLib.Variant? val);
        public int id { get; private set; }
        public signal void child_added (int id, DBusMenu.Item item);
        public signal void child_moved (int oldpos, int newpos, DBusMenu.Item item);
        public signal void child_removed (int id, DBusMenu.Item item);
        public signal void property_changed (string name, GLib.Variant? val);
    }
    [CCode (cheader_filename = "vala-panel.h")]
    public class PropertyStore : GLib.Object {
        public PropertyStore (GLib.Variant? props);
        public GLib.Variant? get_prop (string name);
        public void set_prop (string name, GLib.Variant? val);
    }
    [CCode (cheader_filename = "vala-panel.h")]
    public interface GtkItemIface : GLib.Object {
        public abstract DBusMenu.Item item { get; protected set; }
    }
    [CCode (cheader_filename = "vala-panel.h")]
    [DBus (name = "com.canonical.dbusmenu")]
    public interface Iface : GLib.Object {
        public abstract void about_to_show (int id, out bool need_update) throws GLib.IOError;
        public abstract void about_to_show_group (int[] ids, out int[] updates_needed, out int[] id_errors) throws GLib.IOError;
        public abstract void event (int id, string event_id, GLib.Variant? data, uint timestamp) throws GLib.IOError;
        public abstract void event_group ([DBus (signature = "a(isvu)")] GLib.Variant events, out int[] id_errors) throws GLib.IOError;
        public abstract void get_group_properties (int[] ids, string[] property_names, [DBus (signature = "a(ia{sv})")] out GLib.Variant properties) throws GLib.IOError;
        public abstract void get_layout (int parent_id, int recursion_depth, string[] property_names, out uint revision, [DBus (signature = "(ia{sv}av)")] out GLib.Variant layout) throws GLib.IOError;
        public abstract void get_property (int id, string name, out GLib.Variant value) throws GLib.IOError;
        public abstract string[] icon_theme_path { owned get; }
        [DBus (use_string_marshalling = true)]
        public abstract DBusMenu.Status status { get; }
        public abstract string text_direction { owned get; }
        public abstract uint version { get; }
        public signal void item_activation_requested (int id, uint timestamp);
        public signal void items_properties_updated ([DBus (signature = "a(ia{sv})")] GLib.Variant updated_props, [DBus (signature = "a(ias)")] GLib.Variant removed_props);
        public signal void layout_updated (uint revision, int parent);
        public signal void x_valapanel_item_value_changed (int id, uint timestamp);
    }
    [CCode (cheader_filename = "vala-panel.h")]
    [DBus (use_string_marshalling = true)]
    public enum Status {
        [DBus (value = "normal")]
        NORMAL,
        [DBus (value = "notice")]
        NOTICE
    }
}
