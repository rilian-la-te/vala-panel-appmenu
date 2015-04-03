using GLib;

namespace Appmenu
{
    public class AppMenuBar : Gtk.EventBox
    {
        private static DBusMenuRegistrarProxy proxy;
        private HashTable<uint,MenuWidget> menus;
        private GenericSet<uint> desktop_menus;
        private Bamf.Matcher matcher;
        private ulong active_handler;
        private ulong open_handler;
        private ulong close_handler;
        private ulong registered_handler;
        private ulong unregistered_handler;
        static construct
        {
            proxy = new DBusMenuRegistrarProxy();
        }
        public AppMenuBar()
        {
            Object();
        }
        construct
        {
            var gtksettings = this.get_settings();
            gtksettings.gtk_shell_shows_app_menu = false;
            gtksettings.gtk_shell_shows_menubar = false;
            menus = new HashTable<uint,MenuWidget>(direct_hash,direct_equal);
            desktop_menus = new GenericSet<uint>(direct_hash,direct_equal);
            matcher = Bamf.Matcher.get_default();
            registered_handler = proxy.window_registered.connect(register_menu_window);
            unregistered_handler = proxy.window_unregistered.connect(unregister_menu_window);
            active_handler = matcher.active_window_changed.connect(on_active_window_changed);
            open_handler = matcher.view_opened.connect(on_window_opened);
            close_handler = matcher.view_closed.connect(on_window_closed);
            on_active_window_changed(matcher.get_active_window(),null);
            foreach (var window in matcher.get_windows())
                on_window_opened(window);
            foreach (var app in matcher.get_running_applications())
                on_window_opened(app);
        }
        ~AppMenuBar()
        {
            proxy.disconnect(registered_handler);
            proxy.disconnect(unregistered_handler);
            matcher.disconnect(active_handler);
            matcher.disconnect(open_handler);
            matcher.disconnect(close_handler);
        }
        public void register_menu_window(uint window_id, string sender, ObjectPath menu_object_path)
        {
            Bamf.Application app = matcher.get_application_for_xid(window_id);
            MenuWidget menu = new MenuWidgetDbusmenu(window_id,sender,menu_object_path,app);
            if (menus.contains(window_id))
                unregister_menu_window(window_id);
            menus.insert(window_id,menu);
            if (window_id == matcher.get_active_window().get_xid())
                replace_menu(menu);
        }
        public void unregister_menu_window(uint window_id)
        {
            var menu = menus.lookup(window_id);
            if (menu == null)
                return;
            if (this.get_child() == menu)
            {
                this.remove(menu);
                this.child = show_dummy_menu();
            }
            desktop_menus.remove(window_id);
            menus.remove(window_id);
            menu.destroy();
        }
        private void replace_menu(MenuWidget menu)
        {
            if (this.get_child() != null)
                this.remove(this.get_child());
            this.add(menu);
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
                if (window.get_window_type() == Bamf.WindowType.DESKTOP)
                {
                    lookup_menu(window);
                    desktop_menus.add(window.get_xid());
                }
            }
        }
        private void on_window_closed(Bamf.View view)
        {
            if (view is Bamf.Window)
                unregister_menu_window((view as Bamf.Window).get_xid());
        }
        private void on_active_window_changed(Bamf.Window? prev, Bamf.Window? next)
        {
            if (this.get_child() != null)
                this.get_child().hide();
            unowned Bamf.Window win = next != null ? next : matcher.get_active_window();
            replace_menu(lookup_menu(win));
            if (this.get_child() != null)
                this.get_child().show();
        }
        private MenuWidget lookup_menu(Bamf.Window? window)
        {
            MenuWidget? menu = null;
            uint xid = 0;
            while (window != null && menu == null)
            {
                xid = window.get_xid();
                menu = menus.lookup(xid);
                var app = matcher.get_application_for_window(window);
                /* Appmenu hack, because BAMF does not always send a correct Application
                * DBusMenu registration always happened BEFORE a BAMF register application.
                */
                if (menu != null && menu.appmenu == null && app != null)
                {
                    menu.appmenu = new BamfAppmenu(app);
                    menu.add(menu.appmenu);
                    menu.reorder_child(menu.appmenu,0);
                    menu.appmenu.show();
                }
                /* First look to see if we can get these from the
                   GMenuModel access */
                if (menu == null)
                {
                    var uniquename = window.get_utf8_prop ("_GTK_UNIQUE_BUS_NAME");
                    if (uniquename != null)
                    {
                        if (window.get_window_type() == Bamf.WindowType.DESKTOP)
                            menu = new MenuWidgetDesktop(app,window);
                        else
                            menu = new MenuWidgetMenumodel(app,window);
                        menus.insert(xid,menu);
                        return menu;
                    }
                }
                if (menu == null)
                {
                    debug("Looking for parent window on XID %u", xid);
                    if (window.get_transient() == null && app != null)
                    {
                        menu = new MenuWidgetAny(app);
                        menus.insert(window.get_xid(),menu);
                    }
                    window = window.get_transient();
                }
            }
            if (menu == null)
                menu = show_dummy_menu();
            return menu;
        }
    }
}
