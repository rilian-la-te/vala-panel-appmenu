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
        private Gtk.CssProvider provider;
        construct
        {
            provider = new Gtk.CssProvider();
            File ruri = File.new_for_uri("resource://org/vala-panel/appmenu/appmenu.css");
            try
            {
                provider.load_from_file(ruri);
                this.notify.connect((pspec)=>{
                    foreach(unowned Gtk.Widget ch in this.get_children())
                    {
                        unowned Gtk.StyleContext context = ch.get_style_context();
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
            var appmenu = new BamfAppmenu(app);
            this.add(appmenu);
            this.show_all();
            completed_menus = MenuWidgetCompletionFlags.APPMENU;
        }
    }
}
