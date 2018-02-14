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

using Gtk;
using GLib;
using ValaPanel;
using Appmenu;

public class AppmenuApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
#if NEW
                                    string number)
#else
                                    uint number)
#endif
    {
        return new GlobalMenuApplet(toplevel,settings,number);
    }
}
public class GlobalMenuApplet: Applet
#if NEW
#else
    , AppletConfigurable
#endif
{
    unowned MenuWidget layout;
#if NEW
    public GlobalMenuApplet (Toplevel top, GLib.Settings? settings, string number)
#else
    public GlobalMenuApplet (Toplevel top, GLib.Settings? settings, uint number)
#endif
    {
        base(top,settings,number);
#if NEW
        (this.action_group.lookup_action(AppletAction.CONFIGURE) as SimpleAction).set_enabled(true);
#else
    }
    public override void create()
    {
#endif
        var layout = new Appmenu.MenuWidget();
        this.layout = layout;
        settings.bind(Key.COMPACT_MODE,layout,Key.COMPACT_MODE,SettingsBindFlags.DEFAULT);
        settings.bind(Key.BOLD_APPLICATION_NAME,layout,Key.BOLD_APPLICATION_NAME,SettingsBindFlags.DEFAULT);
        this.add(layout);
        show_all();
    }
#if NEW
    public override Widget get_settings_ui()
    {
        var dlg = new Gtk.Box(Gtk.Orientation.VERTICAL,0);
        var entry = new CheckButton.with_label(_("Use Compact mode (all menus in application menu)"));
        this.settings.bind(Key.COMPACT_MODE,entry,"active",SettingsBindFlags.DEFAULT);
        dlg.pack_start(entry,false,false,2);
        entry = new CheckButton.with_label(_("Use bold application name"));
        this.settings.bind(Key.BOLD_APPLICATION_NAME,entry,"active",SettingsBindFlags.DEFAULT);
        dlg.pack_start(entry,false,false,2);
        dlg.show_all();
        return dlg;
    }
#else
    public Dialog get_config_dialog()
    {
        var dlg = new Gtk.Dialog.with_buttons( _("Configure AppMenu"), toplevel, DialogFlags.DESTROY_WITH_PARENT,
                                             _("_Close"),
                                              ResponseType.CLOSE,
                                              null );
        Gtk.Box dlg_vbox = dlg.get_content_area() as Gtk.Box;
        var entry = new CheckButton.with_label(_("Use Compact mode (all menus in application menu)"));
        this.settings.bind(Key.COMPACT_MODE,entry,"active",SettingsBindFlags.DEFAULT);
        dlg_vbox.pack_start(entry,false,false,2);
        entry = new CheckButton.with_label(_("Use bold application name"));
        this.settings.bind(Key.BOLD_APPLICATION_NAME,entry,"active",SettingsBindFlags.DEFAULT);
        dlg_vbox.pack_start(entry,false,false,2);
        dlg_vbox.show_all();
        return dlg;
    }
#endif
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(AppmenuApplet));
}
