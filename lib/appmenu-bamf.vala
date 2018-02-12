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
using Gtk;

namespace Appmenu
{
    DBusMenuHelper get_dbus_menu_helper_with_bamf(MenuWidget w, string name, ObjectPath path, Bamf.Application? app)
    {
        string? title = null;
        DesktopAppInfo? info = null;
        if (app != null)
        {
            var desktop_file = app.get_desktop_file();
            if (desktop_file != null)
            {
                info = new DesktopAppInfo.from_filename(desktop_file);
                title = info.get_name();
            }
        }
        if (title == null && app != null)
            title = app.get_name();
        return new DBusMenuHelper(w,name,path,title,info);
    }
    MenuModelHelper get_menu_model_helper_with_bamf(MenuWidget w, Bamf.Window window, Bamf.Application? app)
    {
        var gtk_unique_bus_name = window.get_utf8_prop("_GTK_UNIQUE_BUS_NAME");
        var app_menu_path = window.get_utf8_prop("_GTK_APP_MENU_OBJECT_PATH");
        var menubar_path = window.get_utf8_prop("_GTK_MENUBAR_OBJECT_PATH");
        var application_path = window.get_utf8_prop("_GTK_APPLICATION_OBJECT_PATH");
        var window_path = window.get_utf8_prop("_GTK_WINDOW_OBJECT_PATH");
        var unity_path = window.get_utf8_prop("_UNITY_OBJECT_PATH");
        DesktopAppInfo? info = null;
        string? title = null;
        if (app != null)
        {
            var desktop_file = app.get_desktop_file();
            if (desktop_file != null)
            {
                info = new DesktopAppInfo.from_filename(desktop_file);
                title = info.get_name();
            }
        }
        if (title == null && app != null)
            title = app.get_name();
        if (title == null)
            title = window.get_name();
        return new MenuModelHelper(w,gtk_unique_bus_name,app_menu_path,menubar_path,application_path,window_path,unity_path,title,info);
    }
    DBusAppMenu get_stub_helper_with_bamf(MenuWidget w, Bamf.Application app)
    {
        DesktopAppInfo? info = null;
        string? title = null;
        var desktop_file = app.get_desktop_file();
        if (desktop_file != null)
        {
            info = new DesktopAppInfo.from_filename(desktop_file);
            title = info.get_name();
        }
        if (title == null)
            title = app.get_name();
        return new DBusAppMenu(w,title,null,info);
    }
}
