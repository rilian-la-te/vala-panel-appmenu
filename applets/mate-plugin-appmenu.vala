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

private static Gtk.Action? preferences_action;
private static uint preferences_bus_id;

[DBus (name = "org.mate.GlobalMenu.Preferences")]
private class PreferencesService : Object
{
    public void show() throws GLib.Error
    {
        if (preferences_action != null)
            preferences_action.activate();
    }
}

private static void export_preferences_action(Gtk.Action action)
{
    preferences_action = action;
    if (preferences_bus_id != 0)
        return;

    preferences_bus_id = Bus.own_name(
        BusType.SESSION,
        "org.mate.GlobalMenu",
        BusNameOwnerFlags.NONE,
        (connection) => {
            try {
                connection.register_object(
                    "/org/mate/GlobalMenu/Preferences",
                    new PreferencesService());
            } catch (Error error) {
                warning("Could not export Global Menu preferences: %s",
                        error.message);
            }
        },
        null,
        null);
}

private static int shortcut_preset_index(string shortcut)
{
    uint keyval;
    Gdk.ModifierType mods;
    Gtk.accelerator_parse(shortcut, out keyval, out mods);
    if (mods == Gdk.ModifierType.CONTROL_MASK &&
        (keyval == Gdk.Key.Alt_L || keyval == Gdk.Key.Alt_R))
        return 4;
    if (mods != 0)
        return -1;
    switch (keyval) {
    case Gdk.Key.Alt_L:
    case Gdk.Key.Alt_R:
    case Gdk.Key.Meta_L:
    case Gdk.Key.Meta_R:
        return 0;
    case Gdk.Key.Super_L:
    case Gdk.Key.Super_R:
        return 1;
    case Gdk.Key.Hyper_L:
    case Gdk.Key.Hyper_R:
        return 2;
    case Gdk.Key.ISO_Level3_Shift:
        return 3;
    default:
        return -1;
    }
}

private static string shortcut_preset_value(int preset, bool right,
                                             Gdk.Keymap keymap, string previous)
{
    string suffix = right ? "_R" : "_L";
    if (preset == 3)
        return "ISO_Level3_Shift";
    if (preset == 4)
        return "<Control>Alt" + suffix;
    if (preset == 1)
        return "Super" + suffix;
    if (preset == 2)
        return "Hyper" + suffix;
    string preferred = previous.has_prefix("Meta_") ? "Meta" : "Alt";
    string fallback = preferred == "Alt" ? "Meta" : "Alt";
    Gdk.KeymapKey[] keys;
    if (!keymap.get_entries_for_keyval(Gdk.keyval_from_name(preferred + suffix), out keys) &&
        keymap.get_entries_for_keyval(Gdk.keyval_from_name(fallback + suffix), out keys))
        preferred = fallback;
    return preferred + suffix;
}

private static void populate_shortcut_side(Gtk.ComboBoxText side, int preset, bool right)
{
    side.remove_all();
    if (preset == 3) {
        side.append_text(dgettext(Config.GETTEXT_PACKAGE, "Keyboard layout"));
        side.active = 0;
        side.sensitive = false;
    } else {
        side.append_text(dgettext(Config.GETTEXT_PACKAGE, "Left"));
        side.append_text(dgettext(Config.GETTEXT_PACKAGE, "Right"));
        side.active = right ? 1 : 0;
        side.sensitive = true;
    }
}

private static void update_hud_shortcut_ui(CheckButton hud_enable,
                                           CheckButton sys_shortcut_check,
                                           Widget shortcut_row,
                                           Label sys_note,
                                           Label internal_note,
                                           Label shortcut_warning,
                                           Gtk.ToggleButton[] shortcut_buttons)
{
    bool internal = hud_enable.active && !sys_shortcut_check.active;
    shortcut_row.sensitive = internal;
    sys_shortcut_check.sensitive = hud_enable.active;
    sys_note.visible = hud_enable.active && sys_shortcut_check.active;
    internal_note.visible = internal;
    bool selected = false;
    foreach (var button in shortcut_buttons)
        selected = selected || button.active;
    shortcut_warning.visible = internal && !selected;
}

private static void update_hud_applet_geometry(MatePanel.Applet applet,
                                                GLib.Settings settings)
{
    var toplevel = applet.get_toplevel();
    var window = toplevel.get_window();
    int offset_x = 0;
    int offset_y = 0;
    int origin_x = 0;
    int origin_y = 0;

    if (window == null ||
        !applet.translate_coordinates(toplevel, 0, 0, out offset_x, out offset_y))
        return;

    window.get_origin(out origin_x, out origin_y);
    int x = origin_x + offset_x;
    int y = origin_y + offset_y;
    int width = applet.get_allocated_width();
    int height = applet.get_allocated_height();
    if (width <= 0 || height <= 0)
        return;

    if (settings.get_int("hud-applet-x") != x)
        settings.set_int("hud-applet-x", x);
    if (settings.get_int("hud-applet-y") != y)
        settings.set_int("hud-applet-y", y);
    if (settings.get_int("hud-applet-width") != width)
        settings.set_int("hud-applet-width", width);
    if (settings.get_int("hud-applet-height") != height)
        settings.set_int("hud-applet-height", height);
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
    applet.size_allocate.connect(() => update_hud_applet_geometry(applet, settings));
    applet.map_event.connect(() => {
        update_hud_applet_geometry(applet, settings);
        return false;
    });
    applet.get_toplevel().configure_event.connect(() => {
        update_hud_applet_geometry(applet, settings);
        return false;
    });
    update_hud_applet_geometry(applet, settings);
    var action_group = new Gtk.ActionGroup ("AppmenuApplet Menu Actions");
    action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
    Gtk.Action a = new Gtk.Action("AppMenuAppletPreferences",N_("_Preferences"),null,Gtk.Stock.PREFERENCES);
    a.activate.connect(()=>
    {
        var dlg = (Gtk.Dialog) GLib.Object.new(typeof(Gtk.Dialog),
            "title", dgettext(Config.GETTEXT_PACKAGE, "Configure AppMenu"),
            "transient-for", layout.get_toplevel() as Window,
            "destroy-with-parent", true,
            "use-header-bar", 0,
            "resizable", false);
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

        entry = new CheckButton.with_label(dgettext(Config.GETTEXT_PACKAGE,"Show application icon"));
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

        /* Modifier-only shortcut selector. Opening preferences never changes it. */
        var shortcut_row = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
        var shortcut_label = new Gtk.Label(
            dgettext(Config.GETTEXT_PACKAGE, "Shortcut:"));
        shortcut_label.set_halign(Gtk.Align.START);
        var shortcut_switch = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
        shortcut_switch.get_style_context().add_class("linked");
        string[] shortcut_labels = { "Alt/Meta", "Super", "Hyper", "AltGr", "Ctrl+Alt" };
        var keymap = Gdk.Keymap.get_for_display(applet.get_display());
        var shortcut_side = new Gtk.ComboBoxText();
        shortcut_side.get_accessible().set_name(dgettext(Config.GETTEXT_PACKAGE, "Modifier side"));
        shortcut_side.set_tooltip_text(dgettext(Config.GETTEXT_PACKAGE,
            "Choose the modifier key's side. For Ctrl+Alt, this selects Alt; either Ctrl key works. AltGr follows your keyboard layout."));
        var shortcut_buttons = new Gtk.ToggleButton[5];
        bool syncing_shortcut = true;
        string saved_shortcut = settings.get_string("hud-shortcut");
        bool shortcut_right = saved_shortcut.has_suffix("_R");
        int selected_shortcut = shortcut_preset_index(saved_shortcut);
        populate_shortcut_side(shortcut_side, selected_shortcut, shortcut_right);
        for (int i = 0; i < shortcut_buttons.length; i++) {
            var button = new Gtk.ToggleButton.with_label(shortcut_labels[i]);
            button.set_data<int>("shortcut-preset", i);
            button.active = selected_shortcut == i;
            shortcut_buttons[i] = button;
            shortcut_switch.pack_start(button, false, false, 0);
            button.clicked.connect(() => {
                if (syncing_shortcut)
                    return;
                syncing_shortcut = true;
                int preset = button.get_data<int>("shortcut-preset");
                foreach (var other in shortcut_buttons)
                    other.active = other == button;
                populate_shortcut_side(shortcut_side, preset, shortcut_right);
                settings.set_string("hud-shortcut", shortcut_preset_value(preset,
                    shortcut_right, keymap, settings.get_string("hud-shortcut")));
                syncing_shortcut = false;
            });
        }
        shortcut_side.changed.connect(() => {
            if (syncing_shortcut || shortcut_side.active < 0)
                return;
            int preset = shortcut_preset_index(settings.get_string("hud-shortcut"));
            if (preset == 3)
                return;
            shortcut_right = shortcut_side.active == 1;
            if (preset >= 0)
                settings.set_string("hud-shortcut", shortcut_preset_value(preset,
                    shortcut_right, keymap, settings.get_string("hud-shortcut")));
        });
        syncing_shortcut = false;
        shortcut_row.pack_start(shortcut_label, true, true, 0);
        shortcut_row.pack_start(shortcut_switch, false, false, 0);
        shortcut_row.pack_start(shortcut_side, false, false, 0);
        hud_page.pack_start(shortcut_row, false, false, 2);
        shortcut_row.show_all();

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
                "Tap and release the selected modifier to open the HUD.\n"
                + "For Ctrl+Alt, hold either Ctrl key, then press Alt on the selected side.\n"
                + "AltGr follows your keyboard layout and is not the same as Ctrl+Alt.\n"
                + "Hyper and AltGr require matching keys in your keyboard layout."));
        internal_note.set_halign(Gtk.Align.START);
        internal_note.set_line_wrap(true);
        internal_note.set_xalign(0.0f);
        internal_note.set_margin_start(20);
        hud_page.pack_start(internal_note, false, false, 2);

        var shortcut_warning = new Gtk.Label(dgettext(Config.GETTEXT_PACKAGE,
            "The saved shortcut is not one of these modifiers. Choose a modifier or use MATE Control Center."));
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
        var attach_applet = new Gtk.ToggleButton.with_label(
            dgettext(Config.GETTEXT_PACKAGE, "global menu applet"));
        attach_switch.pack_start(attach_monitor, false, false, 0);
        attach_switch.pack_start(attach_window, false, false, 0);
        attach_switch.pack_start(attach_applet, false, false, 0);
        attach_row.pack_start(attach_label, true, true, 0);
        attach_row.pack_start(attach_switch, false, false, 0);
        placement_group.pack_start(attach_row, false, false, 2);

        bool syncing_attach = false;
        string attach_to = settings.get_string("hud-attach-to");
        syncing_attach = true;
        attach_monitor.active = attach_to == "monitor";
        attach_window.active = attach_to != "monitor" && attach_to != "applet";
        attach_applet.active = attach_to == "applet";
        syncing_attach = false;

        var location_row = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 6);
        location_row.sensitive = attach_to != "applet";

        attach_monitor.clicked.connect(() => {
            if (syncing_attach)
                return;
            syncing_attach = true;
            attach_monitor.active = true;
            attach_window.active = false;
            attach_applet.active = false;
            settings.set_string("hud-attach-to", "monitor");
            location_row.sensitive = true;
            syncing_attach = false;
        });
        attach_window.clicked.connect(() => {
            if (syncing_attach)
                return;
            syncing_attach = true;
            attach_monitor.active = false;
            attach_window.active = true;
            attach_applet.active = false;
            settings.set_string("hud-attach-to", "window");
            location_row.sensitive = true;
            syncing_attach = false;
        });
        attach_applet.clicked.connect(() => {
            if (syncing_attach)
                return;
            syncing_attach = true;
            attach_monitor.active = false;
            attach_window.active = false;
            attach_applet.active = true;
            settings.set_string("hud-attach-to", "applet");
            location_row.sensitive = false;
            syncing_attach = false;
        });
        ulong attach_changed_id = settings.changed["hud-attach-to"].connect(() => {
            if (syncing_attach)
                return;
            string value = settings.get_string("hud-attach-to");
            syncing_attach = true;
            attach_monitor.active = value == "monitor";
            attach_window.active = value != "monitor" && value != "applet";
            attach_applet.active = value == "applet";
            location_row.sensitive = value != "applet";
            syncing_attach = false;
        });

        var location_label = new Gtk.Label(
            dgettext(Config.GETTEXT_PACKAGE, "HUD location:"));
        location_label.set_halign(Gtk.Align.START);
        location_row.pack_start(location_label, true, true, 0);
        placement_group.pack_start(location_row, false, false, 2);

        var location_grid = new Gtk.Grid();
        location_grid.set_row_spacing(4);
        location_grid.set_column_spacing(4);
        location_grid.set_halign(Gtk.Align.START);
        location_grid.set_row_homogeneous(true);
        location_grid.set_column_homogeneous(true);
        location_grid.set_size_request(120, 120);
        location_row.pack_start(location_grid, false, false, 0);

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

        ulong location_changed_id = settings.changed["hud-location"].connect(() => {
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
        attach_applet.show();
        attach_switch.show();
        attach_row.show();
        location_label.show();
        location_grid.show_all();
        location_row.show();
        placement_group.show();

        var hud_tab = new Gtk.Label(dgettext(Config.GETTEXT_PACKAGE, "HUD"));
        hud_tab.set_halign(Gtk.Align.CENTER);
        hud_tab.set_hexpand(true);
        notebook.append_page(hud_page, hud_tab);
        hud_page.show();
        hud_tab.show();

        ulong shortcut_changed_id = settings.changed["hud-shortcut"].connect(() => {
            if (!syncing_shortcut) {
                syncing_shortcut = true;
                string value = settings.get_string("hud-shortcut");
                int selected = shortcut_preset_index(value);
                if (selected != 3)
                    shortcut_right = value.has_suffix("_R");
                populate_shortcut_side(shortcut_side, selected, shortcut_right);
                for (int i = 0; i < shortcut_buttons.length; i++)
                    shortcut_buttons[i].active = selected == i;
                syncing_shortcut = false;
            }
            update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                                   sys_note, internal_note, shortcut_warning,
                                   shortcut_buttons);
        });
        dlg.destroy.connect(() => {
            settings.disconnect(shortcut_changed_id);
            settings.disconnect(attach_changed_id);
            settings.disconnect(location_changed_id);
        });
        hud_enable.toggled.connect(() => {
            update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                                   sys_note, internal_note, shortcut_warning,
                                   shortcut_buttons);
            placement_group.sensitive = hud_enable.active;
        });
        sys_shortcut_check.toggled.connect(() => {
            update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                                   sys_note, internal_note, shortcut_warning,
                                   shortcut_buttons);
        });
        update_hud_shortcut_ui(hud_enable, sys_shortcut_check, shortcut_row,
                               sys_note, internal_note, shortcut_warning,
                               shortcut_buttons);

        notebook.show();
        dlg.show();
        dlg.present();
        dlg.response.connect(()=>{
            dlg.destroy();
        });
    });
    export_preferences_action(a);
    action_group.add_action (a);
    applet.setup_menu("""<menuitem name="Appmenu Preferences Item" action="AppMenuAppletPreferences" />""",action_group);
    return true;
}


public int _mate_panel_applet_shlib_factory()
{
    GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE, Config.LOCALE_DIR);
    GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE, "UTF-8");
    return MatePanel.Applet.factory_setup_in_process("AppmenuAppletFactory", typeof (MatePanel.Applet), factory_callback);
}

//void main(string[] args) {
//    Gtk.init(ref args);
//    MatePanel.Applet.factory_main("SNTrayAppletFactory", true, typeof (MatePanel.Applet), StatusNotifier.factory_callback);
//}
