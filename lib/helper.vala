// SPDX-License-Identifier: LGPL-3.0-or-later

using GLib;

namespace Appmenu
{
    public enum ModelType
    {
        NONE,
        STUB,
        DESKTOP,
        MENUMODEL,
        DBUSMENU
    }
    [CCode (has_target = false)]
    public delegate string? TitleFilter(int64 pid, string? class_name, string? window_name);
    public abstract class Helper: Object
    {
        protected static GLib.MenuModel create_model(GLib.MenuModel? appmenu, GLib.MenuModel? menubar, bool compact_mode)
        {
            var combined = new GLib.Menu();
            if (appmenu != null)
                combined.append_section(null, appmenu);
            if (menubar != null)
                combined.append_section(null, menubar);

            bool has_menubar_items = (menubar != null && menubar.get_n_items() > 0);
            if (compact_mode && has_menubar_items)
            {
                var compact = new GLib.Menu();
                string name = resolve_compact_label(appmenu);
                compact.append_submenu(name, combined);
                return compact;
            }

            return combined;
        }

        private static string resolve_compact_label(GLib.MenuModel? appmenu)
        {
            string? name = null;
            if (appmenu != null && appmenu.get_n_items() > 0)
                appmenu.get_item_attribute(0, "label", "s", &name);

            return name ?? GLib.dgettext(Config.GETTEXT_PACKAGE, "Compact Menu");
        }

        protected GLib.MenuModel menu;
        protected GLib.HashTable<unowned string, GLib.ActionGroup> actions;
    }
}
