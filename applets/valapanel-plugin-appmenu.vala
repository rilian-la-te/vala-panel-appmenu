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


#if NEW
public class AppmenuApplet : AppletPlugin
{
    public override Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new GlobalMenuApplet(toplevel,settings,number);
    }
}
#else
public class AppmenuApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new GlobalMenuApplet(toplevel,settings,number);
    }
}
#endif
public class GlobalMenuApplet: Applet
{
    unowned MenuWidget layout;
    public GlobalMenuApplet (Toplevel top, GLib.Settings? settings, string number)
    {
        base(top,settings,number);
        (this.action_group.lookup_action(AppletAction.CONFIGURE) as SimpleAction).set_enabled(true);
        var layout = new Appmenu.MenuWidget();
        this.layout = layout;
        settings.bind(Key.COMPACT_MODE,layout,Key.COMPACT_MODE,SettingsBindFlags.DEFAULT);
        settings.bind(Key.BOLD_APPLICATION_NAME,layout,Key.BOLD_APPLICATION_NAME,SettingsBindFlags.DEFAULT);
        this.add(layout);
        layout.show();
        this.show();
    }
    ~GlobalMenuApplet()
    {
        layout.unref();
    }
    public override Widget get_settings_ui()
    {
        var dlg = new Gtk.Box(Gtk.Orientation.VERTICAL,0);
        var entry = new CheckButton.with_label(_("Use Compact mode (all menus in application menu)"));
        this.settings.bind(Key.COMPACT_MODE,entry,"active",SettingsBindFlags.DEFAULT);
        dlg.pack_start(entry,false,false,2);
        entry.show();
        entry = new CheckButton.with_label(_("Use bold application name"));
        this.settings.bind(Key.BOLD_APPLICATION_NAME,entry,"active",SettingsBindFlags.DEFAULT);
        dlg.pack_start(entry,false,false,2);
        entry.show();
        dlg.show();
        return dlg;
    }
} // End class

#if NEW
[ModuleInit]
public void g_io_appmenu_load(GLib.TypeModule module)
{
    // boilerplate - all modules need this
//    GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
//    GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
    module.use();
    GLib.IOExtensionPoint.implement(ValaPanel.Applet.EXTENSION_POINT,typeof(AppmenuApplet),"org.valapanel.appmenu",10);
}

public void g_io_appmenu_unload(GLib.IOModule module)
{
    // boilerplate - all modules need this
}
#else
[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(AppmenuApplet));
}
#endif
