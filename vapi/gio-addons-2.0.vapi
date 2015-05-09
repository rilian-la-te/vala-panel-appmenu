/*
 * vala-panel
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

[CCode (cname = "volatile int")]
public struct Volatile : int {
}

namespace Posix
{
    [CCode (cname = "getcwd", cheader_filename = "unistd.h")]
    public long getcwd(char[] cwd);
}
[CCode (cprefix = "G", lower_case_cprefix = "g_")]
namespace GLib
{
    [CCode (cname = "g_slist_free_full")]
    public void slist_free_full(SList list, DestroyNotify? free_func);
    namespace SignalHandler
    {
        [CCode (cname = "g_signal_handlers_disconnect_by_data")]
        public void disconnect_by_data(Object instance, void* data);
    }
    [CCode (cname = "GSettingsBackend")]
    public class KeyfileSettingsBackend : GLib.SettingsBackend
    {
        [CCode (cname = "g_keyfile_settings_backend_new", cheader_filename = "gio/gsettingsbackend.h")]
        public KeyfileSettingsBackend(string filename, string root_path, string? root_group);
    }
    [CCode (cname = "GTask",cheader_filename = "gio/gio.h", type_id = "g_task_get_type ()")]
    public class AsyncTask : GLib.Task
    {
        [CCode (cname = "g_task_new")]
        public static AsyncTask create (GLib.Object? source_object, GLib.Cancellable? cancellable = null, AsyncReadyCallback? back = null);
        [CCode (has_target = false)]
        public delegate void TaskThreadFunc (Task task, Object source, void* task_data, Cancellable cancellable);
        [CCode (cname = "g_task_run_in_thread")]
        public void run_in_thread(TaskThreadFunc func);
    }
}
