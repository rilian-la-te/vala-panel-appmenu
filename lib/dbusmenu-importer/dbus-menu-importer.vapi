/*
 * vala-panel-appmenu
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

using GLib;

namespace DBusMenu
{
	[CCode(cheader_filename = "dbusmenu-importer/importer.h")]
	public class Importer : Object
	{
		[NoAccessorMethod]
		public string bus_name {construct;}
		[NoAccessorMethod]
		public string object_path {construct;}
		[NoAccessorMethod]
		public GLib.MenuModel model {owned get;}
		[NoAccessorMethod]
		public GLib.ActionGroup action_group {owned get;}
		public Importer(string bus_name, string object_path);
		public bool check();
	}
}

