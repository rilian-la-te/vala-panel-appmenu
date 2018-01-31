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
using Appmenu;
using Xfce;

public class AppmenuPlugin : Xfce.PanelPlugin {

    public override void @construct() {
        GLib.Intl.setlocale(LocaleCategory.CTYPE,"");
        GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
        GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
        GLib.Intl.textdomain(Config.GETTEXT_PACKAGE);
        widget = new AppMenuBar();
        add(widget);
        add_action_widget(widget);
        this.width_request = -1;
        this.set_expand(true);
        widget.show_all();
    }
    private Gtk.Widget widget;
}

[ModuleInit]
public Type xfce_panel_module_init (TypeModule module) {
    return typeof (AppmenuPlugin);
}

public bool xfce_panel_module_preinit (string[] args) {
    Gdk.disable_multidevice();
    return true;
}
