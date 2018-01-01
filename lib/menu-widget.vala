/*
 * vala-panel-appmenu
 * Copyright (C) 2015 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    [Flags]
    public enum MenuWidgetCompletionFlags
    {
        NONE,
        APPMENU,
        MENUBAR
    }
    public abstract class MenuWidget: Gtk.Paned
    {
        public uint window_id {get; protected set construct;}
        public MenuWidgetCompletionFlags completed_menus {get; internal set;}
        public bool compact_mode {get; set;}
        private Gtk.Adjustment? scroll_adj = null;
        private Gtk.ScrolledWindow? scroller = null;
        private Gtk.CssProvider provider;
        construct
        {
            scroll_adj = new Gtk.Adjustment(0, 0, 0, 20, 20, 0);
            scroller = new Gtk.ScrolledWindow(scroll_adj, null);
            scroller.set_hexpand(true);
            scroller.set_policy(Gtk.PolicyType.EXTERNAL, Gtk.PolicyType.NEVER);
            scroller.set_shadow_type(Gtk.ShadowType.NONE);
            scroller.scroll_event.connect(on_scroll_event);
            this.pack2(scroller,true,false);

            provider = new Gtk.CssProvider();
            provider.load_from_resource("/org/vala-panel/appmenu/appmenu.css");
            unowned Gtk.StyleContext context = this.get_style_context();
            context.add_provider_for_screen(this.get_screen(), provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        public void set_appmenu(Gtk.MenuBar? appmenu)
        {
            if (this.get_child1() is Gtk.Widget)
                this.get_child1().destroy();
            if (appmenu != null)
            {
                this.pack1(appmenu,false,false);
                unowned Gtk.StyleContext context = appmenu.get_style_context();
#if BOLD
                context.add_class("-vala-panel-appmenu-bold");
#endif
                context.add_class("-vala-panel-appmenu-private");
                completed_menus |= MenuWidgetCompletionFlags.APPMENU;
            }
            else
                completed_menus &= ~MenuWidgetCompletionFlags.APPMENU;
        }
        public void set_menubar(Gtk.MenuBar? menubar)
        {
            if (scroller.get_child() is Gtk.Widget)
                scroller.get_child().destroy();
            if (menubar != null)
            {
                scroller.add(menubar);
                unowned Gtk.StyleContext context = menubar.get_style_context();
                context.add_class("-vala-panel-appmenu-private");
                completed_menus |= MenuWidgetCompletionFlags.MENUBAR;
                scroller.show();
            }
            else
            {
                scroller.hide();
                completed_menus &= ~MenuWidgetCompletionFlags.MENUBAR;
            }
        }
        protected bool on_scroll_event(Gdk.EventScroll event)
        {
            print("scroll event: ");
            var val = scroll_adj.get_value();
            var incr = scroll_adj.get_step_increment();
            if (event.direction == Gdk.ScrollDirection.UP)
            {
                print ("up\n");
                scroll_adj.set_value(val - incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.DOWN)
            {
                print ("down\n");
                scroll_adj.set_value(val + incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.LEFT)
            {
                scroll_adj.set_value(val - incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.RIGHT)
            {
                scroll_adj.set_value(val + incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.SMOOTH)
            {
                print("smooth %f %f\n", event.delta_x, event.delta_y);
                scroll_adj.set_value(val + incr * (event.delta_y + event.delta_x));
                return true;
            }
            return false;
        }
    }
    public class MenuWidgetAny : MenuWidget
    {
        public MenuWidgetAny(Bamf.Application app)
        {
            var appmenu = new BamfAppmenu(app);
            this.set_appmenu(appmenu);
            this.set_menubar(null);
            this.show_all();
        }
    }
}
