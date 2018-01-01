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
        private Gtk.CssProvider provider;
        construct
        {
            provider = new Gtk.CssProvider();
            provider.load_from_resource("/org/vala-panel/appmenu/appmenu.css");
            unowned Gtk.StyleContext context = this.get_style_context();
            Gtk.StyleContext.add_provider_for_screen(this.get_screen(), provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        private void apply_style(Gtk.Widget ch)
        {
            unowned Gtk.StyleContext context = ch.get_style_context();
#if BOLD
            context.add_class("-vala-panel-appmenu-bold");
#endif
            context.add_class("-vala-panel-appmenu-private");
        }
        public void set_appmenu(Gtk.MenuBar? appmenu)
        {
            if (appmenu != null)
            {
                if (this.get_child1() is Gtk.Widget)
                    this.get_child1().destroy();
                this.pack1(appmenu,false,false);
                apply_style(appmenu);
                completed_menus |= MenuWidgetCompletionFlags.APPMENU;
            }
            else
                completed_menus &= ~MenuWidgetCompletionFlags.APPMENU;
        }
        public void set_menubar(Gtk.MenuBar? appmenu)
        {
            if (appmenu != null)
            {
                if (this.get_child2() is Gtk.Widget)
                    this.get_child2().destroy();
                this.pack2(appmenu,true,false);
                apply_style(appmenu);
                completed_menus |= MenuWidgetCompletionFlags.MENUBAR;
            }
            else
                completed_menus &= ~MenuWidgetCompletionFlags.MENUBAR;
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
