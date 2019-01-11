/*
 * vala-panel-appmenu
 * Copyright (C) 2016 Konstantin Pugin <ria.freelander@gmail.com>
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
using Appmenu;
private bool factory_callback(MatePanel.Applet applet, string iid)
{
    if (iid != "AppmenuApplet") {
        return false;
    }
    applet.flags = MatePanel.AppletFlags.HAS_HANDLE | MatePanel.AppletFlags.EXPAND_MAJOR | MatePanel.AppletFlags.EXPAND_MINOR;
    var layout = new Appmenu.MenuWidget();
    var settings = MatePanel.AppletSettings.@new(applet,"org.valapanel.appmenu");
    settings.bind(Key.COMPACT_MODE,layout,Key.COMPACT_MODE,SettingsBindFlags.DEFAULT);
    settings.bind(Key.BOLD_APPLICATION_NAME,layout,Key.BOLD_APPLICATION_NAME,SettingsBindFlags.DEFAULT);
    applet.add(layout);
    layout.show();
    applet.show();
    var action_group = new Gtk.ActionGroup ("AppmenuApplet Menu Actions");
    action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
    Gtk.Action a = new Gtk.Action("AppMenuAppletPreferences",N_("_Preferences"),null,Gtk.Stock.PREFERENCES);
    a.activate.connect(()=>
    {
        var dlg = new Gtk.Dialog.with_buttons( _("Configure AppMenu"), layout.get_toplevel() as Window,
                                              DialogFlags.DESTROY_WITH_PARENT,
                                              null );
        Gtk.Box dlg_vbox = dlg.get_content_area() as Gtk.Box;
        var entry = new CheckButton.with_label(_("Use Compact mode (all menus in application menu)"));
        settings.bind(Key.COMPACT_MODE,entry,"active",SettingsBindFlags.DEFAULT);
        dlg_vbox.pack_start(entry,false,false,2);
        entry.show();
        entry = new CheckButton.with_label(_("Use bold application name"));
        settings.bind(Key.BOLD_APPLICATION_NAME,entry,"active",SettingsBindFlags.DEFAULT);
        dlg_vbox.pack_start(entry,false,false,2);
        entry.show();
        dlg.show();
        dlg.present();
        dlg.response.connect(()=>{
            dlg.destroy();
        });
    });
    action_group.add_action (a);
    applet.setup_menu("""<menuitem name="Appmenu Preferences Item" action="AppMenuAppletPreferences" />""",action_group);
    return true;
}


public int _mate_panel_applet_shlib_factory()
{
//    GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
//    GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
    return MatePanel.Applet.factory_setup_in_process("AppmenuAppletFactory", typeof (MatePanel.Applet), factory_callback);
}

//void main(string[] args) {
//    Gtk.init(ref args);
//    MatePanel.Applet.factory_main("SNTrayAppletFactory", true, typeof (MatePanel.Applet), StatusNotifier.factory_callback);
//}
