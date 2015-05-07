using GLib;

namespace Appmenu
{
    [Compact]
    private class SpawnData
    {
        internal Posix.pid_t pid; /* getpgid(getppid()); */
        public SpawnData()
        {
            pid = Posix.getpgid(Posix.getppid());
        }
        public void child_spawn_func()
        {
            Posix.setpgid(0,this.pid);
        }
    }
    public abstract class MenuWidget: Gtk.Box
    {
        public uint window_id {get; protected set construct;}
        public Gtk.MenuBar menubar {get; protected set construct;}
        public Gtk.MenuBar appmenu {get; internal set construct;}
        construct
        {
            var provider = new Gtk.CssProvider();
            File ruri = File.new_for_uri("resource://org/vala-panel/appmenu/appmenu.css");
            try
            {
                provider.load_from_file(ruri);
                this.notify.connect((pspec)=>{
                    if (pspec.name == "appmenu" && appmenu != null)
                    {
                        var context = appmenu.get_style_context();
                        context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
                        context.add_class("-vala-panel-appmenu-private");
                    }
                    if (pspec.name == "menubar" && menubar != null)
                    {
                        var context = menubar.get_style_context();
                        context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
                        context.add_class("-vala-panel-appmenu-private");
                    }
                });
            } catch (GLib.Error e) {}
        }
    }
    public class MenuWidgetAny : MenuWidget
    {
        public MenuWidgetAny(Bamf.Application app)
        {
            appmenu = new BamfAppmenu(app);
            this.add(appmenu);
            this.show_all();
        }
    }
}
