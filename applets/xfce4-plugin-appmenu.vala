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
using Gtk;
using Appmenu;
using Xfce;

public class AppmenuPlugin : Xfce.PanelPlugin {

    public override void @construct() {
        GLib.Intl.setlocale(LocaleCategory.CTYPE,"");
        GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
        GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
        GLib.Intl.textdomain(Config.GETTEXT_PACKAGE);
        var layout = new MenuWidget();
        widget = layout;
        add(widget);
        add_action_widget(widget);
        this.width_request = -1;
        try{
            Xfconf.init();
            channel = this.get_channel();
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+Key.COMPACT_MODE,typeof(bool),widget,Key.COMPACT_MODE);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+Key.BOLD_APPLICATION_NAME,typeof(bool),widget,Key.BOLD_APPLICATION_NAME);
            Xfconf.Property.bind(channel,this.get_property_base()+"/expand",typeof(bool),widget,"hexpand");
            this.menu_show_configure();
        } catch (Xfconf.Error e) {
            stderr.printf("Xfconf init failed. Configuration will not be saved.\n");
        }
        this.shrink = true;
        this.set_expand(true);
        widget.show();
    }
    public override void configure_plugin()
    {
        var dlg = new Gtk.Dialog.with_buttons( _("Configure AppMenu"), this.get_toplevel() as Window,
                                              DialogFlags.DESTROY_WITH_PARENT,
                                              null );
        Gtk.Box dlg_vbox = dlg.get_content_area() as Gtk.Box;
        var entry = new CheckButton.with_label(_("Use Compact mode (all menus in application menu)"));
        entry.bind_property("active",widget,Key.COMPACT_MODE,BindingFlags.SYNC_CREATE);
        dlg_vbox.pack_start(entry,false,false,2);
        entry.show();
        entry = new CheckButton.with_label(_("Use bold application name"));
        entry.bind_property("active",widget,Key.BOLD_APPLICATION_NAME,BindingFlags.SYNC_CREATE);
        dlg_vbox.pack_start(entry,false,false,2);
        entry.show();
        entry = new CheckButton.with_label(_("Expand plugin on panel"));
        entry.bind_property("active",widget,"hexpand",BindingFlags.SYNC_CREATE);
        dlg_vbox.pack_start(entry,false,false,2);
        entry.show();
        dlg.show();
        dlg.present();
        dlg.unmap.connect(()=>{
            dlg.destroy();
        });
    }
    private Xfconf.Channel channel;
    private unowned MenuWidget widget;
}

[ModuleInit]
public Type xfce_panel_module_init (TypeModule module) {
    return typeof (AppmenuPlugin);
}

public bool xfce_panel_module_preinit (string[] args) {
    Gdk.disable_multidevice();
    return true;
}
