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
    [CCode (cname = "appmenu_get_applicable_backend")]
    private extern static Backend? register_backend ();
    public class MenuWidget: Gtk.Bin
    {
        public bool compact_mode {get; set; default = false;}
        public bool bold_application_name {get; set; default = false;}
        private Gtk.Adjustment? scroll_adj = null;
        private Gtk.ScrolledWindow? scroller = null;
        private Gtk.CssProvider provider;
        private GLib.MenuModel? appmenu = null;
        private GLib.MenuModel? menubar = null;
        private GLib.HashTable<unowned string, GLib.ActionGroup?> action_groups = new GLib.HashTable<unowned string, GLib.ActionGroup?>(str_hash, str_equal);
        private Backend backend;
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
            backend = register_backend();
            backend_connector = backend.active_model_changed.connect(()=>{
                restock();
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
        private void restock_from_model(GLib.MenuModel model, GLib.HashTable<unowned string, GLib.ActionGroup?> new_actions)
        {
            var iter = HashTableIter<unowned string, GLib.ActionGroup?> (this.action_groups);
            this.menubar = model;
            unowned string key;
            unowned GLib.ActionGroup? val;
            while (iter.next (out key, out val)) {
                this.insert_action_group(key, null);
                iter.remove ();
            }
            new_actions.foreach ((tkey, tval) => {
                this.insert_action_group(tkey, tval);
                this.action_groups.insert(tkey, tval);
            });
            mwidget.bind_model(this.menubar,null,true);
            unowned Gtk.StyleContext mcontext = mwidget.get_style_context();
            if(this.menubar != null && this.menubar.get_n_items() > 0)
            {
                var children = mwidget.get_children();
                if(children.length() > 0)
                    children.nth_data(0).get_style_context().add_class("-vala-panel-appmenu-application-name");
            }
            if(bold_application_name)
                mcontext.add_class("-vala-panel-appmenu-bold");
            else
                mcontext.remove_class("-vala-panel-appmenu-bold");
        }
        private void restock()
        {
            if(this.backend != null && this.backend.helper != null && this.backend.helper.menu != null)
                this.restock_from_model(backend.helper.menu, backend.helper.actions);
            else
                mwidget.bind_model(null, null, false);
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
        protected override void get_preferred_height(out int minimum_height, out int natural_height)
        {
            int min, nat;
            scroller.get_preferred_height(out min, null);
            mwidget.get_preferred_height(null, out nat);
            minimum_height = int.min(min, nat);
            natural_height = int.max(min, nat);
        }
        protected override void get_preferred_width(out int minimum_height, out int natural_height)
        {
            int min, nat;
            scroller.get_preferred_width(out min, null);
            mwidget.get_preferred_width(null, out nat);
            minimum_height = int.min(min, nat);
            natural_height = int.max(min, nat);
        }
    }
}
