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
        protected void conf_menus()
        {
            unowned Gtk.StyleContext context;
            if ((completed_menus & MenuWidgetCompletionFlags.MENUBAR) == 0)
            {
                context = menubar.get_style_context();
                context.add_class("-vala-panel-appmenu-private");

                // Usually appmenu created more probably, than menubar
                menubar.move_selected.connect(on_menubar_sel_move);
                appmenu.move_selected.connect(on_appmenu_sel_move);
            }
            if ((completed_menus & MenuWidgetCompletionFlags.APPMENU) == 0)
            {
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
            Gdk.Rectangle rect;
            var children = menubar.get_children();
            var elem = menubar.get_selected_item();

            if (distance > 0)
            {
                if (elem == children.last().data)
                {
                    distance = -distance;
                    menubar.deselect();
                    appmenu.select_first(true);
                    elem = children.first().data;
                } else
                    elem = children.find(elem).next.data;
            }
            else if (distance < 0)
            {
                if (elem == children.first().data)
                {
                    menubar.deselect();
                    appmenu.select_first(true);
                    return false;
                } else
                    elem = children.find(elem).prev.data;
            }
            elem.get_allocation(out rect);

            if (distance == 0)
            {
                // Artificial case, for ability to manually update scroller position
                if (rect.x < scroll_adj.get_value())
                    scroll_adj.set_value(rect.x);
                else if (rect.x > scroll_adj.get_value())
                    scroll_adj.set_value(rect.x + scroll_adj.get_page_size());
                return true;
            }
            if (distance > 0)
            {
                double item_margin = rect.x + rect.width;
                double page_size = scroll_adj.get_page_size();
                double scroll_margin = scroll_adj.get_value() + page_size;
                if (scroll_margin < item_margin)
                    scroll_adj.set_value(item_margin - page_size);
                return false;
            }
            if (distance < 0)
            {
                double scroll_margin = scroll_adj.get_value();
                if (scroll_margin > rect.x)
                    scroll_adj.set_value(rect.x);
                return false;
            }
            return false;
        }
        // Don't use this handler, if menubar is not ready
        // It is just to include appmenu into menubar's keynav order
        protected bool on_appmenu_sel_move(int distance)
        {
            var children = menubar.get_children();
            appmenu.deselect();
            if (distance > 0)
                menubar.select_item(children.first().data);
            else if (distance < 0)
                menubar.select_item(children.last().data);
            menubar.move_selected(0);
            return false;
        }
    }
    public class MenuWidgetAny : MenuWidget
    {
        public MenuWidgetAny(Bamf.Application app)
        {
            appmenu = new BamfAppmenu(app);
            this.add(appmenu);
            conf_menus();
            this.show_all();
            completed_menus = MenuWidgetCompletionFlags.APPMENU;
        }
    }
}
