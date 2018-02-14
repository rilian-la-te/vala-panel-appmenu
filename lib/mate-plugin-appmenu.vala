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

    applet.flags = MatePanel.AppletFlags.HAS_HANDLE | MatePanel.AppletFlags.EXPAND_MAJOR;

    var layout = new Appmenu.MenuWidget();
    applet.add(layout);
    applet.show_all();
    return true;
}

/*In-process applet not working for some reason*/
//public int _mate_panel_applet_shlib_factory()
//{
//    GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
//    GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
//    return MatePanel.Applet.factory_main("AppmenuAppletFactory", false, typeof (MatePanel.Applet), factory_callback);
//}

void main(string[] args) {
    GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
    GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
    Gdk.disable_multidevice();
    Gtk.init(ref args);
    MatePanel.Applet.factory_main("AppmenuAppletFactory", true, typeof (MatePanel.Applet), factory_callback);
}
