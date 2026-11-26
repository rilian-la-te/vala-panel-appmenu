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
        protected static GLib.MenuModel create_model(GLib.MenuModel? appmenu, GLib.MenuModel? menubar)
        {
            var combined = new GLib.Menu();
            if (appmenu != null)
                combined.append_section(null, appmenu);
            if (menubar != null)
                combined.append_section(null, menubar);
            return combined;
        }

        protected static void merge_actions(GLib.HashTable<unowned string, GLib.ActionGroup> src, GLib.HashTable<unowned string, GLib.ActionGroup> dst)
        {
            src.foreach ((key, val) => {
                dst.insert (key, val);
            });
        }
        public GLib.MenuModel menu;
        public GLib.HashTable<unowned string, GLib.ActionGroup> actions = new HashTable<unowned string, GLib.ActionGroup>(str_hash, str_equal);
        public signal void content_updated();
    }
}
