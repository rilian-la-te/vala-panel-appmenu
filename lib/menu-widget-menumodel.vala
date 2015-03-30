using GLib;
using Gtk;

namespace Appmenu
{
    public class MenuWidgetMenumodel : Gtk.Box
    {
        public uint window_id {get; private set construct;}
        private Gtk.MenuBar appmenu;
        private Gtk.MenuBar menubar;
        private Gtk.MenuBar unity;
        private GLib.ActionGroup appmenu_actions;
        private GLib.ActionGroup menubar_actions;
        private GLib.ActionGroup unity_actions;
        public MenuWidgetMenumodel(uint xid)
        {
            this.window_id = xid;
            var disp = Gdk.Display.get_default() as Gdk.X11.Display;
            var conn = ((X.xcb.Display)disp.get_xdisplay()).connection;
        }
    }
}
