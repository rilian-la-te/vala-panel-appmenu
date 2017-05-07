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
        try
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
        } catch (GLib.Error e) {}
    }

    public override bool supports_settings()
    {
        return false;
    }
    public GlobalMenuApplet (string uuid)
    {
        Object(uuid: uuid);
        var layout = new Appmenu.AppMenuBar();
        layout.add.connect_after((w)=>{
            add_budgie_style(layout);
        });
        this.add(layout);
        add_budgie_style(layout);
        show_all();
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(Budgie.Plugin), typeof(AppmenuApplet));
}
