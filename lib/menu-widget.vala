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
    public abstract class MenuWidget: Gtk.Box
    {
        public uint window_id {get; protected set construct;}
        public MenuWidgetCompletionFlags completed_menus {get; internal set;}
        public bool compact_mode {get; set;}
        protected Gtk.Adjustment? scroll_adj = null;
        protected Gtk.ScrolledWindow? scroller = null;
        protected Gtk.MenuBar? appmenu = null;
        protected Gtk.MenuBar? menubar = null;
        private Gtk.CssProvider provider;
        construct
        {
            // Scroller setup
            scroll_adj = new Gtk.Adjustment(0, 0, 0, 20, 20, 0);
            scroller = new Gtk.ScrolledWindow(scroll_adj, null);
            scroller.set_hexpand(true);
            scroller.set_policy(Gtk.PolicyType.EXTERNAL, Gtk.PolicyType.NEVER);
            scroller.set_shadow_type(Gtk.ShadowType.NONE);
            scroller.scroll_event.connect(on_scroll_event);

            provider = new Gtk.CssProvider();
            try
            {
                provider.load_from_resource("/org/vala-panel/appmenu/appmenu.css");
                this.notify.connect((pspec)=>{
                    foreach(unowned Gtk.Widget ch in this.get_children())
                    {
                        unowned Gtk.StyleContext context = ch.get_style_context();
                        context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
#if BOLD
                        context.add_class("-vala-panel-appmenu-bold");
#endif
                        context.add_class("-vala-panel-appmenu-private");
                    }
                });
            } catch (GLib.Error e) {}
        }
        protected void init_style()
        {
            unowned Gtk.StyleContext context;
            if (menubar != null) {
                context = menubar.get_style_context();
                context.add_class("-vala-panel-appmenu-private");
            }
            if (appmenu != null) {
                context = appmenu.get_style_context();
#if BOLD
                context.add_class("-vala-panel-appmenu-bold");
#endif
                context.add_class("-vala-panel-appmenu-private");
            }
        }
        // Common functions for derived classes
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
        protected bool on_menubar_sel_move(int distance)
        {
            Gtk.Allocation allocation;
            var children = menubar.get_children();
            var elem = menubar.get_selected_item();

            if (distance > 0 && elem == children.last().data)
            {
                elem = children.first().data;
                distance = -distance;
            }
            else if (distance < 0 && elem == children.first().data)
            {
                elem = children.last().data;
                distance = -distance;
            } else {
                elem = (distance > 0) ? (children.find(elem).next.data) :
                                        (children.find(elem).prev.data);
            }

            elem.get_allocation(out allocation);
            var rect = (Gdk.Rectangle)allocation;
            if (distance > 0)
            {
                double item_margin = rect.x + rect.width;
                double scroll_margin = scroll_adj.get_value() + scroll_adj.get_page_size();
                double page_size = scroll_adj.get_page_size();
                if (scroll_margin < item_margin)
                    scroll_adj.set_value(item_margin - page_size);
            }
            else if (distance < 0)
            {
                double item_margin = rect.x;
                double scroll_margin = scroll_adj.get_value();
                if (scroll_margin > item_margin)
                    scroll_adj.set_value(item_margin);
            }
            return false;
        }
    }
    public class MenuWidgetAny : MenuWidget
    {
        public MenuWidgetAny(Bamf.Application app)
        {
            appmenu = new BamfAppmenu(app);
            this.add(appmenu);
            init_style();
            this.show_all();
            completed_menus = MenuWidgetCompletionFlags.APPMENU;
        }
    }
}
