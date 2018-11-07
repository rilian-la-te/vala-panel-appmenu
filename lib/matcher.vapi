/*
 * vala-panel
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
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

[CCode(cheader_filename="matcher.h")]
public class ValaPanel.Matcher : GLib.Object
{
    public Matcher();
    public static string? get_x11_atom_string(long xid, Gdk.Atom atom, bool utf8);
    public static string? get_gtk_application_id(long xid);
    public unowned GLib.DesktopAppInfo match_wnck_window(Wnck.Window? window);
}
