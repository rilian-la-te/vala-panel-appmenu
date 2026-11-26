// SPDX-License-Identifier: LGPL-3.0-or-later

using GLib;

namespace Appmenu
{
    [CCode (cname = "appmenu_get_applicable_backend")]
    private extern static Backend? register_backend ();
    public class MenuProvider: GLib.Object
    {
        private Backend backend;
        private ulong backend_connector = 0;

        construct {
            backend = register_backend();
            backend_connector = backend.active_model_changed.connect(()=>{
                content_updated();
            });
        }

        public unowned GLib.MenuModel? get_menu()
        {
            if(backend != null && backend.helper  != null && backend.helper.menu  != null)
                return backend.helper.menu;
            return null;
        }
        public unowned GLib.HashTable<unowned string, GLib.ActionGroup?>? get_actions()
        {
            if(backend != null && backend.helper  != null && backend.helper.actions  != null)
                return backend.helper.actions;
            return null;
        }
        public signal void content_updated();
    }
}
