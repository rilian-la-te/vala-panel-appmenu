using GLib;

namespace Appmenu
{
    public const string DBUS_NAME = "com.canonical.AppMenu.Registrar";
    public const string REG_IFACE = "com.canonical.AppMenu.Registrar";
    public const string REG_OBJECT = "/com/canonical/AppMenu/Registrar";

    [DBus (name = "com.canonical.AppMenu.Registrar")]
    public interface OuterRegistrar : Object
    {
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        public abstract void register_window(uint window, ObjectPath menu_object_path) throws Error;
        public abstract void unregister_window(uint window) throws Error;
        public abstract void get_menu_for_window(uint window, out string service, out ObjectPath path) throws Error;
        public abstract void get_menus([DBus (signature="a(uso)")] out Variant menus) throws Error;
    }

    internal struct DBusAddress
    {
        string name;
        ObjectPath path;
    }

    [DBus (name = "com.canonical.AppMenu.Registrar")]
    public class InnerRegistrar : Object
    {
        private HashTable<uint,DBusAddress?> menus;
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        public InnerRegistrar()
        {
            Object();
        }
        construct
        {
            menus = new HashTable<uint,DBusAddress?>(direct_hash,direct_equal);
        }
        public void register_window(uint window_id, ObjectPath menu_object_path, BusName sender)
        {
            DBusAddress addr = DBusAddress();
            addr.name = (string)sender;
            addr.path = menu_object_path;
            menus.insert(window_id,addr);
            window_registered(window_id,sender,menu_object_path);
        }
        public void unregister_window(uint window_id)
        {
            if (!(window_id in menus))
                return;
            menus.remove(window_id);
            window_unregistered(window_id);
        }
        public void get_menu_for_window(uint window, out string service, out ObjectPath path)
        {
            unowned DBusAddress? menu = menus.lookup(window);
            if (menu != null)
            {
                service = menu.name;
                path = menu.path;
            }
            else
            {
                service = "";
                path = new ObjectPath("/");
            }
            if (menu != null && !DBusMenu.GtkClient.check(service,path))
            {
                service = "";
                path = new ObjectPath("/");
                unregister_window(window);
            }
        }
        public void get_menus([DBus (signature="a(uso)")] out Variant menus)
        {
            VariantBuilder builder = new VariantBuilder(new VariantType("a(uso)"));
            this.menus.foreach((k,v)=>{
                string service;
                ObjectPath path;
                get_menu_for_window(k,out service,out path);
                builder.add("(uso)",k,service,path);
            });
            menus = builder.end();
        }
    }
    public class DBusMenuRegistrarProxy: Object
    {
        public bool have_registrar {get; private set;}
        private InnerRegistrar inner_registrar;
        private OuterRegistrar outer_registrar;
        private bool is_inner_registrar;
        private uint owned_name;
        private uint watched_name;
        public DBusMenuRegistrarProxy()
        {
            Object();
        }
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        private void on_bus_aquired(DBusConnection conn)
        {
            try {
                inner_registrar = new InnerRegistrar();
                outer_registrar = null;
                conn.register_object (REG_OBJECT, inner_registrar);
                inner_registrar.window_registered.connect((w,s,p)=>{this.window_registered(w,s,p);});
                inner_registrar.window_unregistered.connect((w)=>{this.window_unregistered(w);});
            } catch (IOError e) {
                stderr.printf ("Could not register service. Waiting for external registrar\n");
            }
        }
        private void create_inner_registrar()
        {
            owned_name = Bus.own_name (BusType.SESSION, REG_IFACE, BusNameOwnerFlags.NONE,
                on_bus_aquired,
                () => {
                    have_registrar = true;
                    is_inner_registrar = true;
                },
                () => {
                    is_inner_registrar = false;
                    create_outer_registrar();
                });
        }
        private void create_outer_registrar()
        {
            try{
                outer_registrar = Bus.get_proxy_sync(BusType.SESSION,REG_IFACE,REG_OBJECT);
                watched_name = Bus.watch_name(BusType.SESSION,REG_IFACE,GLib.BusNameWatcherFlags.NONE,
                                                        () => {
                                                            inner_registrar = null;
                                                            is_inner_registrar = false;
                                                            have_registrar = true;
                                                            },
                                                        () => {
                                                            Bus.unwatch_name(watched_name);
                                                            is_inner_registrar = true;
                                                            create_inner_registrar();
                                                            }
                                                        );
                outer_registrar.window_registered.connect((w,s,p)=>{this.window_registered(w,s,p);});
                outer_registrar.window_unregistered.connect((w)=>{this.window_unregistered(w);});
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
                return;
            }
        }
        construct
        {
            is_inner_registrar = true;
            have_registrar = false;
            create_inner_registrar();
        }
        public void get_menu_for_window(uint window, out string name, out ObjectPath path)
        {
            name = "";
            path = new ObjectPath("/");
            if (!have_registrar)
                return;
            if (is_inner_registrar)
                inner_registrar.get_menu_for_window(window,out name, out path);
            else
                try{
                    outer_registrar.get_menu_for_window(window,out name, out path);
                } catch (Error e) {stderr.printf("%s\n",e.message);}
        }
        ~DBusMenuRegistrarProxy()
        {
            if (is_inner_registrar)
                Bus.unown_name(owned_name);
            else
                Bus.unwatch_name(watched_name);
        }
    }
}
