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
{
#if NEW
    public GlobalMenuApplet (Toplevel top, GLib.Settings? settings, string number)
#else
    public GlobalMenuApplet (Toplevel top, GLib.Settings? settings, uint number)
#endif
    {
        base(top,settings,number);
#if NEW
#else
    }
    public override void create()
    {
#endif
        var layout = new Appmenu.AppMenuBar();
        this.add(layout);
        show_all();
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(AppmenuApplet));
}
