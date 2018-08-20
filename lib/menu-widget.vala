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

namespace Key
{
    public const string COMPACT_MODE = "compact-mode";
    public const string BOLD_APPLICATION_NAME = "bold-application-name";
}

namespace Appmenu
{
    public class MenuWidget: Gtk.Bin
    {
        public bool compact_mode {get; set; default = false;}
        public bool bold_application_name {get; set; default = false;}
        private Gtk.Adjustment? scroll_adj = null;
        private Gtk.ScrolledWindow? scroller = null;
        private Gtk.CssProvider provider;
        private GLib.MenuModel? appmenu = null;
        private GLib.MenuModel? menubar = null;
        private Backend backend = new BackendBAMF();
        private Gtk.MenuBar mwidget = new Gtk.MenuBar();
        private ulong backend_connector = 0;
        private ulong compact_connector = 0;
        construct
        {
            provider = new Gtk.CssProvider();
            provider.load_from_resource("/org/vala-panel/appmenu/appmenu.css");
            unowned Gtk.StyleContext context = this.get_style_context();
            context.add_class("-vala-panel-appmenu-core");
            unowned Gtk.StyleContext mcontext = mwidget.get_style_context();
            Signal.connect(this,"notify",(GLib.Callback)restock,null);
            backend_connector = backend.active_model_changed.connect(()=>{
                Timeout.add(50,()=>{
                    backend.set_active_window_menu(this);
                    return Source.REMOVE;
                });
            });
            mcontext.add_class("-vala-panel-appmenu-private");
            Gtk.StyleContext.add_provider_for_screen(this.get_screen(), provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
            //Setup menubar
            scroll_adj = new Gtk.Adjustment(0, 0, 0, 20, 20, 0);
            scroller = new Gtk.ScrolledWindow(scroll_adj, null);
            scroller.set_hexpand(true);
            scroller.set_policy(Gtk.PolicyType.EXTERNAL, Gtk.PolicyType.NEVER);
            scroller.set_shadow_type(Gtk.ShadowType.NONE);
            scroller.scroll_event.connect(on_scroll_event);
            scroller.set_min_content_width(16);
            scroller.set_min_content_height(16);
            scroller.set_propagate_natural_height(true);
            scroller.set_propagate_natural_width(true);
            this.add(scroller);
            scroller.add(mwidget);
            mwidget.show();
            scroller.show();
            this.show();
        }
        public MenuWidget()
        {
            Object();
        }
        private void restock()
        {
            unowned Gtk.StyleContext mcontext = mwidget.get_style_context();
            if(bold_application_name)
                mcontext.add_class("-vala-panel-appmenu-bold");
            else
                mcontext.remove_class("-vala-panel-appmenu-bold");
            var menu = new GLib.Menu();
            if (this.appmenu != null)
                menu.append_section(null,this.appmenu);
            if (this.menubar != null)
                menu.append_section(null,this.menubar);

            int items = -1;
            if (this.menubar != null)
                items = this.menubar.get_n_items();

            if (this.compact_mode && items == 0)
            {
                compact_connector = this.menubar.items_changed.connect((a,b,c)=>{
                    restock();
                });
            }
            if (this.compact_mode && items > 0)
            {
                if(compact_connector > 0)
                {
                    this.menubar.disconnect(compact_connector);
                    compact_connector = 0;
                }
                var compact = new GLib.Menu();
                string? name = null;
                if(this.appmenu != null)
                    this.appmenu.get_item_attribute(0,"label","s",&name);
                else
                    name = GLib.dgettext(Config.GETTEXT_PACKAGE,"Compact Menu");
                compact.append_submenu(name,menu);
                mwidget.bind_model(compact,null,true);
            }
            else
                mwidget.bind_model(menu,null,true);
        }
        public void set_appmenu(GLib.MenuModel? appmenu_model)
        {
            this.appmenu = appmenu_model;
            this.restock();
        }
        public void set_menubar(GLib.MenuModel? menubar_model)
        {
            this.menubar = menubar_model;
            this.restock();
        }
        protected bool on_scroll_event(Gtk.Widget w, Gdk.EventScroll event)
        {
            var val = scroll_adj.get_value();
            var incr = scroll_adj.get_step_increment();
            if (event.direction == Gdk.ScrollDirection.UP)
            {
                scroll_adj.set_value(val - incr);
                return true;
            }
            if (event.direction == Gdk.ScrollDirection.DOWN)
            {
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
                scroll_adj.set_value(val + incr * (event.delta_y + event.delta_x));
                return true;
            }
            return false;
        }
        protected override void map()
        {
            base.map();
            unowned Gtk.Settings gtksettings = this.get_settings();
            gtksettings.gtk_shell_shows_app_menu = false;
            gtksettings.gtk_shell_shows_menubar = false;
        }
    }
}
