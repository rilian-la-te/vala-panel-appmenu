using GLib;

namespace Appmenu
{
    public const string DBUS_NAME = "com.canonical.AppMenu.Registrar";
    public const string REG_IFACE = "com.canonical.AppMenu.Registrar";
    public const string REG_OBJECT = "/com/canonical/AppMenu/Registrar";

    public abstract class MenuWidget: Gtk.Box
    {
        public uint window_id {get; protected set construct;}
        public Gtk.MenuBar menubar {get; protected set construct;}
        public Gtk.MenuBar appmenu {get; internal set construct;}
    }
    [DBus (name = "com.canonical.AppMenu.Registrar")]
    public class Registrar : Object
    {
        private HashTable<uint,MenuWidget> menus;
        private GenericSet<uint> desktop_menus;
        private Bamf.Matcher matcher;
        private ulong active_handler;
        private ulong open_handler;
        private ulong close_handler;
        public signal void window_registered(uint window_id, string service, ObjectPath path);
        public signal void window_unregistered(uint window_id);
        [DBus (visible = false)]
        public MenuWidget? active_menu {get; private set;}
        public Registrar()
        {
            Object();
        }
        construct
        {
            menus = new HashTable<uint,MenuWidget>(direct_hash,direct_equal);
            desktop_menus = new GenericSet<uint>(direct_hash,direct_equal);
            matcher = Bamf.Matcher.get_default();
            active_handler = matcher.active_window_changed.connect(on_active_window_changed);
            open_handler = matcher.view_opened.connect(on_window_opened);
            close_handler = matcher.view_closed.connect(on_window_closed);
            on_active_window_changed(matcher.get_active_window(),null);
            foreach (var window in matcher.get_windows())
                on_window_opened(window);
            foreach (var app in matcher.get_running_applications())
                on_window_opened(app);
        }
        ~Registrar()
        {
            matcher.disconnect(active_handler);
            matcher.disconnect(open_handler);
            matcher.disconnect(close_handler);
        }
        public void register_window(uint window_id, ObjectPath menu_object_path, BusName sender)
        {
            Bamf.Application app = matcher.get_application_for_xid(window_id);
            MenuWidget menu = new MenuWidgetDbusmenu(window_id,sender,menu_object_path,app);
            if (menus.contains(window_id))
                unregister_window(window_id);
            menus.insert(window_id,menu);
            if (window_id == matcher.get_active_window().get_xid())
                this.active_menu = menu;
            window_registered(window_id,sender,menu_object_path);
        }
        public void unregister_window(uint window_id)
        {
            var menu = menus.lookup(window_id);
            if (menu == null)
                return;
            if (this.active_menu == menu)
                this.active_menu = show_dummy_menu();
            desktop_menus.remove(window_id);
            menus.remove(window_id);
            menu.destroy();
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
        private MenuWidget? show_dummy_menu()
        {
            MenuWidget? menu = null;
            if (desktop_menus.length > 0)
            {
                desktop_menus.foreach((k)=>{
                    menu = menus.lookup(k);
                    if (menu != null)
                        return;
                });
            }
            return menu;
        }
        private void on_window_opened(Bamf.View view)
        {
            if(view is Bamf.Window)
            {
                unowned Bamf.Window window = view as Bamf.Window;
                if (window.get_type() == Bamf.WindowType.DESKTOP)
                {
                    menus.insert(window.get_xid(),lookup_menu(window));
                    desktop_menus.add(window.get_xid());
                }
                /* Chromium hack, because BAMF does not always send a correct Application
                * DBusMenu registration always happened BEFORE a BAMF register application.
                */
                var menu = menus.lookup(window.get_xid());
                if (menu != null && menu.appmenu == null)
                {
                    var app = matcher.get_application_for_window(window);
                    if (app != null)
                    {
                        menu.appmenu = new BamfAppmenu(app);
                        menu.add(menu.appmenu);
                        menu.reorder_child(menu.appmenu,0);
                        menu.appmenu.show();
                    }
                }
            }
            /* Appmenu hack, because BAMF does not always send a correct Application
             * DBusMenu registration always happened BEFORE a BAMF register application.
             * For Chromium we need different hack - it is not working by some reason.
             */
            if(view is Bamf.Application)
            {
                unowned Bamf.Application app = view as Bamf.Application;
                foreach (var window in app.get_windows())
                {
                    var menu = menus.lookup(window.get_xid());
                    if (menu != null && menu.appmenu == null)
                    {
                        menu.appmenu = new BamfAppmenu(app);
                        menu.add(menu.appmenu);
                        menu.reorder_child(menu.appmenu,0);
                        menu.appmenu.show();
                    }
                }
            }
        }
        private void on_window_closed(Bamf.View view)
        {
            if (view is Bamf.Window)
                unregister_window((view as Bamf.Window).get_xid());
        }
        private void on_active_window_changed(Bamf.Window? prev, Bamf.Window? next)
        {
            if (this.active_menu != null)
                this.active_menu.hide();
            unowned Bamf.Window win = next != null ? next : matcher.get_active_window();
            this.active_menu = lookup_menu(win);
            if (this.active_menu != null)
                this.active_menu.show();
        }
        private MenuWidget lookup_menu(Bamf.Window? window)
        {
            MenuWidget? menu = null;
            uint xid = 0;
            while (window != null && menu == null)
            {
                xid = window.get_xid();
                menu = menus.lookup(xid);
                /* First look to see if we can get these from the
                   GMenuModel access */
                if (menu == null)
                {
                    var uniquename = window.get_utf8_prop ("_GTK_UNIQUE_BUS_NAME");
                    if (uniquename != null)
                    {
                        try {
                            Bamf.Application app = matcher.get_application_for_window(window);
                            menu = new MenuWidgetMenumodel(app,window);
                            menus.insert(xid,menu);
                            return menu;
                        } catch (Error e) {
                            stderr.printf("%s\n",e.message);
                        }
                    }
                }
                if (menu == null)
                {
                    debug("Looking for parent window on XID %u", xid);
                    window = window.get_transient();
                }
            }
            if (menu == null)
                menu = show_dummy_menu();
            return menu;
        }
    }
}
