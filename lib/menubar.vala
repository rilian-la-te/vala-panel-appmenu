using GLib;
using Gtk;

namespace Appmenu
{
    public class ActiveMenu : Gtk.EventBox
    {
        private Registrar registrar;
        private void on_bus_aquired (DBusConnection conn)
        {
            try {
                registrar = new Registrar();
                conn.register_object (REG_OBJECT, registrar);
                connect_signals();
            } catch (IOError e) {
                stderr.printf ("Could not register service\n");
            }
        }
        private void on_name_lost(DBusConnection conn,string name)
        {
            print("Name is lost\n");
            remove_menu();
        }
        private void remove_menu()
        {
            if (this.get_child() != null)
                this.remove(this.get_child());
        }
        private void connect_signals()
        {
            this.add(registrar.active_menu);
            this.registrar.notify["active-menu"].connect(()=>{
                this.remove_menu();
                if (registrar.active_menu != null)
                    this.add(registrar.active_menu);
            });
        }
        public ActiveMenu()
        {
            Gtk.Settings.get_default().gtk_shell_shows_app_menu = false;
            Gtk.Settings.get_default().gtk_shell_shows_menubar = false;
            Bus.own_name(BusType.SESSION, REG_IFACE, BusNameOwnerFlags.NONE,
                        on_bus_aquired,
                        ()=>{},
                        on_name_lost);
        }
    }
}
