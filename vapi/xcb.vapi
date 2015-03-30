/*
 * Copyright (C) 2012-2014  Nicolas Bruguier
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the names of the authors or their
 * institutions shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the authors.
 */

using Xcb;

[CCode (cheader_filename="xcb/xcb.h,xcb/xproto.h")]
namespace Xcb
{
    [Compact, CCode (cname = "xcb_connection_t", free_function = "xcb_disconnect")]
    public class Connection : Xcb.BaseConnection {
        [CCode (cname = "xcb_connect")]
        public Connection (string? displayname = null, out int screen = null);

        /**
         * Get atom identifier by name
         *
         * Retrieves the identifier (xcb_atom_t TODO) for the atom with the specified
         * name. Atoms are used in protocols like EWMH, for example to store window titles
         * (`_NET_WM_NAME` atom) as property of a window.
         *
         * If `only_if_exists` is 0, the atom will be created if it does not already exist.
         * If `only_if_exists` is 1, `XCB_ATOM_NONE` will be returned if the atom does
         * not yet exist.
         *
         * = Errors: =
         *
         *  * {@link AllocError}:  TODO: reasons?
         *
         * @param only_if_exists Return a valid atom id only if the atom already exists.
         * @param name The name of the atom.
         *
         * @see Atom.get_name
         */
        [CCode (cname = "xcb_intern_atom")]
        public InternAtomCookie intern_atom (bool only_if_exists, [CCode (array_length_pos = 1.2)]char[]? name);
        /**
         * release the pointer
         *
         * Releases the pointer and any queued events if you actively grabbed the pointer
         * before using `xcb_grab_pointer`, `xcb_grab_button` or within a normal button
         * press.
         *
         * EnterNotify and LeaveNotify events are generated.
         *
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The pointer will not be released if `time` is earlier than the
         *             last-pointer-grab time or later than the current X server time.
         *
         * @see Window.grab_pointer
         * @see Window.grab_button
         * @see EnterNotifyEvent
         * @see LeaveNotifyEvent
         */
        [CCode (cname = "xcb_ungrab_pointer")]
        public VoidCookie ungrab_pointer (Timestamp time);
        /**
         * release the pointer
         *
         * Releases the pointer and any queued events if you actively grabbed the pointer
         * before using `xcb_grab_pointer`, `xcb_grab_button` or within a normal button
         * press.
         *
         * EnterNotify and LeaveNotify events are generated.
         *
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The pointer will not be released if `time` is earlier than the
         *             last-pointer-grab time or later than the current X server time.
         *
         * @see Window.grab_pointer
         * @see Window.grab_button
         * @see EnterNotifyEvent
         * @see LeaveNotifyEvent
         */
        [CCode (cname = "xcb_ungrab_pointer_checked")]
        public VoidCookie ungrab_pointer_checked (Timestamp time);
        [CCode (cname = "xcb_ungrab_keyboard")]
        public VoidCookie ungrab_keyboard (Timestamp time);
        [CCode (cname = "xcb_ungrab_keyboard_checked")]
        public VoidCookie ungrab_keyboard_checked (Timestamp time);
        /**
         * release queued events
         *
         * Releases queued events if the client has caused a device (pointer/keyboard) to
         * freeze due to grabbing it actively. This request has no effect if `time` is
         * earlier than the last-grab time of the most recent active grab for this client
         * or if `time` is later than the current X server time.
         *
         * = Errors: =
         *
         *  * {@link ValueError}:  You specified an invalid `mode`.
         *
         * @param mode mode
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The special value `XCB_CURRENT_TIME` will be replaced with the current server
         *             time.
         */
        [CCode (cname = "xcb_allow_events")]
        public VoidCookie allow_events (Allow mode, Timestamp time);
        /**
         * release queued events
         *
         * Releases queued events if the client has caused a device (pointer/keyboard) to
         * freeze due to grabbing it actively. This request has no effect if `time` is
         * earlier than the last-grab time of the most recent active grab for this client
         * or if `time` is later than the current X server time.
         *
         * = Errors: =
         *
         *  * {@link ValueError}:  You specified an invalid `mode`.
         *
         * @param mode mode
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The special value `XCB_CURRENT_TIME` will be replaced with the current server
         *             time.
         */
        [CCode (cname = "xcb_allow_events_checked")]
        public VoidCookie allow_events_checked (Allow mode, Timestamp time);
        [CCode (cname = "xcb_grab_server")]
        public VoidCookie grab_server ();
        [CCode (cname = "xcb_grab_server_checked")]
        public VoidCookie grab_server_checked ();
        [CCode (cname = "xcb_ungrab_server")]
        public VoidCookie ungrab_server ();
        [CCode (cname = "xcb_ungrab_server_checked")]
        public VoidCookie ungrab_server_checked ();
        [CCode (cname = "xcb_get_input_focus")]
        public GetInputFocusCookie get_input_focus ();
        [CCode (cname = "xcb_query_keymap")]
        public QueryKeymapCookie query_keymap ();
        /**
         * get matching font names
         *
         * Gets a list of available font names which match the given `pattern`.
         *
         * @param max_names The maximum number of fonts to be returned.
         * @param pattern A font pattern, for example "-misc-fixed-*".
         *                The asterisk (*) is a wildcard for any number of characters. The question mark
         *                (?) is a wildcard for a single character. Use of uppercase or lowercase does
         *                not matter.
         */
        [CCode (cname = "xcb_list_fonts")]
        public ListFontsCookie list_fonts (uint16 max_names, [CCode (array_length_pos = 1.2)]char[]? pattern);
        /**
         * get matching font names and information
         *
         * Gets a list of available font names which match the given `pattern`.
         *
         * @param max_names The maximum number of fonts to be returned.
         * @param pattern A font pattern, for example "-misc-fixed-*".
         *                The asterisk (*) is a wildcard for any number of characters. The question mark
         *                (?) is a wildcard for a single character. Use of uppercase or lowercase does
         *                not matter.
         */
        [CCode (cname = "xcb_list_fonts_with_info")]
        public ListFontsWithInfoCookie list_fonts_with_info (uint16 max_names, [CCode (array_length_pos = 1.2)]char[]? pattern);
        [CCode (cname = "xcb_set_font_path")]
        public VoidCookie set_font_path ([CCode (array_length_pos = 0.1)]Str[]? font);
        [CCode (cname = "xcb_set_font_path_checked")]
        public VoidCookie set_font_path_checked ([CCode (array_length_pos = 0.1)]Str[]? font);
        [CCode (cname = "xcb_get_font_path")]
        public GetFontPathCookie get_font_path ();
        /**
         * check if extension is present
         *
         * Determines if the specified extension is present on this X11 server.
         *
         * Every extension has a unique `major_opcode` to identify requests, the minor
         * opcodes and request formats are extension-specific. If the extension provides
         * events and errors, the `first_event` and `first_error` fields in the reply are
         * set accordingly.
         *
         * There should rarely be a need to use this request directly, XCB provides the
         * `xcb_get_extension_data` function instead.
         *
         * @param name The name of the extension to query, for example "RANDR". This is case
         *             sensitive!
         *
         */
        [CCode (cname = "xcb_query_extension")]
        public QueryExtensionCookie query_extension ([CCode (array_length_pos = 0.1)]char[]? name);
        [CCode (cname = "xcb_list_extensions")]
        public ListExtensionsCookie list_extensions ();
        [CCode (cname = "xcb_change_keyboard_mapping")]
        public VoidCookie change_keyboard_mapping (Keycode first_keycode, uint8 keysyms_per_keycode, [CCode (array_length_pos = 0.1)]Keysym[]? keysyms);
        [CCode (cname = "xcb_change_keyboard_mapping_checked")]
        public VoidCookie change_keyboard_mapping_checked (Keycode first_keycode, uint8 keysyms_per_keycode, [CCode (array_length_pos = 0.1)]Keysym[]? keysyms);
        [CCode (cname = "xcb_get_keyboard_mapping")]
        public GetKeyboardMappingCookie get_keyboard_mapping (Keycode first_keycode, uint8 count);
        [CCode (cname = "xcb_change_keyboard_control")]
        public VoidCookie change_keyboard_control (uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        [CCode (cname = "xcb_change_keyboard_control_checked")]
        public VoidCookie change_keyboard_control_checked (uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        [CCode (cname = "xcb_get_keyboard_control")]
        public GetKeyboardControlCookie get_keyboard_control ();
        [CCode (cname = "xcb_bell")]
        public VoidCookie bell (int8 percent);
        [CCode (cname = "xcb_bell_checked")]
        public VoidCookie bell_checked (int8 percent);
        [CCode (cname = "xcb_change_pointer_control")]
        public VoidCookie change_pointer_control (int16 acceleration_numerator, int16 acceleration_denominator, int16 threshold, bool do_acceleration, bool do_threshold);
        [CCode (cname = "xcb_change_pointer_control_checked")]
        public VoidCookie change_pointer_control_checked (int16 acceleration_numerator, int16 acceleration_denominator, int16 threshold, bool do_acceleration, bool do_threshold);
        [CCode (cname = "xcb_get_pointer_control")]
        public GetPointerControlCookie get_pointer_control ();
        [CCode (cname = "xcb_set_screen_saver")]
        public VoidCookie set_screen_saver (int16 timeout, int16 interval, Blanking prefer_blanking, Exposures allow_exposures);
        [CCode (cname = "xcb_set_screen_saver_checked")]
        public VoidCookie set_screen_saver_checked (int16 timeout, int16 interval, Blanking prefer_blanking, Exposures allow_exposures);
        [CCode (cname = "xcb_get_screen_saver")]
        public GetScreenSaverCookie get_screen_saver ();
        [CCode (cname = "xcb_change_hosts")]
        public VoidCookie change_hosts (HostMode mode, Family family, [CCode (array_length_pos = 2.3)]uint8[]? address);
        [CCode (cname = "xcb_change_hosts_checked")]
        public VoidCookie change_hosts_checked (HostMode mode, Family family, [CCode (array_length_pos = 2.3)]uint8[]? address);
        [CCode (cname = "xcb_list_hosts")]
        public ListHostsCookie list_hosts ();
        [CCode (cname = "xcb_set_access_control")]
        public VoidCookie set_access_control (AccessControl mode);
        [CCode (cname = "xcb_set_access_control_checked")]
        public VoidCookie set_access_control_checked (AccessControl mode);
        [CCode (cname = "xcb_set_close_down_mode")]
        public VoidCookie set_close_down_mode (CloseDown mode);
        [CCode (cname = "xcb_set_close_down_mode_checked")]
        public VoidCookie set_close_down_mode_checked (CloseDown mode);
        /**
         * kills a client
         *
         * Forces a close down of the client that created the specified `resource`.
         *
         * = Errors: =
         *
         *  * {@link ValueError}:  The specified `resource` does not exist.
         *
         * @param resource Any resource belonging to the client (for example a Window), used to identify
         *                 the client connection.
         *                 The special value of `XCB_KILL_ALL_TEMPORARY`, the resources of all clients
         *                 that have terminated in `RetainTemporary` (TODO) are destroyed.
         *
         */
        [CCode (cname = "xcb_kill_client")]
        public VoidCookie kill_client (uint32 resource);
        /**
         * kills a client
         *
         * Forces a close down of the client that created the specified `resource`.
         *
         * = Errors: =
         *
         *  * {@link ValueError}:  The specified `resource` does not exist.
         *
         * @param resource Any resource belonging to the client (for example a Window), used to identify
         *                 the client connection.
         *                 The special value of `XCB_KILL_ALL_TEMPORARY`, the resources of all clients
         *                 that have terminated in `RetainTemporary` (TODO) are destroyed.
         *
         */
        [CCode (cname = "xcb_kill_client_checked")]
        public VoidCookie kill_client_checked (uint32 resource);
        [CCode (cname = "xcb_force_screen_saver")]
        public VoidCookie force_screen_saver (ScreenSaverType mode);
        [CCode (cname = "xcb_force_screen_saver_checked")]
        public VoidCookie force_screen_saver_checked (ScreenSaverType mode);
        [CCode (cname = "xcb_set_pointer_mapping")]
        public SetPointerMappingCookie set_pointer_mapping ([CCode (array_length_pos = 0.1)]uint8[]? map);
        [CCode (cname = "xcb_get_pointer_mapping")]
        public GetPointerMappingCookie get_pointer_mapping ();
        [CCode (cname = "xcb_set_modifier_mapping")]
        public SetModifierMappingCookie set_modifier_mapping ([CCode (array_length_pos = 0.1)]Keycode[]? keycodes);
        [CCode (cname = "xcb_get_modifier_mapping")]
        public GetModifierMappingCookie get_modifier_mapping ();
        [CCode (cname = "xcb_no_operation")]
        public VoidCookie no_operation ();
        [CCode (cname = "xcb_no_operation_checked")]
        public VoidCookie no_operation_checked ();
    }

    [Compact, CCode (cname = "xcb_intern_atom_reply_t", free_function = "free")]
    public class InternAtomReply {
        public Atom atom;
    }

    [SimpleType, CCode (cname = "xcb_intern_atom_cookie_t")]
    public struct InternAtomCookie : VoidCookie {
        [CCode (cname = "xcb_intern_atom_reply", instance_pos = 1.1)]
        public InternAtomReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_input_focus_reply_t", free_function = "free")]
    public class GetInputFocusReply {
        public InputFocus revert_to;
        public Window focus;
    }

    [SimpleType, CCode (cname = "xcb_get_input_focus_cookie_t")]
    public struct GetInputFocusCookie : VoidCookie {
        [CCode (cname = "xcb_get_input_focus_reply", instance_pos = 1.1)]
        public GetInputFocusReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_query_keymap_reply_t", free_function = "free")]
    public class QueryKeymapReply {
        public uint8 keys[32];
    }

    [SimpleType, CCode (cname = "xcb_query_keymap_cookie_t")]
    public struct QueryKeymapCookie : VoidCookie {
        [CCode (cname = "xcb_query_keymap_reply", instance_pos = 1.1)]
        public QueryKeymapReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_list_fonts_reply_t", free_function = "free")]
    public class ListFontsReply {
        public uint16 names_len;
        [CCode (cname = "xcb_list_fonts_names_iterator")]
        _StrIterator _iterator ();
        public StrIterator iterator () {
            return (StrIterator) _iterator ();
        }
        public int names_length {
            [CCode (cname = "xcb_list_fonts_names_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Str[] names {
            [CCode (cname = "xcb_list_fonts_names")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_list_fonts_cookie_t")]
    public struct ListFontsCookie : VoidCookie {
        [CCode (cname = "xcb_list_fonts_reply", instance_pos = 1.1)]
        public ListFontsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_list_fonts_with_info_reply_t", free_function = "free")]
    public class ListFontsWithInfoReply {
        public uint8 name_len;
        public Charinfo min_bounds;
        public Charinfo max_bounds;
        public uint16 min_char_or_byte2;
        public uint16 max_char_or_byte2;
        public uint16 default_char;
        public uint16 properties_len;
        public FontDraw draw_direction;
        public uint8 min_byte1;
        public uint8 max_byte1;
        public bool all_chars_exist;
        public int16 font_ascent;
        public int16 font_descent;
        public uint32 replies_hint;
        [CCode (cname = "xcb_list_fonts_with_info_properties_iterator")]
        _FontpropIterator _iterator ();
        public FontpropIterator iterator () {
            return (FontpropIterator) _iterator ();
        }
        public int properties_length {
            [CCode (cname = "xcb_list_fonts_with_info_properties_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Fontprop[] properties {
            [CCode (cname = "xcb_list_fonts_with_info_properties")]
            get;
        }
        [CCode (cname = "xcb_list_fonts_with_info_name_length")]
        int _name_length ();
        [CCode (cname = "xcb_list_fonts_with_info_name", array_length = false)]
        unowned char[] _name ();
        public string name {
            owned get {
                GLib.StringBuilder ret = new GLib.StringBuilder ();
                ret.append_len ((string)_name (), _name_length ());
                return ret.str;
            }
        }
    }

    [SimpleType, CCode (cname = "xcb_list_fonts_with_info_cookie_t")]
    public struct ListFontsWithInfoCookie : VoidCookie {
        [CCode (cname = "xcb_list_fonts_with_info_reply", instance_pos = 1.1)]
        public ListFontsWithInfoReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_font_path_reply_t", free_function = "free")]
    public class GetFontPathReply {
        public uint16 path_len;
        [CCode (cname = "xcb_get_font_path_path_iterator")]
        _StrIterator _iterator ();
        public StrIterator iterator () {
            return (StrIterator) _iterator ();
        }
        public int path_length {
            [CCode (cname = "xcb_get_font_path_path_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Str[] path {
            [CCode (cname = "xcb_get_font_path_path")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_get_font_path_cookie_t")]
    public struct GetFontPathCookie : VoidCookie {
        [CCode (cname = "xcb_get_font_path_reply", instance_pos = 1.1)]
        public GetFontPathReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_query_extension_reply_t", free_function = "free")]
    public class QueryExtensionReply {
        public bool present;
        public uint8 major_opcode;
        public uint8 first_event;
        public uint8 first_error;
    }

    [SimpleType, CCode (cname = "xcb_query_extension_cookie_t")]
    public struct QueryExtensionCookie : VoidCookie {
        [CCode (cname = "xcb_query_extension_reply", instance_pos = 1.1)]
        public QueryExtensionReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_list_extensions_reply_t", free_function = "free")]
    public class ListExtensionsReply {
        public uint8 names_len;
        [CCode (cname = "xcb_list_extensions_names_iterator")]
        _StrIterator _iterator ();
        public StrIterator iterator () {
            return (StrIterator) _iterator ();
        }
        public int names_length {
            [CCode (cname = "xcb_list_extensions_names_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Str[] names {
            [CCode (cname = "xcb_list_extensions_names")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_list_extensions_cookie_t")]
    public struct ListExtensionsCookie : VoidCookie {
        [CCode (cname = "xcb_list_extensions_reply", instance_pos = 1.1)]
        public ListExtensionsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_keyboard_mapping_reply_t", free_function = "free")]
    public class GetKeyboardMappingReply {
        public uint8 keysyms_per_keycode;
        public int keysyms_length {
            [CCode (cname = "xcb_get_keyboard_mapping_keysyms_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Keysym[] keysyms {
            [CCode (cname = "xcb_get_keyboard_mapping_keysyms")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_get_keyboard_mapping_cookie_t")]
    public struct GetKeyboardMappingCookie : VoidCookie {
        [CCode (cname = "xcb_get_keyboard_mapping_reply", instance_pos = 1.1)]
        public GetKeyboardMappingReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_keyboard_control_reply_t", free_function = "free")]
    public class GetKeyboardControlReply {
        public AutoRepeatMode global_auto_repeat;
        public uint32 led_mask;
        public uint8 key_click_percent;
        public uint8 bell_percent;
        public uint16 bell_pitch;
        public uint16 bell_duration;
        public uint8 auto_repeats[32];
    }

    [SimpleType, CCode (cname = "xcb_get_keyboard_control_cookie_t")]
    public struct GetKeyboardControlCookie : VoidCookie {
        [CCode (cname = "xcb_get_keyboard_control_reply", instance_pos = 1.1)]
        public GetKeyboardControlReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_pointer_control_reply_t", free_function = "free")]
    public class GetPointerControlReply {
        public uint16 acceleration_numerator;
        public uint16 acceleration_denominator;
        public uint16 threshold;
    }

    [SimpleType, CCode (cname = "xcb_get_pointer_control_cookie_t")]
    public struct GetPointerControlCookie : VoidCookie {
        [CCode (cname = "xcb_get_pointer_control_reply", instance_pos = 1.1)]
        public GetPointerControlReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_screen_saver_reply_t", free_function = "free")]
    public class GetScreenSaverReply {
        public uint16 timeout;
        public uint16 interval;
        public Blanking prefer_blanking;
        public Exposures allow_exposures;
    }

    [SimpleType, CCode (cname = "xcb_get_screen_saver_cookie_t")]
    public struct GetScreenSaverCookie : VoidCookie {
        [CCode (cname = "xcb_get_screen_saver_reply", instance_pos = 1.1)]
        public GetScreenSaverReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_list_hosts_reply_t", free_function = "free")]
    public class ListHostsReply {
        public AccessControl mode;
        public uint16 hosts_len;
        [CCode (cname = "xcb_list_hosts_hosts_iterator")]
        _HostIterator _iterator ();
        public HostIterator iterator () {
            return (HostIterator) _iterator ();
        }
        public int hosts_length {
            [CCode (cname = "xcb_list_hosts_hosts_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Host[] hosts {
            [CCode (cname = "xcb_list_hosts_hosts")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_list_hosts_cookie_t")]
    public struct ListHostsCookie : VoidCookie {
        [CCode (cname = "xcb_list_hosts_reply", instance_pos = 1.1)]
        public ListHostsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_set_pointer_mapping_reply_t", free_function = "free")]
    public class SetPointerMappingReply {
        public MappingStatus status;
    }

    [SimpleType, CCode (cname = "xcb_set_pointer_mapping_cookie_t")]
    public struct SetPointerMappingCookie : VoidCookie {
        [CCode (cname = "xcb_set_pointer_mapping_reply", instance_pos = 1.1)]
        public SetPointerMappingReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_pointer_mapping_reply_t", free_function = "free")]
    public class GetPointerMappingReply {
        public uint8 map_len;
        public int map_length {
            [CCode (cname = "xcb_get_pointer_mapping_map_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned uint8[] map {
            [CCode (cname = "xcb_get_pointer_mapping_map")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_get_pointer_mapping_cookie_t")]
    public struct GetPointerMappingCookie : VoidCookie {
        [CCode (cname = "xcb_get_pointer_mapping_reply", instance_pos = 1.1)]
        public GetPointerMappingReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_set_modifier_mapping_reply_t", free_function = "free")]
    public class SetModifierMappingReply {
        public MappingStatus status;
    }

    [SimpleType, CCode (cname = "xcb_set_modifier_mapping_cookie_t")]
    public struct SetModifierMappingCookie : VoidCookie {
        [CCode (cname = "xcb_set_modifier_mapping_reply", instance_pos = 1.1)]
        public SetModifierMappingReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_modifier_mapping_reply_t", free_function = "free")]
    public class GetModifierMappingReply {
        public uint8 keycodes_per_modifier;
        public int keycodes_length {
            [CCode (cname = "xcb_get_modifier_mapping_keycodes_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Keycode[] keycodes {
            [CCode (cname = "xcb_get_modifier_mapping_keycodes")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_get_modifier_mapping_cookie_t")]
    public struct GetModifierMappingCookie : VoidCookie {
        [CCode (cname = "xcb_get_modifier_mapping_reply", instance_pos = 1.1)]
        public GetModifierMappingReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [SimpleType, CCode (cname = "xcb_char2b_iterator_t")]
    struct _Char2bIterator
    {
        int rem;
        int index;
        unowned Char2b? data;
    }

    [CCode (cname = "xcb_char2b_iterator_t")]
    public struct Char2bIterator
    {
        [CCode (cname = "xcb_char2b_next")]
        void _next ();

        public inline unowned Char2b?
        next_value ()
        {
            if (((_Char2bIterator)this).rem > 0)
            {
                unowned Char2b? d = ((_Char2bIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_char2b_t", has_type_id = false)]
    public struct Char2b {
        public uint8 byte1;
        public uint8 byte2;
    }

    [SimpleType, CCode (cname = "xcb_window_iterator_t")]
    struct _WindowIterator
    {
        int rem;
        int index;
        unowned Window? data;
    }

    [CCode (cname = "xcb_window_iterator_t")]
    public struct WindowIterator
    {
        [CCode (cname = "xcb_window_next")]
        void _next ();

        public inline unowned Window?
        next_value ()
        {
            if (((_WindowIterator)this).rem > 0)
            {
                unowned Window? d = ((_WindowIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [CCode (cname = "xcb_window_t", has_type_id = false)]
    public struct Window : Drawable {
        /**
         * Allocates an XID for a new Window.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_generate_id")]
        public Window (Xcb.Connection connection);

        /**
         * Creates a window
         *
         * Creates an unmapped window as child of the specified `parent` window. A
         * CreateNotify event will be generated. The new window is placed on top in the
         * stacking order with respect to siblings.
         *
         * The coordinate system has the X axis horizontal and the Y axis vertical with
         * the origin [0, 0] at the upper-left corner. Coordinates are integral, in terms
         * of pixels, and coincide with pixel centers. Each window and pixmap has its own
         * coordinate system. For a window, the origin is inside the border at the inside,
         * upper-left corner.
         *
         * The created window is not yet displayed (mapped), call `xcb_map_window` to
         * display it.
         *
         * The created window will initially use the same cursor as its parent.
         *
         * = Errors: =
         *
         *  * {@link ColormapError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param depth Specifies the new window's depth (TODO: what unit?).
         *              The special value `XCB_COPY_FROM_PARENT` means the depth is taken from the
         *              `parent` window.
         * @param parent The parent window of the new window.
         * @param x The X coordinate of the new window.
         * @param y The Y coordinate of the new window.
         * @param width The width of the new window.
         * @param height The height of the new window.
         * @param border_width TODO:
         *                     Must be zero if the `class` is `InputOnly` or a `xcb_match_error_t` occurs.
         * @param _class _class
         * @param visual Specifies the id for the new window's visual.
         *               The special value `XCB_COPY_FROM_PARENT` means the visual is taken from the
         *               `parent` window.
         *
         * @see Window.map
         * @see CreateNotifyEvent
         */
        [CCode (cname = "xcb_create_window", instance_pos = 2.2)]
        public VoidCookie create (Xcb.Connection connection, uint8 depth, Window parent, int16 x, int16 y, uint16 width, uint16 height, uint16 border_width, WindowClass _class, Visualid visual, uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * Creates a window
         *
         * Creates an unmapped window as child of the specified `parent` window. A
         * CreateNotify event will be generated. The new window is placed on top in the
         * stacking order with respect to siblings.
         *
         * The coordinate system has the X axis horizontal and the Y axis vertical with
         * the origin [0, 0] at the upper-left corner. Coordinates are integral, in terms
         * of pixels, and coincide with pixel centers. Each window and pixmap has its own
         * coordinate system. For a window, the origin is inside the border at the inside,
         * upper-left corner.
         *
         * The created window is not yet displayed (mapped), call `xcb_map_window` to
         * display it.
         *
         * The created window will initially use the same cursor as its parent.
         *
         * = Errors: =
         *
         *  * {@link ColormapError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param depth Specifies the new window's depth (TODO: what unit?).
         *              The special value `XCB_COPY_FROM_PARENT` means the depth is taken from the
         *              `parent` window.
         * @param parent The parent window of the new window.
         * @param x The X coordinate of the new window.
         * @param y The Y coordinate of the new window.
         * @param width The width of the new window.
         * @param height The height of the new window.
         * @param border_width TODO:
         *                     Must be zero if the `class` is `InputOnly` or a `xcb_match_error_t` occurs.
         * @param _class _class
         * @param visual Specifies the id for the new window's visual.
         *               The special value `XCB_COPY_FROM_PARENT` means the visual is taken from the
         *               `parent` window.
         *
         * @see Window.map
         * @see CreateNotifyEvent
         */
        [CCode (cname = "xcb_create_window_checked", instance_pos = 2.2)]
        public VoidCookie create_checked (Xcb.Connection connection, uint8 depth, Window parent, int16 x, int16 y, uint16 width, uint16 height, uint16 border_width, WindowClass _class, Visualid visual, uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * change window attributes
         *
         * Changes the attributes specified by `value_mask` for the specified `window`.
         *
         * = Errors: =
         *
         *  * {@link AccessError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param value_mask value_mask
         * @param value_list Values for each of the attributes specified in the bitmask `value_mask`. The
         *                   order has to correspond to the order of possible `value_mask` bits. See the
         *                   example.
         */
        [CCode (cname = "xcb_change_window_attributes", instance_pos = 1.1)]
        public VoidCookie change_attributes (Xcb.Connection connection, uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * change window attributes
         *
         * Changes the attributes specified by `value_mask` for the specified `window`.
         *
         * = Errors: =
         *
         *  * {@link AccessError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param value_mask value_mask
         * @param value_list Values for each of the attributes specified in the bitmask `value_mask`. The
         *                   order has to correspond to the order of possible `value_mask` bits. See the
         *                   example.
         */
        [CCode (cname = "xcb_change_window_attributes_checked", instance_pos = 1.1)]
        public VoidCookie change_attributes_checked (Xcb.Connection connection, uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * Gets window attributes
         *
         * Gets the current attributes for the specified `window`.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `window` does not exist.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_get_window_attributes", instance_pos = 1.1)]
        public GetWindowAttributesCookie get_attributes (Xcb.Connection connection);
        /**
         * Destroys a window
         *
         * Destroys the specified window and all of its subwindows. A DestroyNotify event
         * is generated for each destroyed window (a DestroyNotify event is first generated
         * for any given window's inferiors). If the window was mapped, it will be
         * automatically unmapped before destroying.
         *
         * Calling DestroyWindow on the root window will do nothing.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified window does not exist.
         *
         * @param connection The connection.
         *
         * @see DestroyNotifyEvent
         * @see Window.map
         * @see Window.unmap
         */
        [CCode (cname = "xcb_destroy_window", instance_pos = 1.1)]
        public VoidCookie destroy (Xcb.Connection connection);
        /**
         * Destroys a window
         *
         * Destroys the specified window and all of its subwindows. A DestroyNotify event
         * is generated for each destroyed window (a DestroyNotify event is first generated
         * for any given window's inferiors). If the window was mapped, it will be
         * automatically unmapped before destroying.
         *
         * Calling DestroyWindow on the root window will do nothing.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified window does not exist.
         *
         * @param connection The connection.
         *
         * @see DestroyNotifyEvent
         * @see Window.map
         * @see Window.unmap
         */
        [CCode (cname = "xcb_destroy_window_checked", instance_pos = 1.1)]
        public VoidCookie destroy_checked (Xcb.Connection connection);
        [CCode (cname = "xcb_destroy_subwindows", instance_pos = 1.1)]
        public VoidCookie destroy_subwindows (Xcb.Connection connection);
        [CCode (cname = "xcb_destroy_subwindows_checked", instance_pos = 1.1)]
        public VoidCookie destroy_subwindows_checked (Xcb.Connection connection);
        /**
         * Changes a client's save set
         *
         * TODO: explain what the save set is for.
         *
         * This function either adds or removes the specified window to the client's (your
         * application's) save set.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  You created the specified window. This does not make sense, you can only add windows created by other clients to your save set.
         *
         * @param connection The connection.
         * @param mode Insert to add the specified window to the save set or Delete to delete it from the save set.
         *
         * @see Window.reparent
         */
        [CCode (cname = "xcb_change_save_set", instance_pos = 2.2)]
        public VoidCookie change_save_set (Xcb.Connection connection, SetMode mode);
        /**
         * Changes a client's save set
         *
         * TODO: explain what the save set is for.
         *
         * This function either adds or removes the specified window to the client's (your
         * application's) save set.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  You created the specified window. This does not make sense, you can only add windows created by other clients to your save set.
         *
         * @param connection The connection.
         * @param mode Insert to add the specified window to the save set or Delete to delete it from the save set.
         *
         * @see Window.reparent
         */
        [CCode (cname = "xcb_change_save_set_checked", instance_pos = 2.2)]
        public VoidCookie change_save_set_checked (Xcb.Connection connection, SetMode mode);
        /**
         * Reparents a window
         *
         * Makes the specified window a child of the specified parent window. If the
         * window is mapped, it will automatically be unmapped before reparenting and
         * re-mapped after reparenting. The window is placed in the stacking order on top
         * with respect to sibling windows.
         *
         * After reparenting, a ReparentNotify event is generated.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  The new parent window is not on the same screen as the old parent window.  The new parent window is the specified window or an inferior of the specified window.  The new parent is InputOnly and the window is not.  The specified window has a ParentRelative background and the new parent window is not the same depth as the specified window.
         *
         * @param connection The connection.
         * @param parent The new parent of the window.
         * @param x The X position of the window within its new parent.
         * @param y The Y position of the window within its new parent.
         *
         * @see ReparentNotifyEvent
         * @see Window.map
         * @see Window.unmap
         */
        [CCode (cname = "xcb_reparent_window", instance_pos = 1.1)]
        public VoidCookie reparent (Xcb.Connection connection, Window parent, int16 x, int16 y);
        /**
         * Reparents a window
         *
         * Makes the specified window a child of the specified parent window. If the
         * window is mapped, it will automatically be unmapped before reparenting and
         * re-mapped after reparenting. The window is placed in the stacking order on top
         * with respect to sibling windows.
         *
         * After reparenting, a ReparentNotify event is generated.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  The new parent window is not on the same screen as the old parent window.  The new parent window is the specified window or an inferior of the specified window.  The new parent is InputOnly and the window is not.  The specified window has a ParentRelative background and the new parent window is not the same depth as the specified window.
         *
         * @param connection The connection.
         * @param parent The new parent of the window.
         * @param x The X position of the window within its new parent.
         * @param y The Y position of the window within its new parent.
         *
         * @see ReparentNotifyEvent
         * @see Window.map
         * @see Window.unmap
         */
        [CCode (cname = "xcb_reparent_window_checked", instance_pos = 1.1)]
        public VoidCookie reparent_checked (Xcb.Connection connection, Window parent, int16 x, int16 y);
        /**
         * Makes a window visible
         *
         * Maps the specified window. This means making the window visible (as long as its
         * parent is visible).
         *
         * This MapWindow request will be translated to a MapRequest request if a window
         * manager is running. The window manager then decides to either map the window or
         * not. Set the override-redirect window attribute to true if you want to bypass
         * this mechanism.
         *
         * If the window manager decides to map the window (or if no window manager is
         * running), a MapNotify event is generated.
         *
         * If the window becomes viewable and no earlier contents for it are remembered,
         * the X server tiles the window with its background. If the window's background
         * is undefined, the existing screen contents are not altered, and the X server
         * generates zero or more Expose events.
         *
         * If the window type is InputOutput, an Expose event will be generated when the
         * window becomes visible. The normal response to an Expose event should be to
         * repaint the window.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  The specified window does not exist.
         *
         * @param connection The connection.
         *
         * @see MapNotifyEvent
         * @see ExposeEvent
         * @see Window.unmap
         */
        [CCode (cname = "xcb_map_window", instance_pos = 1.1)]
        public VoidCookie map (Xcb.Connection connection);
        /**
         * Makes a window visible
         *
         * Maps the specified window. This means making the window visible (as long as its
         * parent is visible).
         *
         * This MapWindow request will be translated to a MapRequest request if a window
         * manager is running. The window manager then decides to either map the window or
         * not. Set the override-redirect window attribute to true if you want to bypass
         * this mechanism.
         *
         * If the window manager decides to map the window (or if no window manager is
         * running), a MapNotify event is generated.
         *
         * If the window becomes viewable and no earlier contents for it are remembered,
         * the X server tiles the window with its background. If the window's background
         * is undefined, the existing screen contents are not altered, and the X server
         * generates zero or more Expose events.
         *
         * If the window type is InputOutput, an Expose event will be generated when the
         * window becomes visible. The normal response to an Expose event should be to
         * repaint the window.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  The specified window does not exist.
         *
         * @param connection The connection.
         *
         * @see MapNotifyEvent
         * @see ExposeEvent
         * @see Window.unmap
         */
        [CCode (cname = "xcb_map_window_checked", instance_pos = 1.1)]
        public VoidCookie map_checked (Xcb.Connection connection);
        [CCode (cname = "xcb_map_subwindows", instance_pos = 1.1)]
        public VoidCookie map_subwindows (Xcb.Connection connection);
        [CCode (cname = "xcb_map_subwindows_checked", instance_pos = 1.1)]
        public VoidCookie map_subwindows_checked (Xcb.Connection connection);
        /**
         * Makes a window invisible
         *
         * Unmaps the specified window. This means making the window invisible (and all
         * its child windows).
         *
         * Unmapping a window leads to the `UnmapNotify` event being generated. Also,
         * `Expose` events are generated for formerly obscured windows.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified window does not exist.
         *
         * @param connection The connection.
         *
         * @see UnmapNotifyEvent
         * @see ExposeEvent
         * @see Window.map
         */
        [CCode (cname = "xcb_unmap_window", instance_pos = 1.1)]
        public VoidCookie unmap (Xcb.Connection connection);
        /**
         * Makes a window invisible
         *
         * Unmaps the specified window. This means making the window invisible (and all
         * its child windows).
         *
         * Unmapping a window leads to the `UnmapNotify` event being generated. Also,
         * `Expose` events are generated for formerly obscured windows.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified window does not exist.
         *
         * @param connection The connection.
         *
         * @see UnmapNotifyEvent
         * @see ExposeEvent
         * @see Window.map
         */
        [CCode (cname = "xcb_unmap_window_checked", instance_pos = 1.1)]
        public VoidCookie unmap_checked (Xcb.Connection connection);
        [CCode (cname = "xcb_unmap_subwindows", instance_pos = 1.1)]
        public VoidCookie unmap_subwindows (Xcb.Connection connection);
        [CCode (cname = "xcb_unmap_subwindows_checked", instance_pos = 1.1)]
        public VoidCookie unmap_subwindows_checked (Xcb.Connection connection);
        /**
         * Configures window attributes
         *
         * Configures a window's size, position, border width and stacking order.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  You specified a Sibling without also specifying StackMode or the window is not actually a Sibling.
         *
         * @param connection The connection.
         * @param value_mask Bitmask of attributes to change.
         * @param value_mask Bitmask of attributes to change.
         * @param value_list New values, corresponding to the attributes in value_mask. The order has to
         *                   correspond to the order of possible `value_mask` bits. See the example.
         *
         * @see MapNotifyEvent
         * @see ExposeEvent
         */
        [CCode (cname = "xcb_configure_window", instance_pos = 1.1)]
        public VoidCookie configure (Xcb.Connection connection, uint16 value_mask, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * Configures window attributes
         *
         * Configures a window's size, position, border width and stacking order.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  You specified a Sibling without also specifying StackMode or the window is not actually a Sibling.
         *
         * @param connection The connection.
         * @param value_mask Bitmask of attributes to change.
         * @param value_mask Bitmask of attributes to change.
         * @param value_list New values, corresponding to the attributes in value_mask. The order has to
         *                   correspond to the order of possible `value_mask` bits. See the example.
         *
         * @see MapNotifyEvent
         * @see ExposeEvent
         */
        [CCode (cname = "xcb_configure_window_checked", instance_pos = 1.1)]
        public VoidCookie configure_checked (Xcb.Connection connection, uint16 value_mask, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * Change window stacking order
         *
         * If `direction` is `XCB_CIRCULATE_RAISE_LOWEST`, the lowest mapped child (if
         * any) will be raised to the top of the stack.
         *
         * If `direction` is `XCB_CIRCULATE_LOWER_HIGHEST`, the highest mapped child will
         * be lowered to the bottom of the stack.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `window` does not exist.
         *
         * @param connection The connection.
         * @param direction direction
         */
        [CCode (cname = "xcb_circulate_window", instance_pos = 2.2)]
        public VoidCookie circulate (Xcb.Connection connection, Circulate direction);
        /**
         * Change window stacking order
         *
         * If `direction` is `XCB_CIRCULATE_RAISE_LOWEST`, the lowest mapped child (if
         * any) will be raised to the top of the stack.
         *
         * If `direction` is `XCB_CIRCULATE_LOWER_HIGHEST`, the highest mapped child will
         * be lowered to the bottom of the stack.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `window` does not exist.
         *
         * @param connection The connection.
         * @param direction direction
         */
        [CCode (cname = "xcb_circulate_window_checked", instance_pos = 2.2)]
        public VoidCookie circulate_checked (Xcb.Connection connection, Circulate direction);
        /**
         * query the window tree
         *
         * Gets the root window ID, parent window ID and list of children windows for the
         * specified `window`. The children are listed in bottom-to-top stacking order.
         *
         * @param connection The connection.
         *
         */
        [CCode (cname = "xcb_query_tree", instance_pos = 1.1)]
        public QueryTreeCookie query_tree (Xcb.Connection connection);
        /**
         * Changes a window property
         *
         * Sets or updates a property on the specified `window`. Properties are for
         * example the window title (`WM_NAME`) or its minimum size (`WM_NORMAL_HINTS`).
         * Protocols such as EWMH also use properties - for example EWMH defines the
         * window title, encoded as UTF-8 string, in the `_NET_WM_NAME` property.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param mode mode
         * @param property The property you want to change (an atom).
         * @param type The type of the property you want to change (an atom).
         * @param format Specifies whether the data should be viewed as a list of 8-bit, 16-bit or
         *               32-bit quantities. Possible values are 8, 16 and 32. This information allows
         *               the X server to correctly perform byte-swap operations as necessary.
         * @param data The property data.
         *
         */
        [CCode (cname = "xcb_change_property", instance_pos = 2.2)]
        public VoidCookie change_property (Xcb.Connection connection, PropMode mode, Atom property, Atom type, uint8 format, [CCode (array_length_pos = 5.6)]uint8[]? data);
        /**
         * Changes a window property
         *
         * Sets or updates a property on the specified `window`. Properties are for
         * example the window title (`WM_NAME`) or its minimum size (`WM_NORMAL_HINTS`).
         * Protocols such as EWMH also use properties - for example EWMH defines the
         * window title, encoded as UTF-8 string, in the `_NET_WM_NAME` property.
         *
         * = Errors: =
         *
         *  * {@link MatchError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param mode mode
         * @param property The property you want to change (an atom).
         * @param type The type of the property you want to change (an atom).
         * @param format Specifies whether the data should be viewed as a list of 8-bit, 16-bit or
         *               32-bit quantities. Possible values are 8, 16 and 32. This information allows
         *               the X server to correctly perform byte-swap operations as necessary.
         * @param data The property data.
         *
         */
        [CCode (cname = "xcb_change_property_checked", instance_pos = 2.2)]
        public VoidCookie change_property_checked (Xcb.Connection connection, PropMode mode, Atom property, Atom type, uint8 format, [CCode (array_length_pos = 5.6)]uint8[]? data);
        [CCode (cname = "xcb_delete_property", instance_pos = 1.1)]
        public VoidCookie delete_property (Xcb.Connection connection, Atom property);
        [CCode (cname = "xcb_delete_property_checked", instance_pos = 1.1)]
        public VoidCookie delete_property_checked (Xcb.Connection connection, Atom property);
        /**
         * Gets a window property
         *
         * Gets the specified `property` from the specified `window`. Properties are for
         * example the window title (`WM_NAME`) or its minimum size (`WM_NORMAL_HINTS`).
         * Protocols such as EWMH also use properties - for example EWMH defines the
         * window title, encoded as UTF-8 string, in the `_NET_WM_NAME` property.
         *
         * TODO: talk about `type`
         *
         * TODO: talk about `delete`
         *
         * TODO: talk about the offset/length thing. what's a valid use case?
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `window` does not exist.
         *
         * @param connection The connection.
         * @param _delete Whether the property should actually be deleted. For deleting a property, the
         *                specified `type` has to match the actual property type.
         * @param property The property you want to get (an atom).
         * @param type The type of the property you want to get (an atom).
         * @param long_offset Specifies the offset (in 32-bit multiples) in the specified property where the
         *                    data is to be retrieved.
         * @param long_length Specifies how many 32-bit multiples of data should be retrieved (e.g. if you
         *                    set `long_length` to 4, you will receive 16 bytes of data).
         *
         */
        [CCode (cname = "xcb_get_property", instance_pos = 2.2)]
        public GetPropertyCookie get_property (Xcb.Connection connection, bool _delete, Atom property, Atom type, uint32 long_offset, uint32 long_length);
        [CCode (cname = "xcb_list_properties", instance_pos = 1.1)]
        public ListPropertiesCookie list_properties (Xcb.Connection connection);
        /**
         * Sets the owner of a selection
         *
         * Makes `window` the owner of the selection `selection` and updates the
         * last-change time of the specified selection.
         *
         * TODO: briefly explain what a selection is.
         *
         * = Errors: =
         *
         *  * {@link AtomError}:  `selection` does not refer to a valid atom.
         *
         * @param connection The connection.
         * @param selection The selection.
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The selection will not be changed if `time` is earlier than the current
         *             last-change time of the `selection` or is later than the current X server time.
         *             Otherwise, the last-change time is set to the specified time.
         *             The special value `XCB_CURRENT_TIME` will be replaced with the current server
         *             time.
         *
         * @see Window.set_selection_owner
         */
        [CCode (cname = "xcb_set_selection_owner", instance_pos = 1.1)]
        public VoidCookie set_selection_owner (Xcb.Connection connection, Atom selection, Timestamp time);
        /**
         * Sets the owner of a selection
         *
         * Makes `window` the owner of the selection `selection` and updates the
         * last-change time of the specified selection.
         *
         * TODO: briefly explain what a selection is.
         *
         * = Errors: =
         *
         *  * {@link AtomError}:  `selection` does not refer to a valid atom.
         *
         * @param connection The connection.
         * @param selection The selection.
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The selection will not be changed if `time` is earlier than the current
         *             last-change time of the `selection` or is later than the current X server time.
         *             Otherwise, the last-change time is set to the specified time.
         *             The special value `XCB_CURRENT_TIME` will be replaced with the current server
         *             time.
         *
         * @see Window.set_selection_owner
         */
        [CCode (cname = "xcb_set_selection_owner_checked", instance_pos = 1.1)]
        public VoidCookie set_selection_owner_checked (Xcb.Connection connection, Atom selection, Timestamp time);
        [CCode (cname = "xcb_convert_selection", instance_pos = 1.1)]
        public VoidCookie convert_selection (Xcb.Connection connection, Atom selection, Atom target, Atom property, Timestamp time);
        [CCode (cname = "xcb_convert_selection_checked", instance_pos = 1.1)]
        public VoidCookie convert_selection_checked (Xcb.Connection connection, Atom selection, Atom target, Atom property, Timestamp time);
        /**
         * send an event
         *
         * Identifies the `destination` window, determines which clients should receive
         * the specified event and ignores any active grabs.
         *
         * The `event` must be one of the core events or an event defined by an extension,
         * so that the X server can correctly byte-swap the contents as necessary. The
         * contents of `event` are otherwise unaltered and unchecked except for the
         * `send_event` field which is forced to 'true'.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `destination` window does not exist.
         *
         * @param connection The connection.
         * @param propagate If `propagate` is true and no clients have selected any event on `destination`,
         *                  the destination is replaced with the closest ancestor of `destination` for
         *                  which some client has selected a type in `event_mask` and for which no
         *                  intervening window has that type in its do-not-propagate-mask. If no such
         *                  window exists or if the window is an ancestor of the focus window and
         *                  `InputFocus` was originally specified as the destination, the event is not sent
         *                  to any clients. Otherwise, the event is reported to every client selecting on
         *                  the final destination any of the types specified in `event_mask`.
         * @param event_mask Event_mask for determining which clients should receive the specified event.
         *                   See `destination` and `propagate`.
         * @param event The event to send to the specified `destination`.
         *
         * @see ConfigureNotifyEvent
         */
        [CCode (cname = "xcb_send_event", instance_pos = 1.2)]
        public VoidCookie send_event (Xcb.Connection connection, bool propagate, EventMask event_mask, [CCode (array_length = false)]char event[32]);
        /**
         * send an event
         *
         * Identifies the `destination` window, determines which clients should receive
         * the specified event and ignores any active grabs.
         *
         * The `event` must be one of the core events or an event defined by an extension,
         * so that the X server can correctly byte-swap the contents as necessary. The
         * contents of `event` are otherwise unaltered and unchecked except for the
         * `send_event` field which is forced to 'true'.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `destination` window does not exist.
         *
         * @param connection The connection.
         * @param propagate If `propagate` is true and no clients have selected any event on `destination`,
         *                  the destination is replaced with the closest ancestor of `destination` for
         *                  which some client has selected a type in `event_mask` and for which no
         *                  intervening window has that type in its do-not-propagate-mask. If no such
         *                  window exists or if the window is an ancestor of the focus window and
         *                  `InputFocus` was originally specified as the destination, the event is not sent
         *                  to any clients. Otherwise, the event is reported to every client selecting on
         *                  the final destination any of the types specified in `event_mask`.
         * @param event_mask Event_mask for determining which clients should receive the specified event.
         *                   See `destination` and `propagate`.
         * @param event The event to send to the specified `destination`.
         *
         * @see ConfigureNotifyEvent
         */
        [CCode (cname = "xcb_send_event_checked", instance_pos = 1.2)]
        public VoidCookie send_event_checked (Xcb.Connection connection, bool propagate, EventMask event_mask, [CCode (array_length = false)]char event[32]);
        /**
         * Grab the pointer
         *
         * Actively grabs control of the pointer. Further pointer events are reported only to the grabbing client. Overrides any active pointer grab by this client.
         *
         *
         * = Errors: =
         *
         *  * {@link ValueError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param owner_events If 1, the `grab_window` will still get the pointer events. If 0, events are not
         *                     reported to the `grab_window`.
         * @param event_mask Specifies which pointer events are reported to the client.
         *                   TODO: which values?
         * @param pointer_mode pointer_mode
         * @param keyboard_mode keyboard_mode
         * @param confine_to Specifies the window to confine the pointer in (the user will not be able to
         *                   move the pointer out of that window).
         *                   The special value `XCB_NONE` means don't confine the pointer.
         * @param cursor Specifies the cursor that should be displayed or `XCB_NONE` to not change the
         *               cursor.
         * @param time The time argument allows you to avoid certain circumstances that come up if
         *             applications take a long time to respond or if there are long network delays.
         *             Consider a situation where you have two applications, both of which normally
         *             grab the pointer when clicked on. If both applications specify the timestamp
         *             from the event, the second application may wake up faster and successfully grab
         *             the pointer before the first application. The first application then will get
         *             an indication that the other application grabbed the pointer before its request
         *             was processed.
         *             The special value `XCB_CURRENT_TIME` will be replaced with the current server
         *             time.
         *
         * @see Window.grab_keyboard
         */
        [CCode (cname = "xcb_grab_pointer", instance_pos = 2.2)]
        public GrabPointerCookie grab_pointer (Xcb.Connection connection, bool owner_events, EventMask event_mask, GrabMode pointer_mode, GrabMode keyboard_mode, Window confine_to, Cursor cursor, Timestamp time);
        /**
         * Grab pointer button(s)
         *
         * This request establishes a passive grab. The pointer is actively grabbed as
         * described in GrabPointer, the last-pointer-grab time is set to the time at
         * which the button was pressed (as transmitted in the ButtonPress event), and the
         * ButtonPress event is reported if all of the following conditions are true:
         *
         * The pointer is not grabbed and the specified button is logically pressed when
         * the specified modifier keys are logically down, and no other buttons or
         * modifier keys are logically down.
         *
         * The grab-window contains the pointer.
         *
         * The confine-to window (if any) is viewable.
         *
         * A passive grab on the same button/key combination does not exist on any
         * ancestor of grab-window.
         *
         * The interpretation of the remaining arguments is the same as for GrabPointer.
         * The active grab is terminated automatically when the logical state of the
         * pointer has all buttons released, independent of the logical state of modifier
         * keys. Note that the logical state of a device (as seen by means of the
         * protocol) may lag the physical state if device event processing is frozen. This
         * request overrides all previous passive grabs by the same client on the same
         * button/key combinations on the same window. A modifier of AnyModifier is
         * equivalent to issuing the request for all possible modifier combinations
         * (including the combination of no modifiers). It is not required that all
         * specified modifiers have currently assigned keycodes. A button of AnyButton is
         * equivalent to issuing the request for all possible buttons. Otherwise, it is
         * not required that the button specified currently be assigned to a physical
         * button.
         *
         * An Access error is generated if some other client has already issued a
         * GrabButton request with the same button/key combination on the same window.
         * When using AnyModifier or AnyButton, the request fails completely (no grabs are
         * established), and an Access error is generated if there is a conflicting grab
         * for any combination. The request has no effect on an active grab.
         *
         *
         * = Errors: =
         *
         *  * {@link AccessError}:  Another client has already issued a GrabButton with the same button/key combination on the same window.
         *
         * @param connection The connection.
         * @param owner_events If 1, the `grab_window` will still get the pointer events. If 0, events are not
         *                     reported to the `grab_window`.
         * @param event_mask Specifies which pointer events are reported to the client.
         *                   TODO: which values?
         * @param pointer_mode pointer_mode
         * @param keyboard_mode keyboard_mode
         * @param confine_to Specifies the window to confine the pointer in (the user will not be able to
         *                   move the pointer out of that window).
         *                   The special value `XCB_NONE` means don't confine the pointer.
         * @param cursor Specifies the cursor that should be displayed or `XCB_NONE` to not change the
         *               cursor.
         * @param button button
         * @param modifiers The modifiers to grab.
         *                  Using the special value `XCB_MOD_MASK_ANY` means grab the pointer with all
         *                  possible modifier combinations.
         */
        [CCode (cname = "xcb_grab_button", instance_pos = 2.2)]
        public VoidCookie grab_button (Xcb.Connection connection, bool owner_events, EventMask event_mask, GrabMode pointer_mode, GrabMode keyboard_mode, Window confine_to, Cursor cursor, ButtonIndex button, ModMask modifiers);
        /**
         * Grab pointer button(s)
         *
         * This request establishes a passive grab. The pointer is actively grabbed as
         * described in GrabPointer, the last-pointer-grab time is set to the time at
         * which the button was pressed (as transmitted in the ButtonPress event), and the
         * ButtonPress event is reported if all of the following conditions are true:
         *
         * The pointer is not grabbed and the specified button is logically pressed when
         * the specified modifier keys are logically down, and no other buttons or
         * modifier keys are logically down.
         *
         * The grab-window contains the pointer.
         *
         * The confine-to window (if any) is viewable.
         *
         * A passive grab on the same button/key combination does not exist on any
         * ancestor of grab-window.
         *
         * The interpretation of the remaining arguments is the same as for GrabPointer.
         * The active grab is terminated automatically when the logical state of the
         * pointer has all buttons released, independent of the logical state of modifier
         * keys. Note that the logical state of a device (as seen by means of the
         * protocol) may lag the physical state if device event processing is frozen. This
         * request overrides all previous passive grabs by the same client on the same
         * button/key combinations on the same window. A modifier of AnyModifier is
         * equivalent to issuing the request for all possible modifier combinations
         * (including the combination of no modifiers). It is not required that all
         * specified modifiers have currently assigned keycodes. A button of AnyButton is
         * equivalent to issuing the request for all possible buttons. Otherwise, it is
         * not required that the button specified currently be assigned to a physical
         * button.
         *
         * An Access error is generated if some other client has already issued a
         * GrabButton request with the same button/key combination on the same window.
         * When using AnyModifier or AnyButton, the request fails completely (no grabs are
         * established), and an Access error is generated if there is a conflicting grab
         * for any combination. The request has no effect on an active grab.
         *
         *
         * = Errors: =
         *
         *  * {@link AccessError}:  Another client has already issued a GrabButton with the same button/key combination on the same window.
         *
         * @param connection The connection.
         * @param owner_events If 1, the `grab_window` will still get the pointer events. If 0, events are not
         *                     reported to the `grab_window`.
         * @param event_mask Specifies which pointer events are reported to the client.
         *                   TODO: which values?
         * @param pointer_mode pointer_mode
         * @param keyboard_mode keyboard_mode
         * @param confine_to Specifies the window to confine the pointer in (the user will not be able to
         *                   move the pointer out of that window).
         *                   The special value `XCB_NONE` means don't confine the pointer.
         * @param cursor Specifies the cursor that should be displayed or `XCB_NONE` to not change the
         *               cursor.
         * @param button button
         * @param modifiers The modifiers to grab.
         *                  Using the special value `XCB_MOD_MASK_ANY` means grab the pointer with all
         *                  possible modifier combinations.
         */
        [CCode (cname = "xcb_grab_button_checked", instance_pos = 2.2)]
        public VoidCookie grab_button_checked (Xcb.Connection connection, bool owner_events, EventMask event_mask, GrabMode pointer_mode, GrabMode keyboard_mode, Window confine_to, Cursor cursor, ButtonIndex button, ModMask modifiers);
        [CCode (cname = "xcb_ungrab_button", instance_pos = 2.2)]
        public VoidCookie ungrab_button (Xcb.Connection connection, ButtonIndex button, ModMask modifiers);
        [CCode (cname = "xcb_ungrab_button_checked", instance_pos = 2.2)]
        public VoidCookie ungrab_button_checked (Xcb.Connection connection, ButtonIndex button, ModMask modifiers);
        /**
         * Grab the keyboard
         *
         * Actively grabs control of the keyboard and generates FocusIn and FocusOut
         * events. Further key events are reported only to the grabbing client.
         *
         * Any active keyboard grab by this client is overridden. If the keyboard is
         * actively grabbed by some other client, `AlreadyGrabbed` is returned. If
         * `grab_window` is not viewable, `GrabNotViewable` is returned. If the keyboard
         * is frozen by an active grab of another client, `GrabFrozen` is returned. If the
         * specified `time` is earlier than the last-keyboard-grab time or later than the
         * current X server time, `GrabInvalidTime` is returned. Otherwise, the
         * last-keyboard-grab time is set to the specified time.
         *
         * = Errors: =
         *
         *  * {@link ValueError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param owner_events If 1, the `grab_window` will still get the pointer events. If 0, events are not
         *                     reported to the `grab_window`.
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The special value `XCB_CURRENT_TIME` will be replaced with the current server
         *             time.
         * @param pointer_mode pointer_mode
         * @param keyboard_mode keyboard_mode
         *
         * @see Window.grab_pointer
         */
        [CCode (cname = "xcb_grab_keyboard", instance_pos = 2.2)]
        public GrabKeyboardCookie grab_keyboard (Xcb.Connection connection, bool owner_events, Timestamp time, GrabMode pointer_mode, GrabMode keyboard_mode);
        /**
         * Grab keyboard key(s)
         *
         * Establishes a passive grab on the keyboard. In the future, the keyboard is
         * actively grabbed (as for `GrabKeyboard`), the last-keyboard-grab time is set to
         * the time at which the key was pressed (as transmitted in the KeyPress event),
         * and the KeyPress event is reported if all of the following conditions are true:
         *
         * The keyboard is not grabbed and the specified key (which can itself be a
         * modifier key) is logically pressed when the specified modifier keys are
         * logically down, and no other modifier keys are logically down.
         *
         * Either the grab_window is an ancestor of (or is) the focus window, or the
         * grab_window is a descendant of the focus window and contains the pointer.
         *
         * A passive grab on the same key combination does not exist on any ancestor of
         * grab_window.
         *
         * The interpretation of the remaining arguments is as for XGrabKeyboard.  The active grab is terminated
         * automatically when the logical state of the keyboard has the specified key released (independent of the
         * logical state of the modifier keys), at which point a KeyRelease event is reported to the grabbing window.
         *
         * Note that the logical state of a device (as seen by client applications) may lag the physical state if
         * device event processing is frozen.
         *
         * A modifiers argument of AnyModifier is equivalent to issuing the request for all possible modifier combinations (including the combination of no modifiers).  It is not required that all modifiers specified
         * have currently assigned KeyCodes.  A keycode argument of AnyKey is equivalent to issuing the request for
         * all possible KeyCodes.  Otherwise, the specified keycode must be in the range specified by min_keycode
         * and max_keycode in the connection setup, or a BadValue error results.
         *
         * If some other client has issued a XGrabKey with the same key combination on the same window, a BadAccess
         * error results.  When using AnyModifier or AnyKey, the request fails completely, and a BadAccess error
         * results (no grabs are established) if there is a conflicting grab for any combination.
         *
         *
         * = Errors: =
         *
         *  * {@link AccessError}:  Another client has already issued a GrabKey with the same button/key combination on the same window.
         *
         * @param connection The connection.
         * @param owner_events If 1, the `grab_window` will still get the pointer events. If 0, events are not
         *                     reported to the `grab_window`.
         * @param modifiers The modifiers to grab.
         *                  Using the special value `XCB_MOD_MASK_ANY` means grab the pointer with all
         *                  possible modifier combinations.
         * @param key The keycode of the key to grab.
         *            The special value `XCB_GRAB_ANY` means grab any key.
         * @param pointer_mode pointer_mode
         * @param keyboard_mode keyboard_mode
         *
         * @see Window.grab_keyboard
         */
        [CCode (cname = "xcb_grab_key", instance_pos = 2.2)]
        public VoidCookie grab_key (Xcb.Connection connection, bool owner_events, ModMask modifiers, Keycode key, GrabMode pointer_mode, GrabMode keyboard_mode);
        /**
         * Grab keyboard key(s)
         *
         * Establishes a passive grab on the keyboard. In the future, the keyboard is
         * actively grabbed (as for `GrabKeyboard`), the last-keyboard-grab time is set to
         * the time at which the key was pressed (as transmitted in the KeyPress event),
         * and the KeyPress event is reported if all of the following conditions are true:
         *
         * The keyboard is not grabbed and the specified key (which can itself be a
         * modifier key) is logically pressed when the specified modifier keys are
         * logically down, and no other modifier keys are logically down.
         *
         * Either the grab_window is an ancestor of (or is) the focus window, or the
         * grab_window is a descendant of the focus window and contains the pointer.
         *
         * A passive grab on the same key combination does not exist on any ancestor of
         * grab_window.
         *
         * The interpretation of the remaining arguments is as for XGrabKeyboard.  The active grab is terminated
         * automatically when the logical state of the keyboard has the specified key released (independent of the
         * logical state of the modifier keys), at which point a KeyRelease event is reported to the grabbing window.
         *
         * Note that the logical state of a device (as seen by client applications) may lag the physical state if
         * device event processing is frozen.
         *
         * A modifiers argument of AnyModifier is equivalent to issuing the request for all possible modifier combinations (including the combination of no modifiers).  It is not required that all modifiers specified
         * have currently assigned KeyCodes.  A keycode argument of AnyKey is equivalent to issuing the request for
         * all possible KeyCodes.  Otherwise, the specified keycode must be in the range specified by min_keycode
         * and max_keycode in the connection setup, or a BadValue error results.
         *
         * If some other client has issued a XGrabKey with the same key combination on the same window, a BadAccess
         * error results.  When using AnyModifier or AnyKey, the request fails completely, and a BadAccess error
         * results (no grabs are established) if there is a conflicting grab for any combination.
         *
         *
         * = Errors: =
         *
         *  * {@link AccessError}:  Another client has already issued a GrabKey with the same button/key combination on the same window.
         *
         * @param connection The connection.
         * @param owner_events If 1, the `grab_window` will still get the pointer events. If 0, events are not
         *                     reported to the `grab_window`.
         * @param modifiers The modifiers to grab.
         *                  Using the special value `XCB_MOD_MASK_ANY` means grab the pointer with all
         *                  possible modifier combinations.
         * @param key The keycode of the key to grab.
         *            The special value `XCB_GRAB_ANY` means grab any key.
         * @param pointer_mode pointer_mode
         * @param keyboard_mode keyboard_mode
         *
         * @see Window.grab_keyboard
         */
        [CCode (cname = "xcb_grab_key_checked", instance_pos = 2.2)]
        public VoidCookie grab_key_checked (Xcb.Connection connection, bool owner_events, ModMask modifiers, Keycode key, GrabMode pointer_mode, GrabMode keyboard_mode);
        /**
         * release a key combination
         *
         * Releases the key combination on `grab_window` if you grabbed it using
         * `xcb_grab_key` before.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `grab_window` does not exist.
         *
         * @param connection The connection.
         * @param key The keycode of the specified key combination.
         *            Using the special value `XCB_GRAB_ANY` means releasing all possible key codes.
         * @param modifiers The modifiers of the specified key combination.
         *                  Using the special value `XCB_MOD_MASK_ANY` means releasing the key combination
         *                  with every possible modifier combination.
         *
         * @see Window.grab_key
         */
        [CCode (cname = "xcb_ungrab_key", instance_pos = 2.2)]
        public VoidCookie ungrab_key (Xcb.Connection connection, Keycode key, ModMask modifiers);
        /**
         * release a key combination
         *
         * Releases the key combination on `grab_window` if you grabbed it using
         * `xcb_grab_key` before.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `grab_window` does not exist.
         *
         * @param connection The connection.
         * @param key The keycode of the specified key combination.
         *            Using the special value `XCB_GRAB_ANY` means releasing all possible key codes.
         * @param modifiers The modifiers of the specified key combination.
         *                  Using the special value `XCB_MOD_MASK_ANY` means releasing the key combination
         *                  with every possible modifier combination.
         *
         * @see Window.grab_key
         */
        [CCode (cname = "xcb_ungrab_key_checked", instance_pos = 2.2)]
        public VoidCookie ungrab_key_checked (Xcb.Connection connection, Keycode key, ModMask modifiers);
        /**
         * get pointer coordinates
         *
         * Gets the root window the pointer is logically on and the pointer coordinates
         * relative to the root window's origin.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `window` does not exist.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_query_pointer", instance_pos = 1.1)]
        public QueryPointerCookie query_pointer (Xcb.Connection connection);
        [CCode (cname = "xcb_get_motion_events", instance_pos = 1.1)]
        public GetMotionEventsCookie get_motion_events (Xcb.Connection connection, Timestamp start, Timestamp stop);
        [CCode (cname = "xcb_translate_coordinates", instance_pos = 1.1)]
        public TranslateCoordinatesCookie translate_coordinates (Xcb.Connection connection, Window dst_window, int16 src_x, int16 src_y);
        /**
         * move mouse pointer
         *
         * Moves the mouse pointer to the specified position.
         *
         * If `src_window` is not `XCB_NONE` (TODO), the move will only take place if the
         * pointer is inside `src_window` and within the rectangle specified by (`src_x`,
         * `src_y`, `src_width`, `src_height`). The rectangle coordinates are relative to
         * `src_window`.
         *
         * If `dst_window` is not `XCB_NONE` (TODO), the pointer will be moved to the
         * offsets (`dst_x`, `dst_y`) relative to `dst_window`. If `dst_window` is
         * `XCB_NONE` (TODO), the pointer will be moved by the offsets (`dst_x`, `dst_y`)
         * relative to the current position of the pointer.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param dst_window If `dst_window` is not `XCB_NONE` (TODO), the pointer will be moved to the
         *                   offsets (`dst_x`, `dst_y`) relative to `dst_window`. If `dst_window` is
         *                   `XCB_NONE` (TODO), the pointer will be moved by the offsets (`dst_x`, `dst_y`)
         *                   relative to the current position of the pointer.
         *
         * @see Window.set_input_focus
         */
        [CCode (cname = "xcb_warp_pointer", instance_pos = 1.1)]
        public VoidCookie warp_pointer (Xcb.Connection connection, Window dst_window, int16 src_x, int16 src_y, uint16 src_width, uint16 src_height, int16 dst_x, int16 dst_y);
        /**
         * move mouse pointer
         *
         * Moves the mouse pointer to the specified position.
         *
         * If `src_window` is not `XCB_NONE` (TODO), the move will only take place if the
         * pointer is inside `src_window` and within the rectangle specified by (`src_x`,
         * `src_y`, `src_width`, `src_height`). The rectangle coordinates are relative to
         * `src_window`.
         *
         * If `dst_window` is not `XCB_NONE` (TODO), the pointer will be moved to the
         * offsets (`dst_x`, `dst_y`) relative to `dst_window`. If `dst_window` is
         * `XCB_NONE` (TODO), the pointer will be moved by the offsets (`dst_x`, `dst_y`)
         * relative to the current position of the pointer.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param dst_window If `dst_window` is not `XCB_NONE` (TODO), the pointer will be moved to the
         *                   offsets (`dst_x`, `dst_y`) relative to `dst_window`. If `dst_window` is
         *                   `XCB_NONE` (TODO), the pointer will be moved by the offsets (`dst_x`, `dst_y`)
         *                   relative to the current position of the pointer.
         *
         * @see Window.set_input_focus
         */
        [CCode (cname = "xcb_warp_pointer_checked", instance_pos = 1.1)]
        public VoidCookie warp_pointer_checked (Xcb.Connection connection, Window dst_window, int16 src_x, int16 src_y, uint16 src_width, uint16 src_height, int16 dst_x, int16 dst_y);
        /**
         * Sets input focus
         *
         * Changes the input focus and the last-focus-change time. If the specified `time`
         * is earlier than the current last-focus-change time, the request is ignored (to
         * avoid race conditions when running X over the network).
         *
         * A FocusIn and FocusOut event is generated when focus is changed.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `focus` window does not exist.
         *
         * @param connection The connection.
         * @param revert_to Specifies what happens when the `focus` window becomes unviewable (if `focus`
         *                  is neither `XCB_NONE` nor `XCB_POINTER_ROOT`).
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The special value `XCB_CURRENT_TIME` will be replaced with the current server
         *             time.
         *
         * @see FocusInEvent
         * @see FocusOutEvent
         */
        [CCode (cname = "xcb_set_input_focus", instance_pos = 2.2)]
        public VoidCookie set_input_focus (Xcb.Connection connection, InputFocus revert_to, Timestamp time);
        /**
         * Sets input focus
         *
         * Changes the input focus and the last-focus-change time. If the specified `time`
         * is earlier than the current last-focus-change time, the request is ignored (to
         * avoid race conditions when running X over the network).
         *
         * A FocusIn and FocusOut event is generated when focus is changed.
         *
         * = Errors: =
         *
         *  * {@link WindowError}:  The specified `focus` window does not exist.
         *
         * @param connection The connection.
         * @param revert_to Specifies what happens when the `focus` window becomes unviewable (if `focus`
         *                  is neither `XCB_NONE` nor `XCB_POINTER_ROOT`).
         * @param time Timestamp to avoid race conditions when running X over the network.
         *             The special value `XCB_CURRENT_TIME` will be replaced with the current server
         *             time.
         *
         * @see FocusInEvent
         * @see FocusOutEvent
         */
        [CCode (cname = "xcb_set_input_focus_checked", instance_pos = 2.2)]
        public VoidCookie set_input_focus_checked (Xcb.Connection connection, InputFocus revert_to, Timestamp time);
        [CCode (cname = "xcb_clear_area", instance_pos = 2.2)]
        public VoidCookie clear_area (Xcb.Connection connection, bool exposures, int16 x, int16 y, uint16 width, uint16 height);
        [CCode (cname = "xcb_clear_area_checked", instance_pos = 2.2)]
        public VoidCookie clear_area_checked (Xcb.Connection connection, bool exposures, int16 x, int16 y, uint16 width, uint16 height);
        [CCode (cname = "xcb_list_installed_colormaps", instance_pos = 1.1)]
        public ListInstalledColormapsCookie list_installed_colormaps (Xcb.Connection connection);
        [CCode (cname = "xcb_rotate_properties", instance_pos = 1.1)]
        public VoidCookie rotate_properties (Xcb.Connection connection, int16 delta, [CCode (array_length_pos = 1.2)]Atom[]? atoms);
        [CCode (cname = "xcb_rotate_properties_checked", instance_pos = 1.1)]
        public VoidCookie rotate_properties_checked (Xcb.Connection connection, int16 delta, [CCode (array_length_pos = 1.2)]Atom[]? atoms);
    }

    [Compact, CCode (cname = "xcb_get_window_attributes_reply_t", free_function = "free")]
    public class GetWindowAttributesReply {
        public BackingStore backing_store;
        public Visualid visual;
        public WindowClass _class;
        public Gravity bit_gravity;
        public Gravity win_gravity;
        public uint32 backing_planes;
        public uint32 backing_pixel;
        public bool save_under;
        public bool map_is_installed;
        public MapState map_state;
        public bool override_redirect;
        public Colormap colormap;
        public EventMask all_event_masks;
        public EventMask your_event_mask;
        public EventMask do_not_propagate_mask;
    }

    [SimpleType, CCode (cname = "xcb_get_window_attributes_cookie_t")]
    public struct GetWindowAttributesCookie : VoidCookie {
        [CCode (cname = "xcb_get_window_attributes_reply", instance_pos = 1.1)]
        public GetWindowAttributesReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_query_tree_reply_t", free_function = "free")]
    public class QueryTreeReply {
        public Window root;
        public Window parent;
        public uint16 children_len;
        [CCode (cname = "xcb_query_tree_children_iterator")]
        _WindowIterator _iterator ();
        public WindowIterator iterator () {
            return (WindowIterator) _iterator ();
        }
        public int children_length {
            [CCode (cname = "xcb_query_tree_children_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Window[] children {
            [CCode (cname = "xcb_query_tree_children")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_query_tree_cookie_t")]
    public struct QueryTreeCookie : VoidCookie {
        [CCode (cname = "xcb_query_tree_reply", instance_pos = 1.1)]
        public QueryTreeReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_property_reply_t", free_function = "free")]
    public class GetPropertyReply {
        public uint8 format;
        public Atom type;
        public uint32 bytes_after;
        public uint32 value_len;
        public int value_length {
            [CCode (cname = "xcb_get_property_value_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned void[] value {
            [CCode (cname = "xcb_get_property_value")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_get_property_cookie_t")]
    public struct GetPropertyCookie : VoidCookie {
        [CCode (cname = "xcb_get_property_reply", instance_pos = 1.1)]
        public GetPropertyReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_list_properties_reply_t", free_function = "free")]
    public class ListPropertiesReply {
        public uint16 atoms_len;
        [CCode (cname = "xcb_list_properties_atoms_iterator")]
        _AtomIterator _iterator ();
        public AtomIterator iterator () {
            return (AtomIterator) _iterator ();
        }
        public int atoms_length {
            [CCode (cname = "xcb_list_properties_atoms_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Atom[] atoms {
            [CCode (cname = "xcb_list_properties_atoms")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_list_properties_cookie_t")]
    public struct ListPropertiesCookie : VoidCookie {
        [CCode (cname = "xcb_list_properties_reply", instance_pos = 1.1)]
        public ListPropertiesReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_grab_pointer_reply_t", free_function = "free")]
    public class GrabPointerReply {
        public GrabStatus status;
    }

    [SimpleType, CCode (cname = "xcb_grab_pointer_cookie_t")]
    public struct GrabPointerCookie : VoidCookie {
        [CCode (cname = "xcb_grab_pointer_reply", instance_pos = 1.1)]
        public GrabPointerReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_grab_keyboard_reply_t", free_function = "free")]
    public class GrabKeyboardReply {
        public GrabStatus status;
    }

    [SimpleType, CCode (cname = "xcb_grab_keyboard_cookie_t")]
    public struct GrabKeyboardCookie : VoidCookie {
        [CCode (cname = "xcb_grab_keyboard_reply", instance_pos = 1.1)]
        public GrabKeyboardReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_query_pointer_reply_t", free_function = "free")]
    public class QueryPointerReply {
        public bool same_screen;
        public Window root;
        public Window child;
        public int16 root_x;
        public int16 root_y;
        public int16 win_x;
        public int16 win_y;
        public KeyButMask mask;
    }

    [SimpleType, CCode (cname = "xcb_query_pointer_cookie_t")]
    public struct QueryPointerCookie : VoidCookie {
        [CCode (cname = "xcb_query_pointer_reply", instance_pos = 1.1)]
        public QueryPointerReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_motion_events_reply_t", free_function = "free")]
    public class GetMotionEventsReply {
        public uint32 events_len;
        [CCode (cname = "xcb_get_motion_events_events_iterator")]
        _TimecoordIterator _iterator ();
        public TimecoordIterator iterator () {
            return (TimecoordIterator) _iterator ();
        }
        public int events_length {
            [CCode (cname = "xcb_get_motion_events_events_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Timecoord[] events {
            [CCode (cname = "xcb_get_motion_events_events")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_get_motion_events_cookie_t")]
    public struct GetMotionEventsCookie : VoidCookie {
        [CCode (cname = "xcb_get_motion_events_reply", instance_pos = 1.1)]
        public GetMotionEventsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_translate_coordinates_reply_t", free_function = "free")]
    public class TranslateCoordinatesReply {
        public bool same_screen;
        public Window child;
        public int16 dst_x;
        public int16 dst_y;
    }

    [SimpleType, CCode (cname = "xcb_translate_coordinates_cookie_t")]
    public struct TranslateCoordinatesCookie : VoidCookie {
        [CCode (cname = "xcb_translate_coordinates_reply", instance_pos = 1.1)]
        public TranslateCoordinatesReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_list_installed_colormaps_reply_t", free_function = "free")]
    public class ListInstalledColormapsReply {
        public uint16 cmaps_len;
        [CCode (cname = "xcb_list_installed_colormaps_cmaps_iterator")]
        _ColormapIterator _iterator ();
        public ColormapIterator iterator () {
            return (ColormapIterator) _iterator ();
        }
        public int cmaps_length {
            [CCode (cname = "xcb_list_installed_colormaps_cmaps_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Colormap[] cmaps {
            [CCode (cname = "xcb_list_installed_colormaps_cmaps")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_list_installed_colormaps_cookie_t")]
    public struct ListInstalledColormapsCookie : VoidCookie {
        [CCode (cname = "xcb_list_installed_colormaps_reply", instance_pos = 1.1)]
        public ListInstalledColormapsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [SimpleType, CCode (cname = "xcb_pixmap_iterator_t")]
    struct _PixmapIterator
    {
        int rem;
        int index;
        unowned Pixmap? data;
    }

    [CCode (cname = "xcb_pixmap_iterator_t")]
    public struct PixmapIterator
    {
        [CCode (cname = "xcb_pixmap_next")]
        void _next ();

        public inline unowned Pixmap?
        next_value ()
        {
            if (((_PixmapIterator)this).rem > 0)
            {
                unowned Pixmap? d = ((_PixmapIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [CCode (cname = "xcb_pixmap_t", has_type_id = false)]
    public struct Pixmap : Drawable {
        /**
         * Allocates an XID for a new Pixmap.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_generate_id")]
        public Pixmap (Xcb.Connection connection);

        /**
         * Creates a pixmap
         *
         * Creates a pixmap. The pixmap can only be used on the same screen as `drawable`
         * is on and only with drawables of the same `depth`.
         *
         * = Errors: =
         *
         *  * {@link ValueError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param depth TODO
         * @param drawable Drawable to get the screen from.
         * @param width The width of the new pixmap.
         * @param height The height of the new pixmap.
         *
         */
        [CCode (cname = "xcb_create_pixmap", instance_pos = 2.2)]
        public VoidCookie create (Xcb.Connection connection, uint8 depth, Drawable drawable, uint16 width, uint16 height);
        /**
         * Creates a pixmap
         *
         * Creates a pixmap. The pixmap can only be used on the same screen as `drawable`
         * is on and only with drawables of the same `depth`.
         *
         * = Errors: =
         *
         *  * {@link ValueError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param depth TODO
         * @param drawable Drawable to get the screen from.
         * @param width The width of the new pixmap.
         * @param height The height of the new pixmap.
         *
         */
        [CCode (cname = "xcb_create_pixmap_checked", instance_pos = 2.2)]
        public VoidCookie create_checked (Xcb.Connection connection, uint8 depth, Drawable drawable, uint16 width, uint16 height);
        /**
         * Destroys a pixmap
         *
         * Deletes the association between the pixmap ID and the pixmap. The pixmap
         * storage will be freed when there are no more references to it.
         *
         * = Errors: =
         *
         *  * {@link PixmapError}:  The specified pixmap does not exist.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_free_pixmap", instance_pos = 1.1)]
        public VoidCookie free (Xcb.Connection connection);
        /**
         * Destroys a pixmap
         *
         * Deletes the association between the pixmap ID and the pixmap. The pixmap
         * storage will be freed when there are no more references to it.
         *
         * = Errors: =
         *
         *  * {@link PixmapError}:  The specified pixmap does not exist.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_free_pixmap_checked", instance_pos = 1.1)]
        public VoidCookie free_checked (Xcb.Connection connection);
    }

    [SimpleType, CCode (cname = "xcb_cursor_iterator_t")]
    struct _CursorIterator
    {
        int rem;
        int index;
        unowned Cursor? data;
    }

    [CCode (cname = "xcb_cursor_iterator_t")]
    public struct CursorIterator
    {
        [CCode (cname = "xcb_cursor_next")]
        void _next ();

        public inline unowned Cursor?
        next_value ()
        {
            if (((_CursorIterator)this).rem > 0)
            {
                unowned Cursor? d = ((_CursorIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [CCode (cname = "xcb_cursor_t", has_type_id = false)]
    public struct Cursor : uint32 {
        /**
         * Allocates an XID for a new Cursor.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_generate_id")]
        public Cursor (Xcb.Connection connection);

        [CCode (cname = "xcb_change_active_pointer_grab", instance_pos = 1.1)]
        public VoidCookie change_active_pointer_grab (Xcb.Connection connection, Timestamp time, EventMask event_mask);
        [CCode (cname = "xcb_change_active_pointer_grab_checked", instance_pos = 1.1)]
        public VoidCookie change_active_pointer_grab_checked (Xcb.Connection connection, Timestamp time, EventMask event_mask);
        [CCode (cname = "xcb_create_cursor", instance_pos = 1.1)]
        public VoidCookie create (Xcb.Connection connection, Pixmap source, Pixmap mask, uint16 fore_red, uint16 fore_green, uint16 fore_blue, uint16 back_red, uint16 back_green, uint16 back_blue, uint16 x, uint16 y);
        [CCode (cname = "xcb_create_cursor_checked", instance_pos = 1.1)]
        public VoidCookie create_checked (Xcb.Connection connection, Pixmap source, Pixmap mask, uint16 fore_red, uint16 fore_green, uint16 fore_blue, uint16 back_red, uint16 back_green, uint16 back_blue, uint16 x, uint16 y);
        /**
         * create cursor
         *
         * Creates a cursor from a font glyph. X provides a set of standard cursor shapes
         * in a special font named cursor. Applications are encouraged to use this
         * interface for their cursors because the font can be customized for the
         * individual display type.
         *
         * All pixels which are set to 1 in the source will use the foreground color (as
         * specified by `fore_red`, `fore_green` and `fore_blue`). All pixels set to 0
         * will use the background color (as specified by `back_red`, `back_green` and
         * `back_blue`).
         *
         * = Errors: =
         *
         *  * {@link AllocError}:  The X server could not allocate the requested resources (no memory?).
         *
         * @param connection The connection.
         * @param source_font In which font to look for the cursor glyph.
         * @param mask_font In which font to look for the mask glyph.
         * @param source_char The glyph of `source_font` to use.
         * @param mask_char The glyph of `mask_font` to use as a mask: Pixels which are set to 1 define
         *                  which source pixels are displayed. All pixels which are set to 0 are not
         *                  displayed.
         * @param fore_red The red value of the foreground color.
         * @param fore_green The green value of the foreground color.
         * @param fore_blue The blue value of the foreground color.
         * @param back_red The red value of the background color.
         * @param back_green The green value of the background color.
         * @param back_blue The blue value of the background color.
         */
        [CCode (cname = "xcb_create_glyph_cursor", instance_pos = 1.1)]
        public VoidCookie create_glyph (Xcb.Connection connection, Font source_font, Font mask_font, uint16 source_char, uint16 mask_char, uint16 fore_red, uint16 fore_green, uint16 fore_blue, uint16 back_red, uint16 back_green, uint16 back_blue);
        /**
         * create cursor
         *
         * Creates a cursor from a font glyph. X provides a set of standard cursor shapes
         * in a special font named cursor. Applications are encouraged to use this
         * interface for their cursors because the font can be customized for the
         * individual display type.
         *
         * All pixels which are set to 1 in the source will use the foreground color (as
         * specified by `fore_red`, `fore_green` and `fore_blue`). All pixels set to 0
         * will use the background color (as specified by `back_red`, `back_green` and
         * `back_blue`).
         *
         * = Errors: =
         *
         *  * {@link AllocError}:  The X server could not allocate the requested resources (no memory?).
         *
         * @param connection The connection.
         * @param source_font In which font to look for the cursor glyph.
         * @param mask_font In which font to look for the mask glyph.
         * @param source_char The glyph of `source_font` to use.
         * @param mask_char The glyph of `mask_font` to use as a mask: Pixels which are set to 1 define
         *                  which source pixels are displayed. All pixels which are set to 0 are not
         *                  displayed.
         * @param fore_red The red value of the foreground color.
         * @param fore_green The green value of the foreground color.
         * @param fore_blue The blue value of the foreground color.
         * @param back_red The red value of the background color.
         * @param back_green The green value of the background color.
         * @param back_blue The blue value of the background color.
         */
        [CCode (cname = "xcb_create_glyph_cursor_checked", instance_pos = 1.1)]
        public VoidCookie create_glyph_checked (Xcb.Connection connection, Font source_font, Font mask_font, uint16 source_char, uint16 mask_char, uint16 fore_red, uint16 fore_green, uint16 fore_blue, uint16 back_red, uint16 back_green, uint16 back_blue);
        /**
         * Deletes a cursor
         *
         * Deletes the association between the cursor resource ID and the specified
         * cursor. The cursor is freed when no other resource references it.
         *
         * = Errors: =
         *
         *  * {@link CursorError}:  The specified cursor does not exist.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_free_cursor", instance_pos = 1.1)]
        public VoidCookie free (Xcb.Connection connection);
        /**
         * Deletes a cursor
         *
         * Deletes the association between the cursor resource ID and the specified
         * cursor. The cursor is freed when no other resource references it.
         *
         * = Errors: =
         *
         *  * {@link CursorError}:  The specified cursor does not exist.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_free_cursor_checked", instance_pos = 1.1)]
        public VoidCookie free_checked (Xcb.Connection connection);
        [CCode (cname = "xcb_recolor_cursor", instance_pos = 1.1)]
        public VoidCookie recolor (Xcb.Connection connection, uint16 fore_red, uint16 fore_green, uint16 fore_blue, uint16 back_red, uint16 back_green, uint16 back_blue);
        [CCode (cname = "xcb_recolor_cursor_checked", instance_pos = 1.1)]
        public VoidCookie recolor_checked (Xcb.Connection connection, uint16 fore_red, uint16 fore_green, uint16 fore_blue, uint16 back_red, uint16 back_green, uint16 back_blue);
    }

    [SimpleType, CCode (cname = "xcb_font_iterator_t")]
    struct _FontIterator
    {
        int rem;
        int index;
        unowned Font? data;
    }

    [CCode (cname = "xcb_font_iterator_t")]
    public struct FontIterator
    {
        [CCode (cname = "xcb_font_next")]
        void _next ();

        public inline unowned Font?
        next_value ()
        {
            if (((_FontIterator)this).rem > 0)
            {
                unowned Font? d = ((_FontIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [CCode (cname = "xcb_font_t", has_type_id = false)]
    public struct Font : Fontable {
        /**
         * Allocates an XID for a new Font.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_generate_id")]
        public Font (Xcb.Connection connection);

        /**
         * opens a font
         *
         * Opens any X core font matching the given `name` (for example "-misc-fixed-*").
         *
         * Note that X core fonts are deprecated (but still supported) in favor of
         * client-side rendering using Xft.
         *
         * = Errors: =
         *
         *  * {@link NameError}:  No font matches the given `name`.
         *
         * @param connection The connection.
         * @param name A pattern describing an X core font.
         *
         */
        [CCode (cname = "xcb_open_font", instance_pos = 1.1)]
        public VoidCookie open (Xcb.Connection connection, [CCode (array_length_pos = 1.2)]char[]? name);
        /**
         * opens a font
         *
         * Opens any X core font matching the given `name` (for example "-misc-fixed-*").
         *
         * Note that X core fonts are deprecated (but still supported) in favor of
         * client-side rendering using Xft.
         *
         * = Errors: =
         *
         *  * {@link NameError}:  No font matches the given `name`.
         *
         * @param connection The connection.
         * @param name A pattern describing an X core font.
         *
         */
        [CCode (cname = "xcb_open_font_checked", instance_pos = 1.1)]
        public VoidCookie open_checked (Xcb.Connection connection, [CCode (array_length_pos = 1.2)]char[]? name);
        [CCode (cname = "xcb_close_font", instance_pos = 1.1)]
        public VoidCookie close (Xcb.Connection connection);
        [CCode (cname = "xcb_close_font_checked", instance_pos = 1.1)]
        public VoidCookie close_checked (Xcb.Connection connection);
    }

    [SimpleType, CCode (cname = "xcb_gcontext_iterator_t")]
    struct _GContextIterator
    {
        int rem;
        int index;
        unowned GContext? data;
    }

    [CCode (cname = "xcb_gcontext_iterator_t")]
    public struct GContextIterator
    {
        [CCode (cname = "xcb_gcontext_next")]
        void _next ();

        public inline unowned GContext?
        next_value ()
        {
            if (((_GContextIterator)this).rem > 0)
            {
                unowned GContext? d = ((_GContextIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [CCode (cname = "xcb_gcontext_t", has_type_id = false)]
    public struct GContext : Fontable {
        /**
         * Allocates an XID for a new GContext.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_generate_id")]
        public GContext (Xcb.Connection connection);

        /**
         * Creates a graphics context
         *
         * Creates a graphics context. The graphics context can be used with any drawable
         * that has the same root and depth as the specified drawable.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param drawable Drawable to get the root/depth from.
         *
         */
        [CCode (cname = "xcb_create_gc", instance_pos = 1.1)]
        public VoidCookie create (Xcb.Connection connection, Drawable drawable, uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * Creates a graphics context
         *
         * Creates a graphics context. The graphics context can be used with any drawable
         * that has the same root and depth as the specified drawable.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param drawable Drawable to get the root/depth from.
         *
         */
        [CCode (cname = "xcb_create_gc_checked", instance_pos = 1.1)]
        public VoidCookie create_checked (Xcb.Connection connection, Drawable drawable, uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * change graphics context components
         *
         * Changes the components specified by `value_mask` for the specified graphics context.
         *
         * = Errors: =
         *
         *  * {@link FontError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param value_mask value_mask
         * @param value_list Values for each of the components specified in the bitmask `value_mask`. The
         *                   order has to correspond to the order of possible `value_mask` bits. See the
         *                   example.
         */
        [CCode (cname = "xcb_change_gc", instance_pos = 1.1)]
        public VoidCookie change (Xcb.Connection connection, uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        /**
         * change graphics context components
         *
         * Changes the components specified by `value_mask` for the specified graphics context.
         *
         * = Errors: =
         *
         *  * {@link FontError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param value_mask value_mask
         * @param value_list Values for each of the components specified in the bitmask `value_mask`. The
         *                   order has to correspond to the order of possible `value_mask` bits. See the
         *                   example.
         */
        [CCode (cname = "xcb_change_gc_checked", instance_pos = 1.1)]
        public VoidCookie change_checked (Xcb.Connection connection, uint32 value_mask = 0, [CCode (array_length = false)]uint32[]? value_list = null);
        [CCode (cname = "xcb_copy_gc", instance_pos = 1.1)]
        public VoidCookie copy (Xcb.Connection connection, GContext dst_gc, GC value_mask);
        [CCode (cname = "xcb_copy_gc_checked", instance_pos = 1.1)]
        public VoidCookie copy_checked (Xcb.Connection connection, GContext dst_gc, GC value_mask);
        [CCode (cname = "xcb_set_dashes", instance_pos = 1.1)]
        public VoidCookie set_dashes (Xcb.Connection connection, uint16 dash_offset, [CCode (array_length_pos = 2.3)]uint8[]? dashes);
        [CCode (cname = "xcb_set_dashes_checked", instance_pos = 1.1)]
        public VoidCookie set_dashes_checked (Xcb.Connection connection, uint16 dash_offset, [CCode (array_length_pos = 2.3)]uint8[]? dashes);
        [CCode (cname = "xcb_set_clip_rectangles", instance_pos = 1.2)]
        public VoidCookie set_clip_rectangles (Xcb.Connection connection, ClipOrdering ordering, int16 clip_x_origin, int16 clip_y_origin, [CCode (array_length_pos = 4.4)]Rectangle[]? rectangles);
        [CCode (cname = "xcb_set_clip_rectangles_checked", instance_pos = 1.2)]
        public VoidCookie set_clip_rectangles_checked (Xcb.Connection connection, ClipOrdering ordering, int16 clip_x_origin, int16 clip_y_origin, [CCode (array_length_pos = 4.4)]Rectangle[]? rectangles);
        /**
         * Destroys a graphics context
         *
         * Destroys the specified `gc` and all associated storage.
         *
         * = Errors: =
         *
         *  * {@link GContextError}:  The specified graphics context does not exist.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_free_gc", instance_pos = 1.1)]
        public VoidCookie free (Xcb.Connection connection);
        /**
         * Destroys a graphics context
         *
         * Destroys the specified `gc` and all associated storage.
         *
         * = Errors: =
         *
         *  * {@link GContextError}:  The specified graphics context does not exist.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_free_gc_checked", instance_pos = 1.1)]
        public VoidCookie free_checked (Xcb.Connection connection);
    }

    [SimpleType, CCode (cname = "xcb_colormap_iterator_t")]
    struct _ColormapIterator
    {
        int rem;
        int index;
        unowned Colormap? data;
    }

    [CCode (cname = "xcb_colormap_iterator_t")]
    public struct ColormapIterator
    {
        [CCode (cname = "xcb_colormap_next")]
        void _next ();

        public inline unowned Colormap?
        next_value ()
        {
            if (((_ColormapIterator)this).rem > 0)
            {
                unowned Colormap? d = ((_ColormapIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [CCode (cname = "xcb_colormap_t", has_type_id = false)]
    public struct Colormap : uint32 {
        /**
         * Allocates an XID for a new Colormap.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_generate_id")]
        public Colormap (Xcb.Connection connection);

        [CCode (cname = "xcb_create_colormap", instance_pos = 2.2)]
        public VoidCookie create (Xcb.Connection connection, ColormapAlloc alloc, Window window, Visualid visual);
        [CCode (cname = "xcb_create_colormap_checked", instance_pos = 2.2)]
        public VoidCookie create_checked (Xcb.Connection connection, ColormapAlloc alloc, Window window, Visualid visual);
        [CCode (cname = "xcb_free_colormap", instance_pos = 1.1)]
        public VoidCookie free (Xcb.Connection connection);
        [CCode (cname = "xcb_free_colormap_checked", instance_pos = 1.1)]
        public VoidCookie free_checked (Xcb.Connection connection);
        [CCode (cname = "xcb_copy_colormap_and_free", instance_pos = 1.1)]
        public VoidCookie copy_and_free (Xcb.Connection connection, Colormap src_cmap);
        [CCode (cname = "xcb_copy_colormap_and_free_checked", instance_pos = 1.1)]
        public VoidCookie copy_and_free_checked (Xcb.Connection connection, Colormap src_cmap);
        [CCode (cname = "xcb_install_colormap", instance_pos = 1.1)]
        public VoidCookie install (Xcb.Connection connection);
        [CCode (cname = "xcb_install_colormap_checked", instance_pos = 1.1)]
        public VoidCookie install_checked (Xcb.Connection connection);
        [CCode (cname = "xcb_uninstall_colormap", instance_pos = 1.1)]
        public VoidCookie uninstall (Xcb.Connection connection);
        [CCode (cname = "xcb_uninstall_colormap_checked", instance_pos = 1.1)]
        public VoidCookie uninstall_checked (Xcb.Connection connection);
        /**
         * Allocate a color
         *
         * Allocates a read-only colormap entry corresponding to the closest RGB value
         * supported by the hardware. If you are using TrueColor, you can take a shortcut
         * and directly calculate the color pixel value to avoid the round trip. But, for
         * example, on 16-bit color setups (VNC), you can easily get the closest supported
         * RGB value to the RGB value you are specifying.
         *
         * = Errors: =
         *
         *  * {@link ColormapError}:  The specified colormap `cmap` does not exist.
         *
         * @param connection The connection.
         * @param red The red value of your color.
         * @param green The green value of your color.
         * @param blue The blue value of your color.
         */
        [CCode (cname = "xcb_alloc_color", instance_pos = 1.1)]
        public AllocColorCookie alloc_color (Xcb.Connection connection, uint16 red, uint16 green, uint16 blue);
        [CCode (cname = "xcb_alloc_named_color", instance_pos = 1.1)]
        public AllocNamedColorCookie alloc_named_color (Xcb.Connection connection, [CCode (array_length_pos = 1.2)]char[]? name);
        [CCode (cname = "xcb_alloc_color_cells", instance_pos = 2.2)]
        public AllocColorCellsCookie alloc_color_cells (Xcb.Connection connection, bool contiguous, uint16 colors, uint16 planes);
        [CCode (cname = "xcb_alloc_color_planes", instance_pos = 2.2)]
        public AllocColorPlanesCookie alloc_color_planes (Xcb.Connection connection, bool contiguous, uint16 colors, uint16 reds, uint16 greens, uint16 blues);
        [CCode (cname = "xcb_free_colors", instance_pos = 1.1)]
        public VoidCookie free_colors (Xcb.Connection connection, uint32 plane_mask, [CCode (array_length_pos = 2.2)]uint32[]? pixels);
        [CCode (cname = "xcb_free_colors_checked", instance_pos = 1.1)]
        public VoidCookie free_colors_checked (Xcb.Connection connection, uint32 plane_mask, [CCode (array_length_pos = 2.2)]uint32[]? pixels);
        [CCode (cname = "xcb_store_colors", instance_pos = 1.1)]
        public VoidCookie store_colors (Xcb.Connection connection, [CCode (array_length_pos = 1.1)]Coloritem[]? items);
        [CCode (cname = "xcb_store_colors_checked", instance_pos = 1.1)]
        public VoidCookie store_colors_checked (Xcb.Connection connection, [CCode (array_length_pos = 1.1)]Coloritem[]? items);
        [CCode (cname = "xcb_store_named_color", instance_pos = 2.2)]
        public VoidCookie store_named_color (Xcb.Connection connection, ColorFlag flags, uint32 pixel, [CCode (array_length_pos = 3.4)]char[]? name);
        [CCode (cname = "xcb_store_named_color_checked", instance_pos = 2.2)]
        public VoidCookie store_named_color_checked (Xcb.Connection connection, ColorFlag flags, uint32 pixel, [CCode (array_length_pos = 3.4)]char[]? name);
        [CCode (cname = "xcb_query_colors", instance_pos = 1.1)]
        public QueryColorsCookie query_colors (Xcb.Connection connection, [CCode (array_length_pos = 1.1)]uint32[]? pixels);
        [CCode (cname = "xcb_lookup_color", instance_pos = 1.1)]
        public LookupColorCookie lookup_color (Xcb.Connection connection, [CCode (array_length_pos = 1.2)]char[]? name);
    }

    [Compact, CCode (cname = "xcb_alloc_color_reply_t", free_function = "free")]
    public class AllocColorReply {
        public uint16 red;
        public uint16 green;
        public uint16 blue;
        public uint32 pixel;
    }

    [SimpleType, CCode (cname = "xcb_alloc_color_cookie_t")]
    public struct AllocColorCookie : VoidCookie {
        [CCode (cname = "xcb_alloc_color_reply", instance_pos = 1.1)]
        public AllocColorReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_alloc_named_color_reply_t", free_function = "free")]
    public class AllocNamedColorReply {
        public uint32 pixel;
        public uint16 exact_red;
        public uint16 exact_green;
        public uint16 exact_blue;
        public uint16 visual_red;
        public uint16 visual_green;
        public uint16 visual_blue;
    }

    [SimpleType, CCode (cname = "xcb_alloc_named_color_cookie_t")]
    public struct AllocNamedColorCookie : VoidCookie {
        [CCode (cname = "xcb_alloc_named_color_reply", instance_pos = 1.1)]
        public AllocNamedColorReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_alloc_color_cells_reply_t", free_function = "free")]
    public class AllocColorCellsReply {
        public uint16 pixels_len;
        public uint16 masks_len;
        public int pixels_length {
            [CCode (cname = "xcb_alloc_color_cells_pixels_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned uint32[] pixels {
            [CCode (cname = "xcb_alloc_color_cells_pixels")]
            get;
        }
        public int masks_length {
            [CCode (cname = "xcb_alloc_color_cells_masks_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned uint32[] masks {
            [CCode (cname = "xcb_alloc_color_cells_masks")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_alloc_color_cells_cookie_t")]
    public struct AllocColorCellsCookie : VoidCookie {
        [CCode (cname = "xcb_alloc_color_cells_reply", instance_pos = 1.1)]
        public AllocColorCellsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_alloc_color_planes_reply_t", free_function = "free")]
    public class AllocColorPlanesReply {
        public uint16 pixels_len;
        public uint32 red_mask;
        public uint32 green_mask;
        public uint32 blue_mask;
        public int pixels_length {
            [CCode (cname = "xcb_alloc_color_planes_pixels_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned uint32[] pixels {
            [CCode (cname = "xcb_alloc_color_planes_pixels")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_alloc_color_planes_cookie_t")]
    public struct AllocColorPlanesCookie : VoidCookie {
        [CCode (cname = "xcb_alloc_color_planes_reply", instance_pos = 1.1)]
        public AllocColorPlanesReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_query_colors_reply_t", free_function = "free")]
    public class QueryColorsReply {
        public uint16 colors_len;
        [CCode (cname = "xcb_query_colors_colors_iterator")]
        _RgbIterator _iterator ();
        public RgbIterator iterator () {
            return (RgbIterator) _iterator ();
        }
        public int colors_length {
            [CCode (cname = "xcb_query_colors_colors_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Rgb[] colors {
            [CCode (cname = "xcb_query_colors_colors")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_query_colors_cookie_t")]
    public struct QueryColorsCookie : VoidCookie {
        [CCode (cname = "xcb_query_colors_reply", instance_pos = 1.1)]
        public QueryColorsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_lookup_color_reply_t", free_function = "free")]
    public class LookupColorReply {
        public uint16 exact_red;
        public uint16 exact_green;
        public uint16 exact_blue;
        public uint16 visual_red;
        public uint16 visual_green;
        public uint16 visual_blue;
    }

    [SimpleType, CCode (cname = "xcb_lookup_color_cookie_t")]
    public struct LookupColorCookie : VoidCookie {
        [CCode (cname = "xcb_lookup_color_reply", instance_pos = 1.1)]
        public LookupColorReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [SimpleType, CCode (cname = "xcb_atom_iterator_t")]
    struct _AtomIterator
    {
        int rem;
        int index;
        unowned Atom? data;
    }

    [CCode (cname = "xcb_atom_iterator_t")]
    public struct AtomIterator
    {
        [CCode (cname = "xcb_atom_next")]
        void _next ();

        public inline unowned Atom?
        next_value ()
        {
            if (((_AtomIterator)this).rem > 0)
            {
                unowned Atom? d = ((_AtomIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [CCode (cname = "xcb_atom_t", has_type_id = false)]
    public struct Atom : uint32 {
        [CCode (cname = "xcb_get_atom_name", instance_pos = 1.1)]
        public GetAtomNameCookie get_name (Xcb.Connection connection);
        /**
         * Gets the owner of a selection
         *
         * Gets the owner of the specified selection.
         *
         * TODO: briefly explain what a selection is.
         *
         * = Errors: =
         *
         *  * {@link AtomError}:  `selection` does not refer to a valid atom.
         *
         * @param connection The connection.
         *
         * @see Window.set_selection_owner
         */
        [CCode (cname = "xcb_get_selection_owner", instance_pos = 1.1)]
        public GetSelectionOwnerCookie get_selection_owner (Xcb.Connection connection);
    }

    [Compact, CCode (cname = "xcb_get_atom_name_reply_t", free_function = "free")]
    public class GetAtomNameReply {
        public uint16 name_len;
        [CCode (cname = "xcb_get_atom_name_name_length")]
        int _name_length ();
        [CCode (cname = "xcb_get_atom_name_name", array_length = false)]
        unowned char[] _name ();
        public string name {
            owned get {
                GLib.StringBuilder ret = new GLib.StringBuilder ();
                ret.append_len ((string)_name (), _name_length ());
                return ret.str;
            }
        }
    }

    [SimpleType, CCode (cname = "xcb_get_atom_name_cookie_t")]
    public struct GetAtomNameCookie : VoidCookie {
        [CCode (cname = "xcb_get_atom_name_reply", instance_pos = 1.1)]
        public GetAtomNameReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_selection_owner_reply_t", free_function = "free")]
    public class GetSelectionOwnerReply {
        public Window owner;
    }

    [SimpleType, CCode (cname = "xcb_get_selection_owner_cookie_t")]
    public struct GetSelectionOwnerCookie : VoidCookie {
        [CCode (cname = "xcb_get_selection_owner_reply", instance_pos = 1.1)]
        public GetSelectionOwnerReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [CCode (cname = "xcb_drawable_t", has_type_id = false)]
    public struct Drawable : uint32 {
        /**
         * Get current window geometry
         *
         * Gets the current geometry of the specified drawable (either `Window` or `Pixmap`).
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  TODO: reasons?
         *
         * @param connection The connection.
         *
         */
        [CCode (cname = "xcb_get_geometry", instance_pos = 1.1)]
        public GetGeometryCookie get_geometry (Xcb.Connection connection);
        /**
         * copy areas
         *
         * Copies the specified rectangle from `src_drawable` to `dst_drawable`.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param dst_drawable The destination drawable (Window or Pixmap).
         * @param gc The graphics context to use.
         * @param src_x The source X coordinate.
         * @param src_y The source Y coordinate.
         * @param dst_x The destination X coordinate.
         * @param dst_y The destination Y coordinate.
         * @param width The width of the area to copy (in pixels).
         * @param height The height of the area to copy (in pixels).
         */
        [CCode (cname = "xcb_copy_area", instance_pos = 1.1)]
        public VoidCookie copy_area (Xcb.Connection connection, Drawable dst_drawable, GContext gc, int16 src_x, int16 src_y, int16 dst_x, int16 dst_y, uint16 width, uint16 height);
        /**
         * copy areas
         *
         * Copies the specified rectangle from `src_drawable` to `dst_drawable`.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param dst_drawable The destination drawable (Window or Pixmap).
         * @param gc The graphics context to use.
         * @param src_x The source X coordinate.
         * @param src_y The source Y coordinate.
         * @param dst_x The destination X coordinate.
         * @param dst_y The destination Y coordinate.
         * @param width The width of the area to copy (in pixels).
         * @param height The height of the area to copy (in pixels).
         */
        [CCode (cname = "xcb_copy_area_checked", instance_pos = 1.1)]
        public VoidCookie copy_area_checked (Xcb.Connection connection, Drawable dst_drawable, GContext gc, int16 src_x, int16 src_y, int16 dst_x, int16 dst_y, uint16 width, uint16 height);
        [CCode (cname = "xcb_copy_plane", instance_pos = 1.1)]
        public VoidCookie copy_plane (Xcb.Connection connection, Drawable dst_drawable, GContext gc, int16 src_x, int16 src_y, int16 dst_x, int16 dst_y, uint16 width, uint16 height, uint32 bit_plane);
        [CCode (cname = "xcb_copy_plane_checked", instance_pos = 1.1)]
        public VoidCookie copy_plane_checked (Xcb.Connection connection, Drawable dst_drawable, GContext gc, int16 src_x, int16 src_y, int16 dst_x, int16 dst_y, uint16 width, uint16 height, uint32 bit_plane);
        [CCode (cname = "xcb_poly_point", instance_pos = 1.2)]
        public VoidCookie poly_point (Xcb.Connection connection, CoordMode coordinate_mode, GContext gc, [CCode (array_length_pos = 3.3)]Point[]? points);
        [CCode (cname = "xcb_poly_point_checked", instance_pos = 1.2)]
        public VoidCookie poly_point_checked (Xcb.Connection connection, CoordMode coordinate_mode, GContext gc, [CCode (array_length_pos = 3.3)]Point[]? points);
        /**
         * draw lines
         *
         * Draws `points_len`-1 lines between each pair of points (point[i], point[i+1])
         * in the `points` array. The lines are drawn in the order listed in the array.
         * They join correctly at all intermediate points, and if the first and last
         * points coincide, the first and last lines also join correctly. For any given
         * line, a pixel is not drawn more than once. If thin (zero line-width) lines
         * intersect, the intersecting pixels are drawn multiple times. If wide lines
         * intersect, the intersecting pixels are drawn only once, as though the entire
         * request were a single, filled shape.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param coordinate_mode coordinate_mode
         * @param gc The graphics context to use.
         * @param points An array of points.
         */
        [CCode (cname = "xcb_poly_line", instance_pos = 1.2)]
        public VoidCookie poly_line (Xcb.Connection connection, CoordMode coordinate_mode, GContext gc, [CCode (array_length_pos = 3.3)]Point[]? points);
        /**
         * draw lines
         *
         * Draws `points_len`-1 lines between each pair of points (point[i], point[i+1])
         * in the `points` array. The lines are drawn in the order listed in the array.
         * They join correctly at all intermediate points, and if the first and last
         * points coincide, the first and last lines also join correctly. For any given
         * line, a pixel is not drawn more than once. If thin (zero line-width) lines
         * intersect, the intersecting pixels are drawn multiple times. If wide lines
         * intersect, the intersecting pixels are drawn only once, as though the entire
         * request were a single, filled shape.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  TODO: reasons?
         *
         * @param connection The connection.
         * @param coordinate_mode coordinate_mode
         * @param gc The graphics context to use.
         * @param points An array of points.
         */
        [CCode (cname = "xcb_poly_line_checked", instance_pos = 1.2)]
        public VoidCookie poly_line_checked (Xcb.Connection connection, CoordMode coordinate_mode, GContext gc, [CCode (array_length_pos = 3.3)]Point[]? points);
        /**
         * draw lines
         *
         * Draws multiple, unconnected lines. For each segment, a line is drawn between
         * (x1, y1) and (x2, y2). The lines are drawn in the order listed in the array of
         * `xcb_segment_t` structures and does not perform joining at coincident
         * endpoints. For any given line, a pixel is not drawn more than once. If lines
         * intersect, the intersecting pixels are drawn multiple times.
         *
         * TODO: include the xcb_segment_t data structure
         *
         * TODO: an example
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` does not exist.
         *
         * @param connection The connection.
         * @param gc The graphics context to use.
         *           TODO: document which attributes of a gc are used
         * @param segments An array of `xcb_segment_t` structures.
         */
        [CCode (cname = "xcb_poly_segment", instance_pos = 1.1)]
        public VoidCookie poly_segment (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Segment[]? segments);
        /**
         * draw lines
         *
         * Draws multiple, unconnected lines. For each segment, a line is drawn between
         * (x1, y1) and (x2, y2). The lines are drawn in the order listed in the array of
         * `xcb_segment_t` structures and does not perform joining at coincident
         * endpoints. For any given line, a pixel is not drawn more than once. If lines
         * intersect, the intersecting pixels are drawn multiple times.
         *
         * TODO: include the xcb_segment_t data structure
         *
         * TODO: an example
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` does not exist.
         *
         * @param connection The connection.
         * @param gc The graphics context to use.
         *           TODO: document which attributes of a gc are used
         * @param segments An array of `xcb_segment_t` structures.
         */
        [CCode (cname = "xcb_poly_segment_checked", instance_pos = 1.1)]
        public VoidCookie poly_segment_checked (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Segment[]? segments);
        [CCode (cname = "xcb_poly_rectangle", instance_pos = 1.1)]
        public VoidCookie poly_rectangle (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Rectangle[]? rectangles);
        [CCode (cname = "xcb_poly_rectangle_checked", instance_pos = 1.1)]
        public VoidCookie poly_rectangle_checked (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Rectangle[]? rectangles);
        [CCode (cname = "xcb_poly_arc", instance_pos = 1.1)]
        public VoidCookie poly_arc (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Arc[]? arcs);
        [CCode (cname = "xcb_poly_arc_checked", instance_pos = 1.1)]
        public VoidCookie poly_arc_checked (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Arc[]? arcs);
        [CCode (cname = "xcb_fill_poly", instance_pos = 1.1)]
        public VoidCookie fill_poly (Xcb.Connection connection, GContext gc, PolyShape shape, CoordMode coordinate_mode, [CCode (array_length_pos = 4.4)]Point[]? points);
        [CCode (cname = "xcb_fill_poly_checked", instance_pos = 1.1)]
        public VoidCookie fill_poly_checked (Xcb.Connection connection, GContext gc, PolyShape shape, CoordMode coordinate_mode, [CCode (array_length_pos = 4.4)]Point[]? points);
        /**
         * Fills rectangles
         *
         * Fills the specified rectangle(s) in the order listed in the array. For any
         * given rectangle, each pixel is not drawn more than once. If rectangles
         * intersect, the intersecting pixels are drawn multiple times.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param gc The graphics context to use.
         *           The following graphics context components are used: function, plane-mask,
         *           fill-style, subwindow-mode, clip-x-origin, clip-y-origin, and clip-mask.
         *           The following graphics context mode-dependent components are used:
         *           foreground, background, tile, stipple, tile-stipple-x-origin, and
         *           tile-stipple-y-origin.
         * @param rectangles The rectangles to fill.
         */
        [CCode (cname = "xcb_poly_fill_rectangle", instance_pos = 1.1)]
        public VoidCookie poly_fill_rectangle (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Rectangle[]? rectangles);
        /**
         * Fills rectangles
         *
         * Fills the specified rectangle(s) in the order listed in the array. For any
         * given rectangle, each pixel is not drawn more than once. If rectangles
         * intersect, the intersecting pixels are drawn multiple times.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param gc The graphics context to use.
         *           The following graphics context components are used: function, plane-mask,
         *           fill-style, subwindow-mode, clip-x-origin, clip-y-origin, and clip-mask.
         *           The following graphics context mode-dependent components are used:
         *           foreground, background, tile, stipple, tile-stipple-x-origin, and
         *           tile-stipple-y-origin.
         * @param rectangles The rectangles to fill.
         */
        [CCode (cname = "xcb_poly_fill_rectangle_checked", instance_pos = 1.1)]
        public VoidCookie poly_fill_rectangle_checked (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Rectangle[]? rectangles);
        [CCode (cname = "xcb_poly_fill_arc", instance_pos = 1.1)]
        public VoidCookie poly_fill_arc (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Arc[]? arcs);
        [CCode (cname = "xcb_poly_fill_arc_checked", instance_pos = 1.1)]
        public VoidCookie poly_fill_arc_checked (Xcb.Connection connection, GContext gc, [CCode (array_length_pos = 2.2)]Arc[]? arcs);
        [CCode (cname = "xcb_put_image", instance_pos = 1.2)]
        public VoidCookie put_image (Xcb.Connection connection, ImageFormat format, GContext gc, uint16 width, uint16 height, int16 dst_x, int16 dst_y, uint8 left_pad, uint8 depth, [CCode (array_length_pos = 9.9)]uint8[]? data);
        [CCode (cname = "xcb_put_image_checked", instance_pos = 1.2)]
        public VoidCookie put_image_checked (Xcb.Connection connection, ImageFormat format, GContext gc, uint16 width, uint16 height, int16 dst_x, int16 dst_y, uint8 left_pad, uint8 depth, [CCode (array_length_pos = 9.9)]uint8[]? data);
        [CCode (cname = "xcb_get_image", instance_pos = 2.2)]
        public GetImageCookie get_image (Xcb.Connection connection, ImageFormat format, int16 x, int16 y, uint16 width, uint16 height, uint32 plane_mask);
        [CCode (cname = "xcb_poly_text_8", instance_pos = 1.1)]
        public VoidCookie poly_text_8 (Xcb.Connection connection, GContext gc, int16 x, int16 y, [CCode (array_length_pos = 4.4)]uint8[]? items);
        [CCode (cname = "xcb_poly_text_8_checked", instance_pos = 1.1)]
        public VoidCookie poly_text_8_checked (Xcb.Connection connection, GContext gc, int16 x, int16 y, [CCode (array_length_pos = 4.4)]uint8[]? items);
        [CCode (cname = "xcb_poly_text_16", instance_pos = 1.1)]
        public VoidCookie poly_text_16 (Xcb.Connection connection, GContext gc, int16 x, int16 y, [CCode (array_length_pos = 4.4)]uint8[]? items);
        [CCode (cname = "xcb_poly_text_16_checked", instance_pos = 1.1)]
        public VoidCookie poly_text_16_checked (Xcb.Connection connection, GContext gc, int16 x, int16 y, [CCode (array_length_pos = 4.4)]uint8[]? items);
        /**
         * Draws text
         *
         * Fills the destination rectangle with the background pixel from `gc`, then
         * paints the text with the foreground pixel from `gc`. The upper-left corner of
         * the filled rectangle is at [x, y - font-ascent]. The width is overall-width,
         * the height is font-ascent + font-descent. The overall-width, font-ascent and
         * font-descent are as returned by `xcb_query_text_extents` (TODO).
         *
         * Note that using X core fonts is deprecated (but still supported) in favor of
         * client-side rendering using Xft.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param gc The graphics context to use.
         *           The following graphics context components are used: plane-mask, foreground,
         *           background, font, subwindow-mode, clip-x-origin, clip-y-origin, and clip-mask.
         * @param x The x coordinate of the first character, relative to the origin of `drawable`.
         * @param y The y coordinate of the first character, relative to the origin of `drawable`.
         * @param string The string to draw. Only the first 255 characters are relevant due to the data
         *               type of `string_len`.
         *
         * @see Drawable.image_text_16
         */
        [CCode (cname = "xcb_image_text_8", instance_pos = 1.2)]
        public VoidCookie image_text_8 (Xcb.Connection connection, GContext gc, int16 x, int16 y, [CCode (array_length_pos = 1.1)]char[]? string);
        /**
         * Draws text
         *
         * Fills the destination rectangle with the background pixel from `gc`, then
         * paints the text with the foreground pixel from `gc`. The upper-left corner of
         * the filled rectangle is at [x, y - font-ascent]. The width is overall-width,
         * the height is font-ascent + font-descent. The overall-width, font-ascent and
         * font-descent are as returned by `xcb_query_text_extents` (TODO).
         *
         * Note that using X core fonts is deprecated (but still supported) in favor of
         * client-side rendering using Xft.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param gc The graphics context to use.
         *           The following graphics context components are used: plane-mask, foreground,
         *           background, font, subwindow-mode, clip-x-origin, clip-y-origin, and clip-mask.
         * @param x The x coordinate of the first character, relative to the origin of `drawable`.
         * @param y The y coordinate of the first character, relative to the origin of `drawable`.
         * @param string The string to draw. Only the first 255 characters are relevant due to the data
         *               type of `string_len`.
         *
         * @see Drawable.image_text_16
         */
        [CCode (cname = "xcb_image_text_8_checked", instance_pos = 1.2)]
        public VoidCookie image_text_8_checked (Xcb.Connection connection, GContext gc, int16 x, int16 y, [CCode (array_length_pos = 1.1)]char[]? string);
        /**
         * Draws text
         *
         * Fills the destination rectangle with the background pixel from `gc`, then
         * paints the text with the foreground pixel from `gc`. The upper-left corner of
         * the filled rectangle is at [x, y - font-ascent]. The width is overall-width,
         * the height is font-ascent + font-descent. The overall-width, font-ascent and
         * font-descent are as returned by `xcb_query_text_extents` (TODO).
         *
         * Note that using X core fonts is deprecated (but still supported) in favor of
         * client-side rendering using Xft.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param gc The graphics context to use.
         *           The following graphics context components are used: plane-mask, foreground,
         *           background, font, subwindow-mode, clip-x-origin, clip-y-origin, and clip-mask.
         * @param x The x coordinate of the first character, relative to the origin of `drawable`.
         * @param y The y coordinate of the first character, relative to the origin of `drawable`.
         * @param string The string to draw. Only the first 255 characters are relevant due to the data
         *               type of `string_len`. Every character uses 2 bytes (hence the 16 in this
         *               request's name).
         *
         * @see Drawable.image_text_8
         */
        [CCode (cname = "xcb_image_text_16", instance_pos = 1.2)]
        public VoidCookie image_text_16 (Xcb.Connection connection, GContext gc, int16 x, int16 y, [CCode (array_length_pos = 1.1)]Char2b[]? string);
        /**
         * Draws text
         *
         * Fills the destination rectangle with the background pixel from `gc`, then
         * paints the text with the foreground pixel from `gc`. The upper-left corner of
         * the filled rectangle is at [x, y - font-ascent]. The width is overall-width,
         * the height is font-ascent + font-descent. The overall-width, font-ascent and
         * font-descent are as returned by `xcb_query_text_extents` (TODO).
         *
         * Note that using X core fonts is deprecated (but still supported) in favor of
         * client-side rendering using Xft.
         *
         * = Errors: =
         *
         *  * {@link DrawableError}:  The specified `drawable` (Window or Pixmap) does not exist.
         *
         * @param connection The connection.
         * @param gc The graphics context to use.
         *           The following graphics context components are used: plane-mask, foreground,
         *           background, font, subwindow-mode, clip-x-origin, clip-y-origin, and clip-mask.
         * @param x The x coordinate of the first character, relative to the origin of `drawable`.
         * @param y The y coordinate of the first character, relative to the origin of `drawable`.
         * @param string The string to draw. Only the first 255 characters are relevant due to the data
         *               type of `string_len`. Every character uses 2 bytes (hence the 16 in this
         *               request's name).
         *
         * @see Drawable.image_text_8
         */
        [CCode (cname = "xcb_image_text_16_checked", instance_pos = 1.2)]
        public VoidCookie image_text_16_checked (Xcb.Connection connection, GContext gc, int16 x, int16 y, [CCode (array_length_pos = 1.1)]Char2b[]? string);
        [CCode (cname = "xcb_query_best_size", instance_pos = 2.2)]
        public QueryBestSizeCookie query_best_size (Xcb.Connection connection, QueryShapeOf _class, uint16 width, uint16 height);
    }

    [Compact, CCode (cname = "xcb_get_geometry_reply_t", free_function = "free")]
    public class GetGeometryReply {
        public uint8 depth;
        public Window root;
        public int16 x;
        public int16 y;
        public uint16 width;
        public uint16 height;
        public uint16 border_width;
    }

    [SimpleType, CCode (cname = "xcb_get_geometry_cookie_t")]
    public struct GetGeometryCookie : VoidCookie {
        [CCode (cname = "xcb_get_geometry_reply", instance_pos = 1.1)]
        public GetGeometryReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_get_image_reply_t", free_function = "free")]
    public class GetImageReply {
        public uint8 depth;
        public Visualid visual;
        public int data_length {
            [CCode (cname = "xcb_get_image_data_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned uint8[] data {
            [CCode (cname = "xcb_get_image_data")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_get_image_cookie_t")]
    public struct GetImageCookie : VoidCookie {
        [CCode (cname = "xcb_get_image_reply", instance_pos = 1.1)]
        public GetImageReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_query_best_size_reply_t", free_function = "free")]
    public class QueryBestSizeReply {
        public uint16 width;
        public uint16 height;
    }

    [SimpleType, CCode (cname = "xcb_query_best_size_cookie_t")]
    public struct QueryBestSizeCookie : VoidCookie {
        [CCode (cname = "xcb_query_best_size_reply", instance_pos = 1.1)]
        public QueryBestSizeReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [CCode (cname = "xcb_fontable_t", has_type_id = false)]
    public struct Fontable : uint32 {
        /**
         * query font metrics
         *
         * Queries information associated with the font.
         *
         * @param connection The connection.
         */
        [CCode (cname = "xcb_query_font", instance_pos = 1.1)]
        public QueryFontCookie query_font (Xcb.Connection connection);
        /**
         * get text extents
         *
         * Query text extents from the X11 server. This request returns the bounding box
         * of the specified 16-bit character string in the specified `font` or the font
         * contained in the specified graphics context.
         *
         * `font_ascent` is set to the maximum of the ascent metrics of all characters in
         * the string. `font_descent` is set to the maximum of the descent metrics.
         * `overall_width` is set to the sum of the character-width metrics of all
         * characters in the string. For each character in the string, let W be the sum of
         * the character-width metrics of all characters preceding it in the string. Let L
         * be the left-side-bearing metric of the character plus W. Let R be the
         * right-side-bearing metric of the character plus W. The lbearing member is set
         * to the minimum L of all characters in the string. The rbearing member is set to
         * the maximum R.
         *
         * For fonts defined with linear indexing rather than 2-byte matrix indexing, each
         * `xcb_char2b_t` structure is interpreted as a 16-bit number with byte1 as the
         * most significant byte. If the font has no defined default character, undefined
         * characters in the string are taken to have all zero metrics.
         *
         * Characters with all zero metrics are ignored. If the font has no defined
         * default_char, the undefined characters in the string are also ignored.
         *
         * = Errors: =
         *
         *  * {@link GContextError}:  The specified graphics context does not exist.
         *
         * @param connection The connection.
         * @param string The text to get text extents for.
         */
        [CCode (cname = "xcb_query_text_extents", instance_pos = 1.1)]
        public QueryTextExtentsCookie query_text_extents (Xcb.Connection connection, [CCode (array_length_pos = 1.1)]Char2b[]? string);
    }

    [Compact, CCode (cname = "xcb_query_font_reply_t", free_function = "free")]
    public class QueryFontReply {
        public Charinfo min_bounds;
        public Charinfo max_bounds;
        public uint16 min_char_or_byte2;
        public uint16 max_char_or_byte2;
        public uint16 default_char;
        public uint16 properties_len;
        public FontDraw draw_direction;
        public uint8 min_byte1;
        public uint8 max_byte1;
        public bool all_chars_exist;
        public int16 font_ascent;
        public int16 font_descent;
        public uint32 char_infos_len;
        public int properties_length {
            [CCode (cname = "xcb_query_font_properties_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Fontprop[] properties {
            [CCode (cname = "xcb_query_font_properties")]
            get;
        }
        [CCode (cname = "xcb_query_font_char_infos_iterator")]
        _CharinfoIterator _iterator ();
        public CharinfoIterator iterator () {
            return (CharinfoIterator) _iterator ();
        }
        public int char_infos_length {
            [CCode (cname = "xcb_query_font_char_infos_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Charinfo[] char_infos {
            [CCode (cname = "xcb_query_font_char_infos")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_query_font_cookie_t")]
    public struct QueryFontCookie : VoidCookie {
        [CCode (cname = "xcb_query_font_reply", instance_pos = 1.1)]
        public QueryFontReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_query_text_extents_reply_t", free_function = "free")]
    public class QueryTextExtentsReply {
        public FontDraw draw_direction;
        public int16 font_ascent;
        public int16 font_descent;
        public int16 overall_ascent;
        public int16 overall_descent;
        public int32 overall_width;
        public int32 overall_left;
        public int32 overall_right;
    }

    [SimpleType, CCode (cname = "xcb_query_text_extents_cookie_t")]
    public struct QueryTextExtentsCookie : VoidCookie {
        [CCode (cname = "xcb_query_text_extents_reply", instance_pos = 1.1)]
        public QueryTextExtentsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [SimpleType, CCode (cname = "xcb_visualid_t", has_type_id = false)]
    public struct Visualid : uint32 {
    }

    [SimpleType, CCode (cname = "xcb_timestamp_t", has_type_id = false)]
    public struct Timestamp : uint32 {
    }

    [SimpleType, CCode (cname = "xcb_keysym_t", has_type_id = false)]
    public struct Keysym : uint32 {
    }

    [SimpleType, CCode (cname = "xcb_keycode_t", has_type_id = false)]
    public struct Keycode : uint8 {
    }

    [SimpleType, CCode (cname = "xcb_button_t", has_type_id = false)]
    public struct Button : uint8 {
    }

    [SimpleType, CCode (cname = "xcb_point_iterator_t")]
    struct _PointIterator
    {
        int rem;
        int index;
        unowned Point? data;
    }

    [CCode (cname = "xcb_point_iterator_t")]
    public struct PointIterator
    {
        [CCode (cname = "xcb_point_next")]
        void _next ();

        public inline unowned Point?
        next_value ()
        {
            if (((_PointIterator)this).rem > 0)
            {
                unowned Point? d = ((_PointIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_point_t", has_type_id = false)]
    public struct Point {
        public int16 x;
        public int16 y;
    }

    [SimpleType, CCode (cname = "xcb_rectangle_iterator_t")]
    struct _RectangleIterator
    {
        int rem;
        int index;
        unowned Rectangle? data;
    }

    [CCode (cname = "xcb_rectangle_iterator_t")]
    public struct RectangleIterator
    {
        [CCode (cname = "xcb_rectangle_next")]
        void _next ();

        public inline unowned Rectangle?
        next_value ()
        {
            if (((_RectangleIterator)this).rem > 0)
            {
                unowned Rectangle? d = ((_RectangleIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_rectangle_t", has_type_id = false)]
    public struct Rectangle {
        public int16 x;
        public int16 y;
        public uint16 width;
        public uint16 height;
    }

    [SimpleType, CCode (cname = "xcb_arc_iterator_t")]
    struct _ArcIterator
    {
        int rem;
        int index;
        unowned Arc? data;
    }

    [CCode (cname = "xcb_arc_iterator_t")]
    public struct ArcIterator
    {
        [CCode (cname = "xcb_arc_next")]
        void _next ();

        public inline unowned Arc?
        next_value ()
        {
            if (((_ArcIterator)this).rem > 0)
            {
                unowned Arc? d = ((_ArcIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_arc_t", has_type_id = false)]
    public struct Arc {
        public int16 x;
        public int16 y;
        public uint16 width;
        public uint16 height;
        public int16 angle1;
        public int16 angle2;
    }

    [SimpleType, CCode (cname = "xcb_format_iterator_t")]
    struct _FormatIterator
    {
        int rem;
        int index;
        unowned Format? data;
    }

    [CCode (cname = "xcb_format_iterator_t")]
    public struct FormatIterator
    {
        [CCode (cname = "xcb_format_next")]
        void _next ();

        public inline unowned Format?
        next_value ()
        {
            if (((_FormatIterator)this).rem > 0)
            {
                unowned Format? d = ((_FormatIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_format_t", has_type_id = false)]
    public struct Format {
        public uint8 depth;
        public uint8 bits_per_pixel;
        public uint8 scanline_pad;
    }

    [CCode (cname = "xcb_visual_class_t", cprefix =  "XCB_VISUAL_CLASS_", has_type_id = false)]
    public enum VisualClass {
        STATIC_GRAY,
        GRAY_SCALE,
        STATIC_COLOR,
        PSEUDO_COLOR,
        TRUE_COLOR,
        DIRECT_COLOR
    }

    [SimpleType, CCode (cname = "xcb_visualtype_iterator_t")]
    struct _VisualtypeIterator
    {
        int rem;
        int index;
        unowned Visualtype? data;
    }

    [CCode (cname = "xcb_visualtype_iterator_t")]
    public struct VisualtypeIterator
    {
        [CCode (cname = "xcb_visualtype_next")]
        void _next ();

        public inline unowned Visualtype?
        next_value ()
        {
            if (((_VisualtypeIterator)this).rem > 0)
            {
                unowned Visualtype? d = ((_VisualtypeIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_visualtype_t", has_type_id = false)]
    public struct Visualtype {
        public Visualid visual_id;
        public VisualClass _class;
        public uint8 bits_per_rgb_value;
        public uint16 colormap_entries;
        public uint32 red_mask;
        public uint32 green_mask;
        public uint32 blue_mask;
    }

    [SimpleType, CCode (cname = "xcb_depth_iterator_t")]
    struct _DepthIterator
    {
        int rem;
        int index;
        unowned Depth? data;
    }

    [CCode (cname = "xcb_depth_iterator_t")]
    public struct DepthIterator
    {
        [CCode (cname = "xcb_depth_next")]
        void _next ();

        public inline unowned Depth?
        next_value ()
        {
            if (((_DepthIterator)this).rem > 0)
            {
                unowned Depth? d = ((_DepthIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_depth_t", has_type_id = false)]
    public struct Depth {
        public uint8 depth;
        public uint16 visuals_len;
        [CCode (cname = "xcb_depth_visuals_iterator")]
        _VisualtypeIterator _iterator ();
        public VisualtypeIterator iterator () {
            return (VisualtypeIterator) _iterator ();
        }
    }

    [Flags, CCode (cname = "xcb_event_mask_t", cprefix =  "XCB_EVENT_MASK_", has_type_id = false)]
    public enum EventMask {
        NO_EVENT,
        KEY_PRESS,
        KEY_RELEASE,
        BUTTON_PRESS,
        BUTTON_RELEASE,
        ENTER_WINDOW,
        LEAVE_WINDOW,
        POINTER_MOTION,
        POINTER_MOTION_HINT,
        BUTTON_1_MOTION,
        BUTTON_2_MOTION,
        BUTTON_3_MOTION,
        BUTTON_4_MOTION,
        BUTTON_5_MOTION,
        BUTTON_MOTION,
        KEYMAP_STATE,
        EXPOSURE,
        VISIBILITY_CHANGE,
        STRUCTURE_NOTIFY,
        RESIZE_REDIRECT,
        SUBSTRUCTURE_NOTIFY,
        SUBSTRUCTURE_REDIRECT,
        FOCUS_CHANGE,
        PROPERTY_CHANGE,
        COLOR_MAP_CHANGE,
        OWNER_GRAB_BUTTON
    }

    [CCode (cname = "xcb_backing_store_t", cprefix =  "XCB_BACKING_STORE_", has_type_id = false)]
    public enum BackingStore {
        NOT_USEFUL,
        WHEN_MAPPED,
        ALWAYS
    }

    [Compact, Immutable, CCode (cname = "xcb_screen_t")]
    public class Screen {
        public Window root;
        public Colormap default_colormap;
        public uint32 white_pixel;
        public uint32 black_pixel;
        public EventMask current_input_masks;
        public uint16 width_in_pixels;
        public uint16 height_in_pixels;
        public uint16 width_in_millimeters;
        public uint16 height_in_millimeters;
        public uint16 min_installed_maps;
        public uint16 max_installed_maps;
        public Visualid root_visual;
        public BackingStore backing_stores;
        public bool save_unders;
        public uint8 root_depth;
        public uint8 allowed_depths_len;
        [CCode (cname = "xcb_screen_allowed_depths_iterator")]
        _DepthIterator _iterator ();
        public DepthIterator iterator () {
            return (DepthIterator) _iterator ();
        }
    }

    [SimpleType, CCode (cname = "xcb_setup_request_iterator_t")]
    struct _SetupRequestIterator
    {
        int rem;
        int index;
        unowned SetupRequest? data;
    }

    [CCode (cname = "xcb_setup_request_iterator_t")]
    public struct SetupRequestIterator
    {
        [CCode (cname = "xcb_setup_request_next")]
        void _next ();

        public inline unowned SetupRequest?
        next_value ()
        {
            if (((_SetupRequestIterator)this).rem > 0)
            {
                unowned SetupRequest? d = ((_SetupRequestIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_setup_request_t", has_type_id = false)]
    public struct SetupRequest {
        public uint8 byte_order;
        public uint16 protocol_major_version;
        public uint16 protocol_minor_version;
        public uint16 authorization_protocol_name_len;
        public uint16 authorization_protocol_data_len;
    }

    [SimpleType, CCode (cname = "xcb_setup_failed_iterator_t")]
    struct _SetupFailedIterator
    {
        int rem;
        int index;
        unowned SetupFailed? data;
    }

    [CCode (cname = "xcb_setup_failed_iterator_t")]
    public struct SetupFailedIterator
    {
        [CCode (cname = "xcb_setup_failed_next")]
        void _next ();

        public inline unowned SetupFailed?
        next_value ()
        {
            if (((_SetupFailedIterator)this).rem > 0)
            {
                unowned SetupFailed? d = ((_SetupFailedIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_setup_failed_t", has_type_id = false)]
    public struct SetupFailed {
        public uint8 status;
        public uint8 reason_len;
        public uint16 protocol_major_version;
        public uint16 protocol_minor_version;
        public uint16 length;
    }

    [SimpleType, CCode (cname = "xcb_setup_authenticate_iterator_t")]
    struct _SetupAuthenticateIterator
    {
        int rem;
        int index;
        unowned SetupAuthenticate? data;
    }

    [CCode (cname = "xcb_setup_authenticate_iterator_t")]
    public struct SetupAuthenticateIterator
    {
        [CCode (cname = "xcb_setup_authenticate_next")]
        void _next ();

        public inline unowned SetupAuthenticate?
        next_value ()
        {
            if (((_SetupAuthenticateIterator)this).rem > 0)
            {
                unowned SetupAuthenticate? d = ((_SetupAuthenticateIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_setup_authenticate_t", has_type_id = false)]
    public struct SetupAuthenticate {
        public uint8 status;
        public uint16 length;
    }

    [CCode (cname = "xcb_image_order_t", cprefix =  "XCB_IMAGE_ORDER_", has_type_id = false)]
    public enum ImageOrder {
        LSB_FIRST,
        MSB_FIRST
    }

    [Compact, Immutable, CCode (cname = "xcb_setup_t")]
    public class Setup {
        public uint8 status;
        public uint16 protocol_major_version;
        public uint16 protocol_minor_version;
        public uint16 length;
        public uint32 release_number;
        public uint32 resource_id_base;
        public uint32 resource_id_mask;
        public uint32 motion_buffer_size;
        public uint16 vendor_len;
        public uint16 maximum_request_length;
        public uint8 roots_len;
        public uint8 pixmap_formats_len;
        public ImageOrder image_byte_order;
        public ImageOrder bitmap_format_bit_order;
        public uint8 bitmap_format_scanline_unit;
        public uint8 bitmap_format_scanline_pad;
        public Keycode min_keycode;
        public Keycode max_keycode;
    }

    [Flags, CCode (cname = "xcb_mod_mask_t", cprefix =  "XCB_MOD_MASK_", has_type_id = false)]
    public enum ModMask {
        SHIFT,
        LOCK,
        CONTROL,
        [CCode (cname = "XCB_MOD_MASK_1")]
        ONE,
        [CCode (cname = "XCB_MOD_MASK_2")]
        TWO,
        [CCode (cname = "XCB_MOD_MASK_3")]
        THREE,
        [CCode (cname = "XCB_MOD_MASK_4")]
        FOUR,
        [CCode (cname = "XCB_MOD_MASK_5")]
        FIVE,
        ANY
    }

    [Flags, CCode (cname = "xcb_key_but_mask_t", cprefix =  "XCB_KEY_BUT_MASK_", has_type_id = false)]
    public enum KeyButMask {
        SHIFT,
        LOCK,
        CONTROL,
        MOD_1,
        MOD_2,
        MOD_3,
        MOD_4,
        MOD_5,
        BUTTON_1,
        BUTTON_2,
        BUTTON_3,
        BUTTON_4,
        BUTTON_5
    }

    [CCode (cname = "xcb_window_t", cprefix =  "XCB_WINDOW_", has_type_id = false)]
    public enum WindowType {
        NONE
    }

    /**
     * a key was pressed/released
     *
     * @see Window.grab_key
     * @see Window.grab_keyboard
     */
    [Compact, CCode (cname = "xcb_key_press_event_t", has_type_id = false)]
    public class KeyPressEvent : Xcb.GenericEvent {
        public Window child;
        /**
         * The keycode (a number representing a physical key on the keyboard) of the key
         * which was pressed.
         */
        public Keycode detail;
        public Window event;
        /**
         * If `same_screen` is true, this is the X coordinate relative to the `event`
         * window's origin. Otherwise, `event_x` will be set to zero.
         */
        public int16 event_x;
        /**
         * If `same_screen` is true, this is the Y coordinate relative to the `event`
         * window's origin. Otherwise, `event_y` will be set to zero.
         */
        public int16 event_y;
        /**
         * The root window of `child`.
         */
        public Window root;
        /**
         * The X coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_x;
        /**
         * The Y coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_y;
        /**
         * Whether the `event` window is on the same screen as the `root` window.
         */
        public bool same_screen;
        /**
         * The logical state of the pointer buttons and modifier keys just prior to the
         * event.
         */
        public KeyButMask state;
        /**
         * Time when the event was generated (in milliseconds).
         */
        public Timestamp time;
    }

    [CCode (cname = "xcb_button_mask_t", cprefix =  "XCB_BUTTON_MASK_", has_type_id = false)]
    public enum ButtonMask {
        [CCode (cname = "XCB_BUTTON_MASK_1")]
        ONE,
        [CCode (cname = "XCB_BUTTON_MASK_2")]
        TWO,
        [CCode (cname = "XCB_BUTTON_MASK_3")]
        THREE,
        [CCode (cname = "XCB_BUTTON_MASK_4")]
        FOUR,
        [CCode (cname = "XCB_BUTTON_MASK_5")]
        FIVE,
        ANY
    }

    /**
     * a mouse button was pressed/released
     *
     * @see Window.grab_button
     * @see Window.grab_pointer
     */
    [Compact, CCode (cname = "xcb_button_press_event_t", has_type_id = false)]
    public class ButtonPressEvent : Xcb.GenericEvent {
        public Window child;
        /**
         * The keycode (a number representing a physical key on the keyboard) of the key
         * which was pressed.
         */
        public Button detail;
        public Window event;
        /**
         * If `same_screen` is true, this is the X coordinate relative to the `event`
         * window's origin. Otherwise, `event_x` will be set to zero.
         */
        public int16 event_x;
        /**
         * If `same_screen` is true, this is the Y coordinate relative to the `event`
         * window's origin. Otherwise, `event_y` will be set to zero.
         */
        public int16 event_y;
        /**
         * The root window of `child`.
         */
        public Window root;
        /**
         * The X coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_x;
        /**
         * The Y coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_y;
        /**
         * Whether the `event` window is on the same screen as the `root` window.
         */
        public bool same_screen;
        /**
         * The logical state of the pointer buttons and modifier keys just prior to the
         * event.
         */
        public KeyButMask state;
        /**
         * Time when the event was generated (in milliseconds).
         */
        public Timestamp time;
    }

    [CCode (cname = "xcb_motion_t", cprefix =  "XCB_MOTION_", has_type_id = false)]
    public enum Motion {
        NORMAL,
        HINT
    }

    /**
     * a key was pressed
     *
     * @see Window.grab_key
     * @see Window.grab_keyboard
     */
    [Compact, CCode (cname = "xcb_motion_notify_event_t", has_type_id = false)]
    public class MotionNotifyEvent : Xcb.GenericEvent {
        /**
         * The keycode (a number representing a physical key on the keyboard) of the key
         * which was pressed.
         */
        public Motion detail;
        /**
         * Time when the event was generated (in milliseconds).
         */
        public Timestamp time;
        /**
         * The root window of `child`.
         */
        public Window root;
        public Window event;
        public Window child;
        /**
         * The X coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_x;
        /**
         * The Y coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_y;
        /**
         * If `same_screen` is true, this is the X coordinate relative to the `event`
         * window's origin. Otherwise, `event_x` will be set to zero.
         */
        public int16 event_x;
        /**
         * If `same_screen` is true, this is the Y coordinate relative to the `event`
         * window's origin. Otherwise, `event_y` will be set to zero.
         */
        public int16 event_y;
        /**
         * The logical state of the pointer buttons and modifier keys just prior to the
         * event.
         */
        public KeyButMask state;
        /**
         * Whether the `event` window is on the same screen as the `root` window.
         */
        public bool same_screen;
    }

    [CCode (cname = "xcb_notify_detail_t", cprefix =  "XCB_NOTIFY_DETAIL_", has_type_id = false)]
    public enum NotifyDetail {
        ANCESTOR,
        VIRTUAL,
        INFERIOR,
        NONLINEAR,
        NONLINEAR_VIRTUAL,
        POINTER,
        POINTER_ROOT,
        NONE
    }

    [CCode (cname = "xcb_notify_mode_t", cprefix =  "XCB_NOTIFY_MODE_", has_type_id = false)]
    public enum NotifyMode {
        NORMAL,
        GRAB,
        UNGRAB,
        WHILE_GRABBED
    }

    /**
     * the pointer is in a different window
     */
    [Compact, CCode (cname = "xcb_enter_notify_event_t", has_type_id = false)]
    public class EnterNotifyEvent : Xcb.GenericEvent {
        /**
         * If the `event` window has subwindows and the final pointer position is in one
         * of them, then `child` is set to that subwindow, `XCB_WINDOW_NONE` otherwise.
         */
        public Window child;
        public NotifyDetail detail;
        /**
         * The window on which the event was generated.
         */
        public Window event;
        /**
         * If `event` is on the same screen as `root`, this is the pointer X coordinate
         * relative to the event window's origin.
         */
        public int16 event_x;
        /**
         * If `event` is on the same screen as `root`, this is the pointer Y coordinate
         * relative to the event window's origin.
         */
        public int16 event_y;
        public NotifyMode mode;
        /**
         * The root window for the final cursor position.
         */
        public Window root;
        /**
         * The pointer X coordinate relative to `root`'s origin at the time of the event.
         */
        public int16 root_x;
        /**
         * The pointer Y coordinate relative to `root`'s origin at the time of the event.
         */
        public int16 root_y;
        public uint8 same_screen_focus;
        public KeyButMask state;
        public Timestamp time;
    }

    /**
     * NOT YET DOCUMENTED
     */
    [Compact, CCode (cname = "xcb_focus_in_event_t", has_type_id = false)]
    public class FocusInEvent : Xcb.GenericEvent {
        public NotifyDetail detail;
        /**
         * The window on which the focus event was generated. This is the window used by
         * the X server to report the event.
         */
        public Window event;
        public NotifyMode mode;
    }

    [Compact, CCode (cname = "xcb_keymap_notify_event_t", has_type_id = false)]
    public class KeymapNotifyEvent : Xcb.GenericEvent {
        public uint8 keys[31];
    }

    /**
     * NOT YET DOCUMENTED
     */
    [Compact, CCode (cname = "xcb_expose_event_t", has_type_id = false)]
    public class ExposeEvent : Xcb.GenericEvent {
        /**
         * The exposed (damaged) window.
         */
        public Window window;
        /**
         * The X coordinate of the left-upper corner of the exposed rectangle, relative to
         * the `window`'s origin.
         */
        public uint16 x;
        /**
         * The Y coordinate of the left-upper corner of the exposed rectangle, relative to
         * the `window`'s origin.
         */
        public uint16 y;
        /**
         * The width of the exposed rectangle.
         */
        public uint16 width;
        /**
         * The height of the exposed rectangle.
         */
        public uint16 height;
        /**
         * The amount of `Expose` events following this one. Simple applications that do
         * not want to optimize redisplay by distinguishing between subareas of its window
         * can just ignore all Expose events with nonzero counts and perform full
         * redisplays on events with zero counts.
         */
        public uint16 count;
    }

    [Compact, CCode (cname = "xcb_graphics_exposure_event_t", has_type_id = false)]
    public class GraphicsExposureEvent : Xcb.GenericEvent {
        public Drawable drawable;
        public uint16 x;
        public uint16 y;
        public uint16 width;
        public uint16 height;
        public uint16 minor_opcode;
        public uint16 count;
        public uint8 major_opcode;
    }

    [Compact, CCode (cname = "xcb_no_exposure_event_t", has_type_id = false)]
    public class NoExposureEvent : Xcb.GenericEvent {
        public Drawable drawable;
        public uint16 minor_opcode;
        public uint8 major_opcode;
    }

    [CCode (cname = "xcb_visibility_t", cprefix =  "XCB_VISIBILITY_", has_type_id = false)]
    public enum Visibility {
        UNOBSCURED,
        PARTIALLY_OBSCURED,
        FULLY_OBSCURED
    }

    [Compact, CCode (cname = "xcb_visibility_notify_event_t", has_type_id = false)]
    public class VisibilityNotifyEvent : Xcb.GenericEvent {
        public Window window;
        public Visibility state;
    }

    [Compact, CCode (cname = "xcb_create_notify_event_t", has_type_id = false)]
    public class CreateNotifyEvent : Xcb.GenericEvent {
        public Window parent;
        public Window window;
        public int16 x;
        public int16 y;
        public uint16 width;
        public uint16 height;
        public uint16 border_width;
        public bool override_redirect;
    }

    /**
     * a window is destroyed
     *
     * @see Window.destroy
     */
    [Compact, CCode (cname = "xcb_destroy_notify_event_t", has_type_id = false)]
    public class DestroyNotifyEvent : Xcb.GenericEvent {
        /**
         * The reconfigured window or its parent, depending on whether `StructureNotify`
         * or `SubstructureNotify` was selected.
         */
        public Window event;
        /**
         * The window that is destroyed.
         */
        public Window window;
    }

    /**
     * a window is unmapped
     *
     * @see Window.unmap
     */
    [Compact, CCode (cname = "xcb_unmap_notify_event_t", has_type_id = false)]
    public class UnmapNotifyEvent : Xcb.GenericEvent {
        /**
         * The reconfigured window or its parent, depending on whether `StructureNotify`
         * or `SubstructureNotify` was selected.
         */
        public Window event;
        /**
         * The window that was unmapped.
         */
        public Window window;
        /**
         * Set to 1 if the event was generated as a result of a resizing of the window's
         * parent when `window` had a win_gravity of `UnmapGravity`.
         */
        public bool from_configure;
    }

    /**
     * a window was mapped
     *
     * @see Window.map
     */
    [Compact, CCode (cname = "xcb_map_notify_event_t", has_type_id = false)]
    public class MapNotifyEvent : Xcb.GenericEvent {
        /**
         * The window which was mapped or its parent, depending on whether
         * `StructureNotify` or `SubstructureNotify` was selected.
         */
        public Window event;
        /**
         * The window that was mapped.
         */
        public Window window;
        /**
         * Window managers should ignore this window if `override_redirect` is 1.
         */
        public bool override_redirect;
    }

    /**
     * window wants to be mapped
     *
     * @see Window.map
     */
    [Compact, CCode (cname = "xcb_map_request_event_t", has_type_id = false)]
    public class MapRequestEvent : Xcb.GenericEvent {
        /**
         * The parent of `window`.
         */
        public Window parent;
        /**
         * The window to be mapped.
         */
        public Window window;
    }

    [Compact, CCode (cname = "xcb_reparent_notify_event_t", has_type_id = false)]
    public class ReparentNotifyEvent : Xcb.GenericEvent {
        public Window event;
        public Window window;
        public Window parent;
        public int16 x;
        public int16 y;
        public bool override_redirect;
    }

    /**
     * NOT YET DOCUMENTED
     *
     * @see Colormap.free
     */
    [Compact, CCode (cname = "xcb_configure_notify_event_t", has_type_id = false)]
    public class ConfigureNotifyEvent : Xcb.GenericEvent {
        /**
         * The reconfigured window or its parent, depending on whether `StructureNotify`
         * or `SubstructureNotify` was selected.
         */
        public Window event;
        /**
         * The window whose size, position, border, and/or stacking order was changed.
         */
        public Window window;
        /**
         * If `XCB_NONE`, the `window` is on the bottom of the stack with respect to
         * sibling windows. However, if set to a sibling window, the `window` is placed on
         * top of this sibling window.
         */
        public Window above_sibling;
        /**
         * The X coordinate of the upper-left outside corner of `window`, relative to the
         * parent window's origin.
         */
        public int16 x;
        /**
         * The Y coordinate of the upper-left outside corner of `window`, relative to the
         * parent window's origin.
         */
        public int16 y;
        /**
         * The inside width of `window`, not including the border.
         */
        public uint16 width;
        /**
         * The inside height of `window`, not including the border.
         */
        public uint16 height;
        /**
         * The border width of `window`.
         */
        public uint16 border_width;
        /**
         * Window managers should ignore this window if `override_redirect` is 1.
         */
        public bool override_redirect;
    }

    [Compact, CCode (cname = "xcb_configure_request_event_t", has_type_id = false)]
    public class ConfigureRequestEvent : Xcb.GenericEvent {
        public StackMode stack_mode;
        public Window parent;
        public Window window;
        public Window sibling;
        public int16 x;
        public int16 y;
        public uint16 width;
        public uint16 height;
        public uint16 border_width;
        public ConfigWindow value_mask;
    }

    [Compact, CCode (cname = "xcb_gravity_notify_event_t", has_type_id = false)]
    public class GravityNotifyEvent : Xcb.GenericEvent {
        public Window event;
        public Window window;
        public int16 x;
        public int16 y;
    }

    [Compact, CCode (cname = "xcb_resize_request_event_t", has_type_id = false)]
    public class ResizeRequestEvent : Xcb.GenericEvent {
        public Window window;
        public uint16 width;
        public uint16 height;
    }

    [CCode (cname = "xcb_place_t", cprefix =  "XCB_PLACE_", has_type_id = false)]
    public enum Place {
        ON_TOP,
        ON_BOTTOM,

    }

    /**
     * NOT YET DOCUMENTED
     *
     * @see Window.circulate
     */
    [Compact, CCode (cname = "xcb_circulate_notify_event_t", has_type_id = false)]
    public class CirculateNotifyEvent : Xcb.GenericEvent {
        /**
         * Either the restacked window or its parent, depending on whether
         * `StructureNotify` or `SubstructureNotify` was selected.
         */
        public Window event;
        public Place place;
        /**
         * The restacked window.
         */
        public Window window;
    }

    [CCode (cname = "xcb_property_t", cprefix =  "XCB_PROPERTY_", has_type_id = false)]
    public enum Property {
        NEW_VALUE,
        DELETE
    }

    /**
     * a window property changed
     *
     * @see Window.change_property
     */
    [Compact, CCode (cname = "xcb_property_notify_event_t", has_type_id = false)]
    public class PropertyNotifyEvent : Xcb.GenericEvent {
        /**
         * The window whose associated property was changed.
         */
        public Window window;
        /**
         * The property's atom, to indicate which property was changed.
         */
        public Atom atom;
        /**
         * A timestamp of the server time when the property was changed.
         */
        public Timestamp time;
        public Property state;
    }

    [Compact, CCode (cname = "xcb_selection_clear_event_t", has_type_id = false)]
    public class SelectionClearEvent : Xcb.GenericEvent {
        public Timestamp time;
        public Window owner;
        public Atom selection;
    }

    [CCode (cname = "xcb_time_t", cprefix =  "XCB_TIME_", has_type_id = false)]
    public enum Time {
        CURRENT_TIME
    }

    [CCode (cname = "xcb_atom_t", cprefix =  "XCB_ATOM_", has_type_id = false)]
    public enum AtomType {
        NONE,
        ANY,
        PRIMARY,
        SECONDARY,
        ARC,
        ATOM,
        BITMAP,
        CARDINAL,
        COLORMAP,
        CURSOR,
        CUT_BUFFER0,
        CUT_BUFFER1,
        CUT_BUFFER2,
        CUT_BUFFER3,
        CUT_BUFFER4,
        CUT_BUFFER5,
        CUT_BUFFER6,
        CUT_BUFFER7,
        DRAWABLE,
        FONT,
        INTEGER,
        PIXMAP,
        POINT,
        RECTANGLE,
        RESOURCE_MANAGER,
        RGB_COLOR_MAP,
        RGB_BEST_MAP,
        RGB_BLUE_MAP,
        RGB_DEFAULT_MAP,
        RGB_GRAY_MAP,
        RGB_GREEN_MAP,
        RGB_RED_MAP,
        STRING,
        VISUALID,
        WINDOW,
        WM_COMMAND,
        WM_HINTS,
        WM_CLIENT_MACHINE,
        WM_ICON_NAME,
        WM_ICON_SIZE,
        WM_NAME,
        WM_NORMAL_HINTS,
        WM_SIZE_HINTS,
        WM_ZOOM_HINTS,
        MIN_SPACE,
        NORM_SPACE,
        MAX_SPACE,
        END_SPACE,
        SUPERSCRIPT_X,
        SUPERSCRIPT_Y,
        SUBSCRIPT_X,
        SUBSCRIPT_Y,
        UNDERLINE_POSITION,
        UNDERLINE_THICKNESS,
        STRIKEOUT_ASCENT,
        STRIKEOUT_DESCENT,
        ITALIC_ANGLE,
        X_HEIGHT,
        QUAD_WIDTH,
        WEIGHT,
        POINT_SIZE,
        RESOLUTION,
        COPYRIGHT,
        NOTICE,
        FONT_NAME,
        FAMILY_NAME,
        FULL_NAME,
        CAP_HEIGHT,
        WM_CLASS,
        WM_TRANSIENT_FOR
    }

    [Compact, CCode (cname = "xcb_selection_request_event_t", has_type_id = false)]
    public class SelectionRequestEvent : Xcb.GenericEvent {
        public Timestamp time;
        public Window owner;
        public Window requestor;
        public Atom selection;
        public Atom target;
        public Atom property;
    }

    [Compact, CCode (cname = "xcb_selection_notify_event_t", has_type_id = false)]
    public class SelectionNotifyEvent : Xcb.GenericEvent {
        public Timestamp time;
        public Window requestor;
        public Atom selection;
        public Atom target;
        public Atom property;
    }

    [CCode (cname = "xcb_colormap_state_t", cprefix =  "XCB_COLORMAP_STATE_", has_type_id = false)]
    public enum ColormapState {
        UNINSTALLED,
        INSTALLED,

    }

    [CCode (cname = "xcb_colormap_t", cprefix =  "XCB_COLORMAP_", has_type_id = false)]
    public enum ColormapType {
        NONE
    }

    /**
     * the colormap for some window changed
     *
     * @see Colormap.free
     */
    [Compact, CCode (cname = "xcb_colormap_notify_event_t", has_type_id = false)]
    public class ColormapNotifyEvent : Xcb.GenericEvent {
        /**
         * The window whose associated colormap is changed, installed or uninstalled.
         */
        public Window window;
        /**
         * The colormap which is changed, installed or uninstalled. This is `XCB_NONE`
         * when the colormap is changed by a call to `FreeColormap`.
         */
        public Colormap colormap;
        public bool new;
        public ColormapState state;
    }

    [SimpleType, CCode (cname = "xcb_client_message_data_t", has_type_id = false)]
    public struct ClientMessageData {
        public uint8 data8[20];
        public uint16 data16[10];
        public uint32 data32[5];
    }

    /**
     * NOT YET DOCUMENTED
     *
     * This event represents a ClientMessage, sent by another X11 client. An example
     * is a client sending the `_NET_WM_STATE` ClientMessage to the root window
     * to indicate the fullscreen window state, effectively requesting that the window
     * manager puts it into fullscreen mode.
     *
     *
     * @see Window.send_event
     */
    [Compact, CCode (cname = "xcb_client_message_event_t", has_type_id = false)]
    public class ClientMessageEvent : Xcb.GenericEvent {
        /**
         * Specifies how to interpret `data`. Can be either 8, 16 or 32.
         */
        public uint8 format;
        public Window window;
        /**
         * An atom which indicates how the data should be interpreted by the receiving
         * client.
         */
        public Atom type;
        /**
         * The data itself (20 bytes max).
         */
        public ClientMessageData data;
    }

    [CCode (cname = "xcb_mapping_t", cprefix =  "XCB_MAPPING_", has_type_id = false)]
    public enum Mapping {
        MODIFIER,
        KEYBOARD,
        POINTER
    }

    /**
     * keyboard mapping changed
     */
    [Compact, CCode (cname = "xcb_mapping_notify_event_t", has_type_id = false)]
    public class MappingNotifyEvent : Xcb.GenericEvent {
        public Mapping request;
        /**
         * The first number in the range of the altered mapping.
         */
        public Keycode first_keycode;
        /**
         * The number of keycodes altered.
         */
        public uint8 count;
    }

    /**
     * generic event (with length)
     */
    [Compact, CCode (cname = "xcb_ge_generic_event_t", has_type_id = false)]
    public class GeGenericEvent : Xcb.GenericEvent {
    }

    [Compact, CCode (cname = "xcb_request_error_t", has_type_id = false)]
    public class RequestError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_value_error_t", has_type_id = false)]
    public class ValueError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [CCode (cname = "xcb_window_class_t", cprefix =  "XCB_WINDOW_CLASS_", has_type_id = false)]
    public enum WindowClass {
        COPY_FROM_PARENT,
        INPUT_OUTPUT,
        INPUT_ONLY
    }

    [CCode (cname = "xcb_cw_t", cprefix =  "XCB_CW_", has_type_id = false)]
    public enum Cw {
        BACK_PIXMAP,
        BACK_PIXEL,
        BORDER_PIXMAP,
        BORDER_PIXEL,
        BIT_GRAVITY,
        WIN_GRAVITY,
        BACKING_STORE,
        BACKING_PLANES,
        BACKING_PIXEL,
        OVERRIDE_REDIRECT,
        SAVE_UNDER,
        EVENT_MASK,
        DONT_PROPAGATE,
        COLORMAP,
        CURSOR,

    }

    [CCode (cname = "xcb_back_pixmap_t", cprefix =  "XCB_BACK_PIXMAP_", has_type_id = false)]
    public enum BackPixmap {
        NONE,
        PARENT_RELATIVE
    }

    [CCode (cname = "xcb_gravity_t", cprefix =  "XCB_GRAVITY_", has_type_id = false)]
    public enum Gravity {
        BIT_FORGET,
        WIN_UNMAP,
        NORTH_WEST,
        NORTH,
        NORTH_EAST,
        WEST,
        CENTER,
        EAST,
        SOUTH_WEST,
        SOUTH,
        SOUTH_EAST,
        STATIC
    }

    [CCode (cname = "xcb_map_state_t", cprefix =  "XCB_MAP_STATE_", has_type_id = false)]
    public enum MapState {
        UNMAPPED,
        UNVIEWABLE,
        VIEWABLE
    }

    [CCode (cname = "xcb_set_mode_t", cprefix =  "XCB_SET_MODE_", has_type_id = false)]
    public enum SetMode {
        INSERT,
        DELETE
    }

    [Flags, CCode (cname = "xcb_config_window_t", cprefix =  "XCB_CONFIG_WINDOW_", has_type_id = false)]
    public enum ConfigWindow {
        X,
        Y,
        WIDTH,
        HEIGHT,
        BORDER_WIDTH,
        SIBLING,
        STACK_MODE
    }

    [CCode (cname = "xcb_stack_mode_t", cprefix =  "XCB_STACK_MODE_", has_type_id = false)]
    public enum StackMode {
        ABOVE,
        BELOW,
        TOP_IF,
        BOTTOM_IF,
        OPPOSITE
    }

    [CCode (cname = "xcb_circulate_t", cprefix =  "XCB_CIRCULATE_", has_type_id = false)]
    public enum Circulate {
        RAISE_LOWEST,
        LOWER_HIGHEST
    }

    [CCode (cname = "xcb_prop_mode_t", cprefix =  "XCB_PROP_MODE_", has_type_id = false)]
    public enum PropMode {
        REPLACE,
        PREPEND,
        APPEND,

    }

    [CCode (cname = "xcb_get_property_type_t", cprefix =  "XCB_GET_PROPERTY_TYPE_", has_type_id = false)]
    public enum GetPropertyType {
        ANY
    }

    [CCode (cname = "xcb_send_event_dest_t", cprefix =  "XCB_SEND_EVENT_DEST_", has_type_id = false)]
    public enum SendEventDest {
        POINTER_WINDOW,
        ITEM_FOCUS
    }

    [CCode (cname = "xcb_grab_mode_t", cprefix =  "XCB_GRAB_MODE_", has_type_id = false)]
    public enum GrabMode {
        SYNC,
        ASYNC,

    }

    [CCode (cname = "xcb_grab_status_t", cprefix =  "XCB_GRAB_STATUS_", has_type_id = false)]
    public enum GrabStatus {
        SUCCESS,
        ALREADY_GRABBED,
        INVALID_TIME,
        NOT_VIEWABLE,
        FROZEN
    }

    [CCode (cname = "xcb_cursor_t", cprefix =  "XCB_CURSOR_", has_type_id = false)]
    public enum CursorType {
        NONE
    }

    [CCode (cname = "xcb_button_index_t", cprefix =  "XCB_BUTTON_INDEX_", has_type_id = false)]
    public enum ButtonIndex {
        ANY,
        [CCode (cname = "XCB_BUTTON_INDEX_1")]
        ONE,
        [CCode (cname = "XCB_BUTTON_INDEX_2")]
        TWO,
        [CCode (cname = "XCB_BUTTON_INDEX_3")]
        THREE,
        [CCode (cname = "XCB_BUTTON_INDEX_4")]
        FOUR,
        [CCode (cname = "XCB_BUTTON_INDEX_5")]
        FIVE,

    }

    [CCode (cname = "xcb_grab_t", cprefix =  "XCB_GRAB_", has_type_id = false)]
    public enum Grab {
        ANY
    }

    [CCode (cname = "xcb_allow_t", cprefix =  "XCB_ALLOW_", has_type_id = false)]
    public enum Allow {
        ASYNC_POINTER,
        SYNC_POINTER,
        REPLAY_POINTER,
        ASYNC_KEYBOARD,
        SYNC_KEYBOARD,
        REPLAY_KEYBOARD,
        ASYNC_BOTH,
        SYNC_BOTH,

    }

    [SimpleType, CCode (cname = "xcb_timecoord_iterator_t")]
    struct _TimecoordIterator
    {
        int rem;
        int index;
        unowned Timecoord? data;
    }

    [CCode (cname = "xcb_timecoord_iterator_t")]
    public struct TimecoordIterator
    {
        [CCode (cname = "xcb_timecoord_next")]
        void _next ();

        public inline unowned Timecoord?
        next_value ()
        {
            if (((_TimecoordIterator)this).rem > 0)
            {
                unowned Timecoord? d = ((_TimecoordIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_timecoord_t", has_type_id = false)]
    public struct Timecoord {
        public Timestamp time;
        public int16 x;
        public int16 y;
    }

    [CCode (cname = "xcb_input_focus_t", cprefix =  "XCB_INPUT_FOCUS_", has_type_id = false)]
    public enum InputFocus {
        NONE,
        POINTER_ROOT,
        PARENT,
        FOLLOW_KEYBOARD,

    }

    [CCode (cname = "xcb_font_draw_t", cprefix =  "XCB_FONT_DRAW_", has_type_id = false)]
    public enum FontDraw {
        LEFT_TO_RIGHT,
        RIGHT_TO_LEFT
    }

    [SimpleType, CCode (cname = "xcb_fontprop_iterator_t")]
    struct _FontpropIterator
    {
        int rem;
        int index;
        unowned Fontprop? data;
    }

    [CCode (cname = "xcb_fontprop_iterator_t")]
    public struct FontpropIterator
    {
        [CCode (cname = "xcb_fontprop_next")]
        void _next ();

        public inline unowned Fontprop?
        next_value ()
        {
            if (((_FontpropIterator)this).rem > 0)
            {
                unowned Fontprop? d = ((_FontpropIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_fontprop_t", has_type_id = false)]
    public struct Fontprop {
        public Atom name;
        public uint32 value;
    }

    [SimpleType, CCode (cname = "xcb_charinfo_iterator_t")]
    struct _CharinfoIterator
    {
        int rem;
        int index;
        unowned Charinfo? data;
    }

    [CCode (cname = "xcb_charinfo_iterator_t")]
    public struct CharinfoIterator
    {
        [CCode (cname = "xcb_charinfo_next")]
        void _next ();

        public inline unowned Charinfo?
        next_value ()
        {
            if (((_CharinfoIterator)this).rem > 0)
            {
                unowned Charinfo? d = ((_CharinfoIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_charinfo_t", has_type_id = false)]
    public struct Charinfo {
        public int16 left_side_bearing;
        public int16 right_side_bearing;
        public int16 character_width;
        public int16 ascent;
        public int16 descent;
        public uint16 attributes;
    }

    [SimpleType, CCode (cname = "xcb_str_iterator_t")]
    struct _StrIterator
    {
        int rem;
        int index;
        unowned Str? data;
    }

    [CCode (cname = "xcb_str_iterator_t")]
    public struct StrIterator
    {
        [CCode (cname = "xcb_str_next")]
        void _next ();

        public inline unowned Str?
        next_value ()
        {
            if (((_StrIterator)this).rem > 0)
            {
                unowned Str? d = ((_StrIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_str_t", has_type_id = false)]
    public struct Str {
        public uint8 name_len;
    }

    [Flags, CCode (cname = "xcb_gc_t", cprefix =  "XCB_GC_", has_type_id = false)]
    public enum GC {
        FUNCTION,
        PLANE_MASK,
        FOREGROUND,
        BACKGROUND,
        LINE_WIDTH,
        LINE_STYLE,
        CAP_STYLE,
        JOIN_STYLE,
        FILL_STYLE,
        FILL_RULE,
        TILE,
        STIPPLE,
        TILE_STIPPLE_ORIGIN_X,
        TILE_STIPPLE_ORIGIN_Y,
        FONT,
        SUBWINDOW_MODE,
        GRAPHICS_EXPOSURES,
        CLIP_ORIGIN_X,
        CLIP_ORIGIN_Y,
        CLIP_MASK,
        DASH_OFFSET,
        DASH_LIST,
        ARC_MODE,

    }

    [CCode (cname = "xcb_gx_t", cprefix =  "XCB_GX_", has_type_id = false)]
    public enum Gx {
        CLEAR,
        AND,
        AND_REVERSE,
        COPY,
        AND_INVERTED,
        NOOP,
        XOR,
        OR,
        NOR,
        EQUIV,
        INVERT,
        OR_REVERSE,
        COPY_INVERTED,
        OR_INVERTED,
        NAND,
        SET
    }

    [CCode (cname = "xcb_line_style_t", cprefix =  "XCB_LINE_STYLE_", has_type_id = false)]
    public enum LineStyle {
        SOLID,
        ON_OFF_DASH,
        DOUBLE_DASH
    }

    [CCode (cname = "xcb_cap_style_t", cprefix =  "XCB_CAP_STYLE_", has_type_id = false)]
    public enum CapStyle {
        NOT_LAST,
        BUTT,
        ROUND,
        PROJECTING
    }

    [CCode (cname = "xcb_join_style_t", cprefix =  "XCB_JOIN_STYLE_", has_type_id = false)]
    public enum JoinStyle {
        MITER,
        ROUND,
        BEVEL
    }

    [CCode (cname = "xcb_fill_style_t", cprefix =  "XCB_FILL_STYLE_", has_type_id = false)]
    public enum FillStyle {
        SOLID,
        TILED,
        STIPPLED,
        OPAQUE_STIPPLED
    }

    [CCode (cname = "xcb_fill_rule_t", cprefix =  "XCB_FILL_RULE_", has_type_id = false)]
    public enum FillRule {
        EVEN_ODD,
        WINDING
    }

    [CCode (cname = "xcb_subwindow_mode_t", cprefix =  "XCB_SUBWINDOW_MODE_", has_type_id = false)]
    public enum SubwindowMode {
        CLIP_BY_CHILDREN,
        INCLUDE_INFERIORS
    }

    [CCode (cname = "xcb_arc_mode_t", cprefix =  "XCB_ARC_MODE_", has_type_id = false)]
    public enum ArcMode {
        CHORD,
        PIE_SLICE
    }

    [CCode (cname = "xcb_clip_ordering_t", cprefix =  "XCB_CLIP_ORDERING_", has_type_id = false)]
    public enum ClipOrdering {
        UNSORTED,
        Y_SORTED,
        YX_SORTED,
        YX_BANDED
    }

    [CCode (cname = "xcb_coord_mode_t", cprefix =  "XCB_COORD_MODE_", has_type_id = false)]
    public enum CoordMode {
        ORIGIN,
        PREVIOUS,

    }

    [SimpleType, CCode (cname = "xcb_segment_iterator_t")]
    struct _SegmentIterator
    {
        int rem;
        int index;
        unowned Segment? data;
    }

    [CCode (cname = "xcb_segment_iterator_t")]
    public struct SegmentIterator
    {
        [CCode (cname = "xcb_segment_next")]
        void _next ();

        public inline unowned Segment?
        next_value ()
        {
            if (((_SegmentIterator)this).rem > 0)
            {
                unowned Segment? d = ((_SegmentIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_segment_t", has_type_id = false)]
    public struct Segment {
        public int16 x1;
        public int16 y1;
        public int16 x2;
        public int16 y2;
    }

    [CCode (cname = "xcb_poly_shape_t", cprefix =  "XCB_POLY_SHAPE_", has_type_id = false)]
    public enum PolyShape {
        COMPLEX,
        NONCONVEX,
        CONVEX
    }

    [CCode (cname = "xcb_image_format_t", cprefix =  "XCB_IMAGE_FORMAT_", has_type_id = false)]
    public enum ImageFormat {
        XY_BITMAP,
        XY_PIXMAP,
        Z_PIXMAP
    }

    [CCode (cname = "xcb_colormap_alloc_t", cprefix =  "XCB_COLORMAP_ALLOC_", has_type_id = false)]
    public enum ColormapAlloc {
        NONE,
        ALL
    }

    [Flags, CCode (cname = "xcb_color_flag_t", cprefix =  "XCB_COLOR_FLAG_", has_type_id = false)]
    public enum ColorFlag {
        RED,
        GREEN,
        BLUE
    }

    [SimpleType, CCode (cname = "xcb_coloritem_iterator_t")]
    struct _ColoritemIterator
    {
        int rem;
        int index;
        unowned Coloritem? data;
    }

    [CCode (cname = "xcb_coloritem_iterator_t")]
    public struct ColoritemIterator
    {
        [CCode (cname = "xcb_coloritem_next")]
        void _next ();

        public inline unowned Coloritem?
        next_value ()
        {
            if (((_ColoritemIterator)this).rem > 0)
            {
                unowned Coloritem? d = ((_ColoritemIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_coloritem_t", has_type_id = false)]
    public struct Coloritem {
        public uint32 pixel;
        public uint16 red;
        public uint16 green;
        public uint16 blue;
        public ColorFlag flags;
    }

    [SimpleType, CCode (cname = "xcb_rgb_iterator_t")]
    struct _RgbIterator
    {
        int rem;
        int index;
        unowned Rgb? data;
    }

    [CCode (cname = "xcb_rgb_iterator_t")]
    public struct RgbIterator
    {
        [CCode (cname = "xcb_rgb_next")]
        void _next ();

        public inline unowned Rgb?
        next_value ()
        {
            if (((_RgbIterator)this).rem > 0)
            {
                unowned Rgb? d = ((_RgbIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_rgb_t", has_type_id = false)]
    public struct Rgb {
        public uint16 red;
        public uint16 green;
        public uint16 blue;
    }

    [CCode (cname = "xcb_pixmap_t", cprefix =  "XCB_PIXMAP_", has_type_id = false)]
    public enum PixmapType {
        NONE
    }

    [CCode (cname = "xcb_font_t", cprefix =  "XCB_FONT_", has_type_id = false)]
    public enum FontType {
        NONE
    }

    [CCode (cname = "xcb_query_shape_of_t", cprefix =  "XCB_QUERY_SHAPE_OF_", has_type_id = false)]
    public enum QueryShapeOf {
        LARGEST_CURSOR,
        FASTEST_TILE,
        FASTEST_STIPPLE
    }

    [CCode (cname = "xcb_kb_t", cprefix =  "XCB_KB_", has_type_id = false)]
    public enum Kb {
        KEY_CLICK_PERCENT,
        BELL_PERCENT,
        BELL_PITCH,
        BELL_DURATION,
        LED,
        LED_MODE,
        KEY,
        AUTO_REPEAT_MODE
    }

    [CCode (cname = "xcb_led_mode_t", cprefix =  "XCB_LED_MODE_", has_type_id = false)]
    public enum LedMode {
        OFF,
        ON
    }

    [CCode (cname = "xcb_auto_repeat_mode_t", cprefix =  "XCB_AUTO_REPEAT_MODE_", has_type_id = false)]
    public enum AutoRepeatMode {
        OFF,
        ON,
        DEFAULT
    }

    [CCode (cname = "xcb_blanking_t", cprefix =  "XCB_BLANKING_", has_type_id = false)]
    public enum Blanking {
        NOT_PREFERRED,
        PREFERRED,
        DEFAULT
    }

    [CCode (cname = "xcb_exposures_t", cprefix =  "XCB_EXPOSURES_", has_type_id = false)]
    public enum Exposures {
        NOT_ALLOWED,
        ALLOWED,
        DEFAULT
    }

    [CCode (cname = "xcb_host_mode_t", cprefix =  "XCB_HOST_MODE_", has_type_id = false)]
    public enum HostMode {
        INSERT,
        DELETE
    }

    [CCode (cname = "xcb_family_t", cprefix =  "XCB_FAMILY_", has_type_id = false)]
    public enum Family {
        INTERNET,
        DE_CNET,
        CHAOS,
        SERVER_INTERPRETED,
        INTERNET_6
    }

    [SimpleType, CCode (cname = "xcb_host_iterator_t")]
    struct _HostIterator
    {
        int rem;
        int index;
        unowned Host? data;
    }

    [CCode (cname = "xcb_host_iterator_t")]
    public struct HostIterator
    {
        [CCode (cname = "xcb_host_next")]
        void _next ();

        public inline unowned Host?
        next_value ()
        {
            if (((_HostIterator)this).rem > 0)
            {
                unowned Host? d = ((_HostIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_host_t", has_type_id = false)]
    public struct Host {
        public Family family;
        public uint16 address_len;
    }

    [CCode (cname = "xcb_access_control_t", cprefix =  "XCB_ACCESS_CONTROL_", has_type_id = false)]
    public enum AccessControl {
        DISABLE,
        ENABLE
    }

    [CCode (cname = "xcb_close_down_t", cprefix =  "XCB_CLOSE_DOWN_", has_type_id = false)]
    public enum CloseDown {
        DESTROY_ALL,
        RETAIN_PERMANENT,
        RETAIN_TEMPORARY
    }

    [CCode (cname = "xcb_kill_t", cprefix =  "XCB_KILL_", has_type_id = false)]
    public enum Kill {
        ALL_TEMPORARY
    }

    [CCode (cname = "xcb_screen_saver_t", cprefix =  "XCB_SCREEN_SAVER_", has_type_id = false)]
    public enum ScreenSaverType {
        RESET,
        ACTIVE
    }

    [CCode (cname = "xcb_mapping_status_t", cprefix =  "XCB_MAPPING_STATUS_", has_type_id = false)]
    public enum MappingStatus {
        SUCCESS,
        BUSY,
        FAILURE
    }

    [CCode (cname = "xcb_map_index_t", cprefix =  "XCB_MAP_INDEX_", has_type_id = false)]
    public enum MapIndex {
        SHIFT,
        LOCK,
        CONTROL,
        [CCode (cname = "XCB_MAP_INDEX_1")]
        ONE,
        [CCode (cname = "XCB_MAP_INDEX_2")]
        TWO,
        [CCode (cname = "XCB_MAP_INDEX_3")]
        THREE,
        [CCode (cname = "XCB_MAP_INDEX_4")]
        FOUR,
        [CCode (cname = "XCB_MAP_INDEX_5")]
        FIVE
    }

    [Compact, CCode (cname = "xcb_window_error_t", has_type_id = false)]
    public class WindowError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_pixmap_error_t", has_type_id = false)]
    public class PixmapError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_atom_error_t", has_type_id = false)]
    public class AtomError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_cursor_error_t", has_type_id = false)]
    public class CursorError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_font_error_t", has_type_id = false)]
    public class FontError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_match_error_t", has_type_id = false)]
    public class MatchError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_drawable_error_t", has_type_id = false)]
    public class DrawableError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_access_error_t", has_type_id = false)]
    public class AccessError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_alloc_error_t", has_type_id = false)]
    public class AllocError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_colormap_error_t", has_type_id = false)]
    public class ColormapError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_g_context_error_t", has_type_id = false)]
    public class GContextError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_id_choice_error_t", has_type_id = false)]
    public class IdchoiceError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_name_error_t", has_type_id = false)]
    public class NameError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_length_error_t", has_type_id = false)]
    public class LengthError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    [Compact, CCode (cname = "xcb_implementation_error_t", has_type_id = false)]
    public class ImplementationError : Xcb.GenericError {
        public uint32 bad_value;
        public uint8 major_opcode;
        public uint16 minor_opcode;
    }

    /**
     * a key was pressed/released
     *
     * @see Window.grab_key
     * @see Window.grab_keyboard
     */
    [Compact, CCode (cname = "xcb_key_release_event_t", has_type_id = false)]
    public class KeyReleaseEvent : Xcb.GenericEvent {
        public Window child;
        /**
         * The keycode (a number representing a physical key on the keyboard) of the key
         * which was pressed.
         */
        public Keycode detail;
        public Window event;
        /**
         * If `same_screen` is true, this is the X coordinate relative to the `event`
         * window's origin. Otherwise, `event_x` will be set to zero.
         */
        public int16 event_x;
        /**
         * If `same_screen` is true, this is the Y coordinate relative to the `event`
         * window's origin. Otherwise, `event_y` will be set to zero.
         */
        public int16 event_y;
        /**
         * The root window of `child`.
         */
        public Window root;
        /**
         * The X coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_x;
        /**
         * The Y coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_y;
        /**
         * Whether the `event` window is on the same screen as the `root` window.
         */
        public bool same_screen;
        /**
         * The logical state of the pointer buttons and modifier keys just prior to the
         * event.
         */
        public KeyButMask state;
        /**
         * Time when the event was generated (in milliseconds).
         */
        public Timestamp time;
    }

    /**
     * a mouse button was pressed/released
     *
     * @see Window.grab_button
     * @see Window.grab_pointer
     */
    [Compact, CCode (cname = "xcb_button_release_event_t", has_type_id = false)]
    public class ButtonReleaseEvent : Xcb.GenericEvent {
        public Window child;
        /**
         * The keycode (a number representing a physical key on the keyboard) of the key
         * which was pressed.
         */
        public Button detail;
        public Window event;
        /**
         * If `same_screen` is true, this is the X coordinate relative to the `event`
         * window's origin. Otherwise, `event_x` will be set to zero.
         */
        public int16 event_x;
        /**
         * If `same_screen` is true, this is the Y coordinate relative to the `event`
         * window's origin. Otherwise, `event_y` will be set to zero.
         */
        public int16 event_y;
        /**
         * The root window of `child`.
         */
        public Window root;
        /**
         * The X coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_x;
        /**
         * The Y coordinate of the pointer relative to the `root` window at the time of
         * the event.
         */
        public int16 root_y;
        /**
         * Whether the `event` window is on the same screen as the `root` window.
         */
        public bool same_screen;
        /**
         * The logical state of the pointer buttons and modifier keys just prior to the
         * event.
         */
        public KeyButMask state;
        /**
         * Time when the event was generated (in milliseconds).
         */
        public Timestamp time;
    }

    /**
     * the pointer is in a different window
     */
    [Compact, CCode (cname = "xcb_leave_notify_event_t", has_type_id = false)]
    public class LeaveNotifyEvent : Xcb.GenericEvent {
        /**
         * If the `event` window has subwindows and the final pointer position is in one
         * of them, then `child` is set to that subwindow, `XCB_WINDOW_NONE` otherwise.
         */
        public Window child;
        public NotifyDetail detail;
        /**
         * The window on which the event was generated.
         */
        public Window event;
        /**
         * If `event` is on the same screen as `root`, this is the pointer X coordinate
         * relative to the event window's origin.
         */
        public int16 event_x;
        /**
         * If `event` is on the same screen as `root`, this is the pointer Y coordinate
         * relative to the event window's origin.
         */
        public int16 event_y;
        public NotifyMode mode;
        /**
         * The root window for the final cursor position.
         */
        public Window root;
        /**
         * The pointer X coordinate relative to `root`'s origin at the time of the event.
         */
        public int16 root_x;
        /**
         * The pointer Y coordinate relative to `root`'s origin at the time of the event.
         */
        public int16 root_y;
        public uint8 same_screen_focus;
        public KeyButMask state;
        public Timestamp time;
    }

    /**
     * NOT YET DOCUMENTED
     */
    [Compact, CCode (cname = "xcb_focus_out_event_t", has_type_id = false)]
    public class FocusOutEvent : Xcb.GenericEvent {
        public NotifyDetail detail;
        /**
         * The window on which the focus event was generated. This is the window used by
         * the X server to report the event.
         */
        public Window event;
        public NotifyMode mode;
    }

    /**
     * NOT YET DOCUMENTED
     *
     * @see Window.circulate
     */
    [Compact, CCode (cname = "xcb_circulate_request_event_t", has_type_id = false)]
    public class CirculateRequestEvent : Xcb.GenericEvent {
        /**
         * Either the restacked window or its parent, depending on whether
         * `StructureNotify` or `SubstructureNotify` was selected.
         */
        public Window event;
        public Place place;
        /**
         * The restacked window.
         */
        public Window window;
    }

    [CCode (cname = "guint8", cprefix =  "XCB_", has_type_id = false)]
    public enum EventType {
        KEY_PRESS,
        KEY_RELEASE,
        BUTTON_PRESS,
        BUTTON_RELEASE,
        MOTION_NOTIFY,
        ENTER_NOTIFY,
        LEAVE_NOTIFY,
        FOCUS_IN,
        FOCUS_OUT,
        KEYMAP_NOTIFY,
        EXPOSE,
        GRAPHICS_EXPOSURE,
        NO_EXPOSURE,
        VISIBILITY_NOTIFY,
        CREATE_NOTIFY,
        DESTROY_NOTIFY,
        UNMAP_NOTIFY,
        MAP_NOTIFY,
        MAP_REQUEST,
        REPARENT_NOTIFY,
        CONFIGURE_NOTIFY,
        CONFIGURE_REQUEST,
        GRAVITY_NOTIFY,
        RESIZE_REQUEST,
        CIRCULATE_NOTIFY,
        CIRCULATE_REQUEST,
        PROPERTY_NOTIFY,
        SELECTION_CLEAR,
        SELECTION_REQUEST,
        SELECTION_NOTIFY,
        COLORMAP_NOTIFY,
        CLIENT_MESSAGE,
        MAPPING_NOTIFY,
        GE_GENERIC
    }
}
