/*
 * vala-panel
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

using GLib;

[CCode (cprefix="")]
namespace Launcher
{
    [CCode (cheader_filename="launcher.h",cname="menu_launch_id")]
    public static void activate_menu_launch_id(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="launcher.h",cname="menu_launch_uri")]
    public static void activate_menu_launch_uri(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="launcher.h",cname="menu_launch_command")]
    public static void activate_menu_launch_command(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cname="posix_get_cmdline_string",cheader_filename="launcher.h")]
    public static string? posix_get_cmdline_string(string filename);
}
[CCode (cprefix="")]
namespace MenuMaker
{
    [CCode (cheader_filename="launcher.h",cname="vala_panel_launch_with_context")]
    public static bool launch_with_context(DesktopAppInfo info, GLib.List<string>? uris, GLib.AppLaunchContext? cxt);
    [CCode (cheader_filename="launcher.h",cname="vala_panel_get_default_for_uri")]
    public static AppInfo get_default_for_uri(string uri);
}

[CCode(cheader_filename="matcher.h")]
public class ValaPanel.Matcher : GLib.Object
{
    [CCode (has_construct_function = false)]
    private Matcher();
    public static Matcher @get();
    public unowned GLib.DesktopAppInfo match_arbitrary(string class, string group, string gtk_id, int pid);
}

[CCode(cheader_filename="helper.h")]
public enum Appmenu.ModelType
{
    NONE,
    STUB,
    DESKTOP,
    MENUMODEL,
    DBUSMENU
}

[CCode(cheader_filename="helper.h", type_id = "appmenu_helper_get_type()")]
public abstract class Appmenu.Helper: Object
{
    protected static GLib.MenuModel create_model(GLib.MenuModel? appmenu, GLib.MenuModel? menubar);
    protected static void merge_actions(GLib.HashTable<unowned string, GLib.ActionGroup> src, GLib.HashTable<unowned string, GLib.ActionGroup> dst);
    [CCode (has_construct_function = false)]
    protected Helper();
    public GLib.MenuModel menu;
    public GLib.HashTable<unowned string, GLib.ActionGroup> actions;
    public signal void content_updated();
}
