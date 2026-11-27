// SPDX-License-Identifier: LGPL-3.0-or-later

namespace Xcb {
	[CCode (cheader_filename = "xcb-aux.h", cname = "XcbAux")]
	[Compact]
	public class Aux {
		[CCode (has_construct_function = false, cname = "vala_panel_xcb_aux_connect")]
		public static unowned Aux? connect();

		[CCode (cname = "vala_panel_xcb_aux_unref")]
		public void unref();

		[CCode (cname = "vala_panel_xcb_aux_get_utf8_prop")]
		public string? get_utf8_prop(uint window, string prop);

		[CCode (cname = "vala_panel_xcb_aux_get_active_window")]
		public uint get_active_window();

		[CCode (cname = "vala_panel_xcb_aux_get_transient_for")]
		public uint get_transient_for(uint window);

		[CCode (cname = "vala_panel_xcb_aux_get_parent")]
		public uint get_parent(uint window);

		[CCode (cname = "vala_panel_xcb_aux_get_name")]
		public string? get_name(uint window);

		[CCode (cname = "vala_panel_xcb_aux_get_pid")]
		public int64 get_pid(uint window);

		[CCode (cname = "vala_panel_xcb_aux_get_wm_class")]
		public void get_wm_class(uint window, out string? instance, out string? group);

		[CCode (cname = "vala_panel_xcb_aux_is_desktop_window")]
		public bool is_desktop_window(uint window);

		[CCode (cname = "vala_panel_xcb_aux_match_app", instance_pos = 1.5)]
		public GLib.DesktopAppInfo? match_app(ValaPanel.Matcher matcher, uint window);

		[CCode (cname = "vala_panel_xcb_aux_add_listener")]
		public void add_listener(GLib.Object listener);

		[CCode (cname = "vala_panel_xcb_aux_remove_listener")]
		public static void remove_listener(GLib.Object? listener);

		[CCode (cname = "vala_panel_xcb_aux_watch_root")]
		public void watch_root();
	}

	[CCode (cname = "is_applicable")]
	public bool is_applicable();
}