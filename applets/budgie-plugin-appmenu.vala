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
using Budgie;

public class AppmenuApplet : Plugin, Peas.ExtensionBase
{
    public Applet get_panel_widget(string uid)
    {
        return new GlobalMenuApplet(uid);
    }
}
public class GlobalMenuApplet: Applet
{
    public string uuid { public set ; public get; }
    private void add_budgie_style(Gtk.Bin layout)
    {
        layout.get_child().notify.connect((pspec)=>{
            foreach(unowned Gtk.Widget ch in (layout.get_child() as Container).get_children())
            {
                unowned Gtk.StyleContext context = ch.get_style_context();
                context.add_class("budgie-menubar");
            }
        });
        foreach(unowned Gtk.Widget ch in (layout.get_child() as Container).get_children())
        {
            unowned Gtk.StyleContext context = ch.get_style_context();
            context.add_class("budgie-menubar");
        }
    }
    public override Widget? get_settings_ui()
    {
        var dlg = new Gtk.Box(Gtk.Orientation.VERTICAL,0);
        var entry = new CheckButton.with_label(_("Use Compact mode (all menus in application menu)"));
        this.settings.bind(Key.COMPACT_MODE,entry,"active",SettingsBindFlags.DEFAULT);
        dlg.pack_start(entry,false,false,2);
        entry = new CheckButton.with_label(_("Use bold application name"));
        this.settings.bind(Key.BOLD_APPLICATION_NAME,entry,"active",SettingsBindFlags.DEFAULT);
        dlg.pack_start(entry,false,false,2);
        dlg.show();
        return dlg;
    }
    public override bool supports_settings()
    {
        return true;
    }
    public GlobalMenuApplet (string uuid)
    {
        Object(uuid: uuid);
        settings_schema = "org.valapanel.appmenu";
        settings_prefix = "/com/solus-project/budgie-panel/instance/appmenu";
        settings = get_applet_settings(uuid);
        var layout = new Appmenu.MenuWidget();
        layout.add.connect_after((w)=>{
            add_budgie_style(layout);
        });
        settings.bind(Key.COMPACT_MODE,layout,Key.COMPACT_MODE,SettingsBindFlags.DEFAULT);
        settings.bind(Key.BOLD_APPLICATION_NAME,layout,Key.BOLD_APPLICATION_NAME,SettingsBindFlags.DEFAULT);
        this.add(layout);
        this.hexpand_set = true;
        this.vexpand_set = true;
        add_budgie_style(layout);
        layout.show();
        this.show();
    }
    private GLib.Settings settings;
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(Budgie.Plugin), typeof(AppmenuApplet));
}
