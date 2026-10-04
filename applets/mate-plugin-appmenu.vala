/*
 * vala-panel-appmenu
 * Copyright (C) 2016 Konstantin Pugin <ria.freelander@gmail.com>
 * Copyright (C) 2025 Kerem Soke
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

private static bool shortcut_key_is_modifier(uint keyval)
{
    switch (keyval) {
    case Gdk.Key.Alt_L:
    case Gdk.Key.Alt_R:
    case Gdk.Key.Super_L:
    case Gdk.Key.Super_R:
    case Gdk.Key.Control_L:
    case Gdk.Key.Control_R:
    case Gdk.Key.Shift_L:
    case Gdk.Key.Shift_R:
    case Gdk.Key.Meta_L:
    case Gdk.Key.Meta_R:
    case Gdk.Key.Hyper_L:
    case Gdk.Key.Hyper_R:
    case Gdk.Key.ISO_Level3_Shift:
        return true;
    default:
        return false;
    }
}

private static bool shortcut_is_modifier_only(string shortcut)
{
    if (shortcut == null || shortcut.strip().length == 0)
        return false;

    uint keyval = 0;
    Gdk.ModifierType mods = 0;
    Gtk.accelerator_parse(shortcut, out keyval, out mods);

    if (keyval == 0) {
        keyval = Gdk.keyval_from_name(shortcut);
        mods = 0;
    }

    return keyval != 0 && shortcut_key_is_modifier(keyval);
}

private static bool shortcut_is_single_modifier(string shortcut)
{
    if (!shortcut_is_modifier_only(shortcut))
        return false;

    uint keyval = 0;
    Gdk.ModifierType mods = 0;
    Gtk.accelerator_parse(shortcut, out keyval, out mods);
    if (keyval == 0)
        mods = 0;

    return mods == 0;
}

private static void update_hud_shortcut_ui(CheckButton hud_enable,
                                           CheckButton sys_shortcut_check,
                                           Widget shortcut_row,
                                           Label sys_note,
                                           Label internal_note,
                                           Label shortcut_warning,
                                           Entry shortcut_entry)
{
    bool hud_enabled = hud_enable.active;
    bool use_system = sys_shortcut_check.active;
    string shortcut = shortcut_entry.text != null ? shortcut_entry.text.strip() : "";
    bool modifier_only = shortcut_is_modifier_only(shortcut);
    bool single_modifier = shortcut_is_single_modifier(shortcut);

    shortcut_row.sensitive = hud_enabled && !use_system;
    sys_shortcut_check.sensitive = hud_enabled;

    sys_note.visible = hud_enabled && use_system;
    internal_note.visible = hud_enabled && !use_system;

    if (!(hud_enabled && !use_system)) {
        shortcut_warning.visible = false;
        shortcut_warning.label = "";
        return;
    }

    if (shortcut.length == 0) {
        shortcut_warning.visible = false;
        shortcut_warning.label = "";
    } else if (!modifier_only) {
        shortcut_warning.visible = true;
        shortcut_warning.label = dgettext(Config.GETTEXT_PACKAGE,
            "Only modifier-only shortcuts are allowed here. For shortcuts such as <Alt>space, <Alt>F3, or <Ctrl><Alt>H, use MATE Control Center.");
    } else if (single_modifier || modifier_only) {
        shortcut_warning.visible = false;
        shortcut_warning.label = "";
    } else {
        shortcut_warning.visible = false;
        shortcut_warning.label = "";
    }
}

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
    settings.bind(Key.SHOW_ICON,layout,Key.SHOW_ICON,SettingsBindFlags.DEFAULT);
    settings.bind(Key.ICON_PADDING,layout,Key.ICON_PADDING,SettingsBindFlags.DEFAULT);
    applet.add(layout);
    layout.show();
    applet.show();
    var action_group = new Gtk.ActionGroup ("AppmenuApplet Menu Actions");
    action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
    Gtk.Action a = new Gtk.Action("AppMenuAppletPreferences",N_("_Preferences"),null,Gtk.Stock.PREFERENCES);
    a.activate.connect(()=>
    {
        var dlg = new Gtk.Dialog.with_buttons( dgettext(Config.GETTEXT_PACKAGE,"Configure AppMenu"), layout.get_toplevel() as Window,
                                              DialogFlags.DESTROY_WITH_PARENT,
                                              null );
        dlg.set_property("use-header-bar", false);
        dlg.set_default_size(480, 360);
        Gtk.Box dlg_vbox = dlg.get_content_area() as Gtk.Box;

        var notebook = new Gtk.Notebook();
        notebook.set_tab_pos(Gtk.PositionType.TOP);
        notebook.set_scrollable(false);
        dlg_vbox.pack_start(notebook, true, true, 0);

        var appmenu_page = new Gtk.Box(Gtk.Orientation.VERTICAL, 8);
        appmenu_page.set_border_width(12);

        var entry = new CheckButton.with_label(dgettext(Config.GETTEXT_PACKAGE,"Use Compact mode (all menus in application menu)"));
        settings.bind(Key.COMPACT_MODE,entry,"active",SettingsBindFlags.DEFAULT);
        appmenu_page.pack_start(entry,false,false,2);
        entry.show();

        entry = new CheckButton.with_label(dgettext(Config.GETTEXT_PACKAGE,"Use bold application name"));
        settings.bind(Key.BOLD_APPLICATION_NAME,entry,"active",SettingsBindFlags.DEFAULT);
        appmenu_page.pack_start(entry,false,false,2);
        entry.show();

        entry = new CheckButton.with_label(dgettext(Config.GETTEXT_PACKAGE,"Show activities icon"));
        settings.bind(Key.SHOW_ICON,entry,"active",SettingsBindFlags.DEFAULT);
        appmenu_page.pack_start(entry,false,false,2);
        entry.show();

        var padding_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
        var padding_label = new Gtk.Label(dgettext(Config.GETTEXT_PACKAGE,"Icon padding"));
        padding_label.set_halign(Gtk.Align.START);
        var padding_adjustment = new Gtk.Adjustment(4, 0, 32, 1, 4, 0);
        var padding_spin = new Gtk.SpinButton(padding_adjustment, 1, 0);
        padding_spin.set_numeric(true);
        settings.bind(Key.ICON_PADDING, padding_spin, "value", SettingsBindFlags.DEFAULT);
        settings.bind(Key.SHOW_ICON, padding_box, "sensitive", SettingsBindFlags.DEFAULT);
        padding_box.pack_start(padding_label, true, true, 0);
        padding_box.pack_start(padding_spin, false, false, 0);
        appmenu_page.pack_start(padding_box, false, false, 2);
        padding_label.show();
        padding_spin.show();
        padding_box.show();

        var appmenu_tab = new Gtk.Label(dgettext(Config.GETTEXT_PACKAGE, "AppMenu"));
        appmenu_tab.set_halign(Gtk.Align.CENTER);
        appmenu_tab.set_hexpand(true);
        notebook.append_page(appmenu_page, appmenu_tab);
        appmenu_page.show();
        appmenu_tab.show();

        /* ── HUD page ─────────────────────────────────────────────── */
        var hud_page = new Gtk.Box(Gtk.Orientation.VERTICAL, 8);
        hud_page.set_border_width(12);

        /* Enable HUD */
        var hud_enable = new CheckButton.with_label(
            dgettext(Config.GETTEXT_PACKAGE,"Enable HUD"));
        settings.bind("hud-enabled", hud_enable, "active", SettingsBindFlags.DEFAULT);
        hud_page.pack_start(hud_enable, false, false, 2);
        hud_enable.show();

        /* Shortcut row (label + entry) */
        var shortcut_row = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
        var shortcut_label = new Gtk.Label(
            dgettext(Config.GETTEXT_PACKAGE,"Shortcut:"));
        shortcut_label.set_halign(Gtk.Align.START);
        var shortcut_entry = new Gtk.Entry();
        shortcut_entry.set_placeholder_text("Alt_L or <Alt>Super_L");
        shortcut_entry.set_width_chars(16);
        shortcut_entry.set_tooltip_text(
            dgettext(Config.GETTEXT_PACKAGE,
                "Internal HUD mode is for modifier-only shortcuts, such as \"Alt_L\" or \"<Alt>Super_L\"."));
        bool syncing_shortcut = false;
        string last_valid_shortcut = settings.get_string("hud-shortcut");
        shortcut_entry.text = last_valid_shortcut;
        shortcut_row.pack_start(shortcut_label, false, false, 0);
        shortcut_row.pack_start(shortcut_entry, true, true, 0);
        hud_page.pack_start(shortcut_row, false, false, 2);
        shortcut_label.show();
        shortcut_entry.show();
        shortcut_row.show();

        /* Use system shortcut checkbox */
        var sys_shortcut_check = new CheckButton.with_label(
            dgettext(Config.GETTEXT_PACKAGE,"Let MATE Control Center handle shortcut"));
        settings.bind("hud-use-system-shortcut", sys_shortcut_check,
                      "active", SettingsBindFlags.DEFAULT);
        hud_page.pack_start(sys_shortcut_check, false, false, 2);
        sys_shortcut_check.show();

        /* Explanatory note */
        var sys_note = new Gtk.Label(
            dgettext(Config.GETTEXT_PACKAGE,
                "When enabled, the daemon will not register shortcuts by itself.\n"
                + "Configure the shortcut in MATE Control Center → Keyboard Shortcuts.\n"
                + "Set the command to:\n"
                + "  gdbus call --session --dest org.mate.HUD \\\n"
                + "    --object-path /org/mate/HUD --method org.mate.HUD.Activate 0"));
        sys_note.set_halign(Gtk.Align.START);
        sys_note.set_selectable(true);
        sys_note.set_line_wrap(true);
        sys_note.set_margin_start(20);
        sys_note.set_xalign(0.0f);
        hud_page.pack_start(sys_note, false, false, 2);

        var internal_note = new Gtk.Label(
            dgettext(Config.GETTEXT_PACKAGE,
                "Internal HUD shortcut mode is reserved for modifier-only shortcuts such as Alt or Alt+Super.\n"
                + "For full accelerators such as <Alt>space, <Alt>F3, or <Ctrl><Alt>H, use MATE Control Center."));
        internal_note.set_halign(Gtk.Align.START);
        internal_note.set_line_wrap(true);
        internal_note.set_xalign(0.0f);
        internal_note.set_margin_start(20);
        hud_page.pack_start(internal_note, false, false, 2);

        var shortcut_warning = new Gtk.Label("");
        shortcut_warning.set_halign(Gtk.Align.START);
        shortcut_warning.set_line_wrap(true);
        shortcut_warning.set_xalign(0.0f);
        shortcut_warning.set_margin_start(20);
        shortcut_warning.get_style_context().add_class("warning");
        hud_page.pack_start(shortcut_warning, false, false, 2);

        var placement_group = new Gtk.Box(Gtk.Orientation.VERTICAL, 8);
        placement_group.set_margin_top(8);
        hud_page.pack_start(placement_group, false, false, 2);

        var attach_row = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
        var attach_label = new Gtk.Label(
            dgettext(Config.GETTEXT_PACKAGE, "Attach HUD to:"));
        attach_label.set_halign(Gtk.Align.START);
        var attach_switch = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
        attach_switch.get_style_context().add_class("linked");
        var attach_monitor = new Gtk.ToggleButton.with_label(
            dgettext(Config.GETTEXT_PACKAGE, "monitor"));
        var attach_window = new Gtk.ToggleButton.with_label(
            dgettext(Config.GETTEXT_PACKAGE, "window"));
        attach_switch.pack_start(attach_monitor, false, false, 0);
        attach_switch.pack_start(attach_window, false, false, 0);
        attach_row.pack_start(attach_label, true, true, 0);
        attach_row.pack_start(attach_switch, false, false, 0);
        placement_group.pack_start(attach_row, false, false, 2);

        bool syncing_attach = false;
        string attach_to = settings.get_string("hud-attach-to");
        syncing_attach = true;
        attach_monitor.active = attach_to == "monitor";
        attach_window.active = attach_to != "monitor";
        syncing_attach = false;

        attach_monitor.clicked.connect(() => {
            if (syncing_attach)
                return;
            syncing_attach = true;
            attach_monitor.active = true;
            attach_window.active = false;
            settings.set_string("hud-attach-to", "monitor");
            syncing_attach = false;
        });
        attach_window.clicked.connect(() => {
            if (syncing_attach)
                return;
            syncing_attach = true;
            attach_monitor.active = false;
            attach_window.active = true;
            settings.set_string("hud-attach-to", "window");
            syncing_attach = false;
        });
        settings.changed["hud-attach-to"].connect(() => {
            if (syncing_attach)
                return;
            string value = settings.get_string("hud-attach-to");
            syncing_attach = true;
            attach_monitor.active = value == "monitor";
            attach_window.active = value != "monitor";
            syncing_attach = false;
        });

        var location_label = new Gtk.Label(
            dgettext(Config.GETTEXT_PACKAGE, "HUD location:"));
        location_label.set_halign(Gtk.Align.START);
        placement_group.pack_start(location_label, false, false, 2);

        var location_grid = new Gtk.Grid();
        location_grid.set_row_spacing(4);
        location_grid.set_column_spacing(4);
        location_grid.set_halign(Gtk.Align.START);
        location_grid.set_row_homogeneous(true);
        location_grid.set_column_homogeneous(true);
        location_grid.set_size_request(120, 120);

        string[] location_values = {
            "northwest", "north", "northeast",
            "west", "center", "east",
            "southwest", "south", "southeast"
        };
        string[] location_labels = {
            "NW", "N", "NE",
            "W", dgettext(Config.GETTEXT_PACKAGE, "Center"), "E",
            "SW", "S", "SE"
        };
        var location_buttons = new Gtk.RadioButton[9];
        bool syncing_location = true;
        string hud_location = settings.get_string("hud-location");

        for (int i = 0; i < 9; i++) {
            var loc_button = new Gtk.RadioButton(null);
            if (i > 0)
                loc_button.join_group(location_buttons[0]);
            loc_button.set_halign(Gtk.Align.CENTER);
            loc_button.set_valign(Gtk.Align.CENTER);
            loc_button.set_tooltip_text(location_labels[i]);
            loc_button.get_accessible().set_name(location_labels[i]);
            loc_button.set_data<string>("hud-location-value", location_values[i]);
            location_buttons[i] = loc_button;
            location_grid.attach(loc_button, i % 3, i / 3, 1, 1);
            loc_button.toggled.connect(() => {
                if (syncing_location || !loc_button.active)
                    return;

                string value = loc_button.get_data<string>("hud-location-value");
                syncing_location = true;
                settings.set_string("hud-location", value);
                syncing_location = false;
            });
        }

        syncing_location = true;
        int selected_location = 0;
        for (int i = 0; i < 9; i++) {
            if (hud_location == location_values[i])
                selected_location = i;
        }
        location_buttons[selected_location].active = true;
        syncing_location = false;

        settings.changed["hud-location"].connect(() => {
            if (syncing_location)
                return;
            string value = settings.get_string("hud-location");
            syncing_location = true;
            int selected = 0;
            for (int i = 0; i < 9; i++) {
                if (value == location_values[i])
                    selected = i;
            }
            location_buttons[selected].active = true;
            syncing_location = false;
        });

        placement_group.sensitive = hud_enable.active;
        attach_label.show();
        attach_monitor.show();
        attach_window.show();
        attach_switch.show();
        attach_row.show();
        location_label.show();
        location_grid.show_all();
        placement_group.show();

        var hud_tab = new Gtk.Label(dgettext(Config.GETTEXT_PACKAGE, "HUD"));
        hud_tab.set_halign(Gtk.Align.CENTER);
        hud_tab.set_hexpand(true);
        notebook.append_page(hud_page, hud_tab);
        hud_page.show();
        hud_tab.show();

        settings.changed["hud-shortcut"].connect(() => {
            if (syncing_shortcut)
                return;

            syncing_shortcut = true;
            last_valid_shortcut = settings.get_string("hud-shortcut");
            if (shortcut_entry.text != last_valid_shortcut)
                shortcut_entry.text = last_valid_shortcut;
            syncing_shortcut = false;

            update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                                   sys_note, internal_note, shortcut_warning,
                                   shortcut_entry);
        });
        shortcut_entry.changed.connect(() => {
            if (syncing_shortcut)
                return;

            string candidate = shortcut_entry.text != null ? shortcut_entry.text.strip() : "";
            if (candidate == "" || shortcut_is_modifier_only(candidate)) {
                syncing_shortcut = true;
                last_valid_shortcut = candidate;
                settings.set_string("hud-shortcut", candidate);
                syncing_shortcut = false;
            }

            update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                                   sys_note, internal_note, shortcut_warning,
                                   shortcut_entry);
        });
        shortcut_entry.focus_out_event.connect((event) => {
            string candidate = shortcut_entry.text != null ? shortcut_entry.text.strip() : "";
            if (!(candidate == "" || shortcut_is_modifier_only(candidate))) {
                syncing_shortcut = true;
                shortcut_entry.text = last_valid_shortcut;
                syncing_shortcut = false;
                update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                                       sys_note, internal_note, shortcut_warning,
                                       shortcut_entry);
            }
            return false;
        });
        hud_enable.toggled.connect(() => {
            update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                                   sys_note, internal_note, shortcut_warning,
                                   shortcut_entry);
            placement_group.sensitive = hud_enable.active;
        });
        sys_shortcut_check.toggled.connect(() => {
            update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                                   sys_note, internal_note, shortcut_warning,
                                   shortcut_entry);
        });
        update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                               sys_note, internal_note, shortcut_warning,
                               shortcut_entry);

        notebook.show();
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
