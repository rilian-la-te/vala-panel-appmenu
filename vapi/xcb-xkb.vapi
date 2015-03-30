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

[CCode (cheader_filename="xcb/xcb.h,xcb/xkb.h")]
namespace Xcb.xkb
{
    [CCode (cname = "xcb_xkb_id")]
    public Xcb.Extension id;

    [Compact, CCode (cname = "xcb_connection_t", free_function = "xcb_disconnect")]
    public class Connection : Xcb.Connection {
        [CCode (cname = "xcb_connect")]
        public Connection (string? displayname = null, out int screen = null);

        [CCode (cname = "xcb_xkb_use_extension")]
        public UseExtensionCookie use_extension (uint16 wantedMajor, uint16 wantedMinor);
        [CCode (cname = "xcb_xkb_select_events", simple_generics = true)]
        public VoidCookie select_events<A> (DeviceSpec deviceSpec, EventType affectWhich, EventType clear, EventType selectAll, MapPart affectMap, MapPart map, void* data);
        [CCode (cname = "xcb_xkb_select_events_checked", simple_generics = true)]
        public VoidCookie select_events_checked<A> (DeviceSpec deviceSpec, EventType affectWhich, EventType clear, EventType selectAll, MapPart affectMap, MapPart map, void* data);
        [CCode (cname = "xcb_xkb_get_state")]
        public GetStateCookie get_state (DeviceSpec deviceSpec);
        [CCode (cname = "xcb_xkb_latch_lock_state")]
        public VoidCookie latch_lock_state (DeviceSpec deviceSpec, uint8 affectModLocks, uint8 modLocks, bool lockGroup, Group groupLock, uint8 affectModLatches, bool latchGroup, uint16 groupLatch);
        [CCode (cname = "xcb_xkb_latch_lock_state_checked")]
        public VoidCookie latch_lock_state_checked (DeviceSpec deviceSpec, uint8 affectModLocks, uint8 modLocks, bool lockGroup, Group groupLock, uint8 affectModLatches, bool latchGroup, uint16 groupLatch);
        [CCode (cname = "xcb_xkb_get_controls")]
        public GetControlsCookie get_controls (DeviceSpec deviceSpec);
        [CCode (cname = "xcb_xkb_set_controls")]
        public VoidCookie set_controls (DeviceSpec deviceSpec, uint8 affectInternalRealMods, uint8 internalRealMods, uint8 affectIgnoreLockRealMods, uint8 ignoreLockRealMods, Vmod affectInternalVirtualMods, Vmod internalVirtualMods, Vmod affectIgnoreLockVirtualMods, Vmod ignoreLockVirtualMods, uint8 mouseKeysDfltBtn, uint8 groupsWrap, Axoption accessXOptions, BoolCtrl affectEnabledControls, BoolCtrl enabledControls, Control changeControls, uint16 repeatDelay, uint16 repeatInterval, uint16 slowKeysDelay, uint16 debounceDelay, uint16 mouseKeysDelay, uint16 mouseKeysInterval, uint16 mouseKeysTimeToMax, uint16 mouseKeysMaxSpeed, int16 mouseKeysCurve, uint16 accessXTimeout, BoolCtrl accessXTimeoutMask, BoolCtrl accessXTimeoutValues, Axoption accessXTimeoutOptionsMask, Axoption accessXTimeoutOptionsValues, [CCode (array_length = false)]uint8 perKeyRepeat[32]);
        [CCode (cname = "xcb_xkb_set_controls_checked")]
        public VoidCookie set_controls_checked (DeviceSpec deviceSpec, uint8 affectInternalRealMods, uint8 internalRealMods, uint8 affectIgnoreLockRealMods, uint8 ignoreLockRealMods, Vmod affectInternalVirtualMods, Vmod internalVirtualMods, Vmod affectIgnoreLockVirtualMods, Vmod ignoreLockVirtualMods, uint8 mouseKeysDfltBtn, uint8 groupsWrap, Axoption accessXOptions, BoolCtrl affectEnabledControls, BoolCtrl enabledControls, Control changeControls, uint16 repeatDelay, uint16 repeatInterval, uint16 slowKeysDelay, uint16 debounceDelay, uint16 mouseKeysDelay, uint16 mouseKeysInterval, uint16 mouseKeysTimeToMax, uint16 mouseKeysMaxSpeed, int16 mouseKeysCurve, uint16 accessXTimeout, BoolCtrl accessXTimeoutMask, BoolCtrl accessXTimeoutValues, Axoption accessXTimeoutOptionsMask, Axoption accessXTimeoutOptionsValues, [CCode (array_length = false)]uint8 perKeyRepeat[32]);
        [CCode (cname = "xcb_xkb_get_map")]
        public GetMapCookie get_map (DeviceSpec deviceSpec, MapPart full, MapPart partial, uint8 firstType, uint8 nTypes, Keycode firstKeySym, uint8 nKeySyms, Keycode firstKeyAction, uint8 nKeyActions, Keycode firstKeyBehavior, uint8 nKeyBehaviors, Vmod virtualMods, Keycode firstKeyExplicit, uint8 nKeyExplicit, Keycode firstModMapKey, uint8 nModMapKeys, Keycode firstVModMapKey, uint8 nVModMapKeys);
        [CCode (cname = "xcb_xkb_set_map", simple_generics = true)]
        public VoidCookie set_map<A> (DeviceSpec deviceSpec, MapPart present, SetMapFlags flags, Keycode minKeyCode, Keycode maxKeyCode, uint8 firstType, Keycode firstKeySym, uint16 totalSyms, Keycode firstKeyAction, Keycode firstKeyBehavior, uint8 nKeyBehaviors, Keycode firstKeyExplicit, uint8 nKeyExplicit, Keycode firstModMapKey, uint8 nModMapKeys, Keycode firstVModMapKey, uint8 nVModMapKeys, Vmod virtualMods, [CCode (array_length_pos = 6.7)]A[]? values);
        [CCode (cname = "xcb_xkb_set_map_checked", simple_generics = true)]
        public VoidCookie set_map_checked<A> (DeviceSpec deviceSpec, MapPart present, SetMapFlags flags, Keycode minKeyCode, Keycode maxKeyCode, uint8 firstType, Keycode firstKeySym, uint16 totalSyms, Keycode firstKeyAction, Keycode firstKeyBehavior, uint8 nKeyBehaviors, Keycode firstKeyExplicit, uint8 nKeyExplicit, Keycode firstModMapKey, uint8 nModMapKeys, Keycode firstVModMapKey, uint8 nVModMapKeys, Vmod virtualMods, [CCode (array_length_pos = 6.7)]A[]? values);
        [CCode (cname = "xcb_xkb_get_compat_map")]
        public GetCompatMapCookie get_compat_map (DeviceSpec deviceSpec, SetOfGroup groups, bool getAllSI, uint16 firstSI, uint16 nSI);
        [CCode (cname = "xcb_xkb_set_compat_map")]
        public VoidCookie set_compat_map (DeviceSpec deviceSpec, bool recomputeActions, bool truncateSI, SetOfGroup groups, uint16 firstSI, [CCode (array_length_pos = 5.6)]SymInterpret[]? si, [CCode (array_length_pos = 7.7)]ModDef[]? groupMaps);
        [CCode (cname = "xcb_xkb_set_compat_map_checked")]
        public VoidCookie set_compat_map_checked (DeviceSpec deviceSpec, bool recomputeActions, bool truncateSI, SetOfGroup groups, uint16 firstSI, [CCode (array_length_pos = 5.6)]SymInterpret[]? si, [CCode (array_length_pos = 7.7)]ModDef[]? groupMaps);
        [CCode (cname = "xcb_xkb_get_indicator_state")]
        public GetIndicatorStateCookie get_indicator_state (DeviceSpec deviceSpec);
        [CCode (cname = "xcb_xkb_get_indicator_map")]
        public GetIndicatorMapCookie get_indicator_map (DeviceSpec deviceSpec, uint32 which);
        [CCode (cname = "xcb_xkb_set_indicator_map")]
        public VoidCookie set_indicator_map (DeviceSpec deviceSpec, uint32 which, [CCode (array_length_pos = 2.2)]IndicatorMap[]? maps);
        [CCode (cname = "xcb_xkb_set_indicator_map_checked")]
        public VoidCookie set_indicator_map_checked (DeviceSpec deviceSpec, uint32 which, [CCode (array_length_pos = 2.2)]IndicatorMap[]? maps);
        [CCode (cname = "xcb_xkb_get_names")]
        public GetNamesCookie get_names (DeviceSpec deviceSpec, NameDetail which);
        [CCode (cname = "xcb_xkb_set_names", simple_generics = true)]
        public VoidCookie set_names<A> (DeviceSpec deviceSpec, Vmod virtualMods, NameDetail which, uint8 firstType, uint8 firstKTLevelt, uint8 nKTLevels, uint32 indicators, SetOfGroup groupNames, Keycode firstKey, uint16 totalKTLevelNames);
        [CCode (cname = "xcb_xkb_set_names_checked", simple_generics = true)]
        public VoidCookie set_names_checked<A> (DeviceSpec deviceSpec, Vmod virtualMods, NameDetail which, uint8 firstType, uint8 firstKTLevelt, uint8 nKTLevels, uint32 indicators, SetOfGroup groupNames, Keycode firstKey, uint16 totalKTLevelNames);
        [CCode (cname = "xcb_xkb_per_client_flags")]
        public PerClientFlagsCookie per_client_flags (DeviceSpec deviceSpec, PerClientFlag change, PerClientFlag value, BoolCtrl ctrlsToChange, BoolCtrl autoCtrls, BoolCtrl autoCtrlsValues);
        [CCode (cname = "xcb_xkb_list_components")]
        public ListComponentsCookie list_components (DeviceSpec deviceSpec, uint16 maxNames);
        [CCode (cname = "xcb_xkb_get_kbd_by_name")]
        public GetKbdByNameCookie get_kbd_by_name (DeviceSpec deviceSpec, Gbndetail need, Gbndetail want, bool load);
        [CCode (cname = "xcb_xkb_get_kbd_by_name_unchecked")]
        public GetKbdByNameCookie get_kbd_by_name_unchecked (DeviceSpec deviceSpec, Gbndetail need, Gbndetail want, bool load);
        [CCode (cname = "xcb_xkb_get_device_info")]
        public GetDeviceInfoCookie get_device_info (DeviceSpec deviceSpec, XIFeature wanted, bool allButtons, uint8 firstButton, uint8 nButtons, LedClass ledClass, Idspec ledID);
        [CCode (cname = "xcb_xkb_set_device_info")]
        public VoidCookie set_device_info (DeviceSpec deviceSpec, uint8 firstBtn, XIFeature change, [CCode (array_length_pos = 2.3)]Action[]? btnActions, [CCode (array_length_pos = 4.5)]DeviceLedInfo[]? leds);
        [CCode (cname = "xcb_xkb_set_device_info_checked")]
        public VoidCookie set_device_info_checked (DeviceSpec deviceSpec, uint8 firstBtn, XIFeature change, [CCode (array_length_pos = 2.3)]Action[]? btnActions, [CCode (array_length_pos = 4.5)]DeviceLedInfo[]? leds);
        [CCode (cname = "xcb_xkb_set_debugging_flags")]
        public SetDebuggingFlagsCookie set_debugging_flags (uint32 affectFlags, uint32 flags, uint32 affectCtrls, uint32 ctrls, [CCode (array_length_pos = 0.1)]String8[]? message);
    }

    [Compact, CCode (cname = "xcb_xkb_use_extension_reply_t", free_function = "free")]
    public class UseExtensionReply {
        public bool supported;
        public uint16 serverMajor;
        public uint16 serverMinor;
    }

    [SimpleType, CCode (cname = "xcb_xkb_use_extension_cookie_t")]
    public struct UseExtensionCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_use_extension_reply", instance_pos = 1.1)]
        public UseExtensionReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_state_reply_t", free_function = "free")]
    public class GetStateReply {
        public uint8 deviceID;
        public uint8 mods;
        public uint8 baseMods;
        public uint8 latchedMods;
        public uint8 lockedMods;
        public Group group;
        public Group lockedGroup;
        public int16 baseGroup;
        public int16 latchedGroup;
        public uint8 compatState;
        public uint8 grabMods;
        public uint8 compatGrabMods;
        public uint8 lookupMods;
        public uint8 compatLookupMods;
        public uint16 ptrBtnState;
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_state_cookie_t")]
    public struct GetStateCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_state_reply", instance_pos = 1.1)]
        public GetStateReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_controls_reply_t", free_function = "free")]
    public class GetControlsReply {
        public uint8 deviceID;
        public uint8 mouseKeysDfltBtn;
        public uint8 numGroups;
        public uint8 groupsWrap;
        public uint8 internalModsMask;
        public uint8 ignoreLockModsMask;
        public uint8 internalModsRealMods;
        public uint8 ignoreLockModsRealMods;
        public Vmod internalModsVmods;
        public Vmod ignoreLockModsVmods;
        public uint16 repeatDelay;
        public uint16 repeatInterval;
        public uint16 slowKeysDelay;
        public uint16 debounceDelay;
        public uint16 mouseKeysDelay;
        public uint16 mouseKeysInterval;
        public uint16 mouseKeysTimeToMax;
        public uint16 mouseKeysMaxSpeed;
        public int16 mouseKeysCurve;
        public Axoption accessXOption;
        public uint16 accessXTimeout;
        public Axoption accessXTimeoutOptionsMask;
        public Axoption accessXTimeoutOptionsValues;
        public BoolCtrl accessXTimeoutMask;
        public BoolCtrl accessXTimeoutValues;
        public BoolCtrl enabledControls;
        public uint8 perKeyRepeat[32];
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_controls_cookie_t")]
    public struct GetControlsCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_controls_reply", instance_pos = 1.1)]
        public GetControlsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_map_reply_t", free_function = "free")]
    public class GetMapReply {
        public uint8 deviceID;
        public Keycode minKeyCode;
        public Keycode maxKeyCode;
        public MapPart present;
        public uint8 firstType;
        public uint8 nTypes;
        public uint8 totalTypes;
        public Keycode firstKeySym;
        public uint16 totalSyms;
        public uint8 nKeySyms;
        public Keycode firstKeyAction;
        public uint16 totalActions;
        public uint8 nKeyActions;
        public Keycode firstKeyBehavior;
        public uint8 nKeyBehaviors;
        public uint8 totalKeyBehaviors;
        public Keycode firstKeyExplicit;
        public uint8 nKeyExplicit;
        public uint8 totalKeyExplicit;
        public Keycode firstModMapKey;
        public uint8 nModMapKeys;
        public uint8 totalModMapKeys;
        public Keycode firstVModMapKey;
        public uint8 nVModMapKeys;
        public uint8 totalVModMapKeys;
        public Vmod virtualMods;
        [CCode (array_length = false)]
        public unowned void[] map {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_types_rtrn_length")]
        public int map_types_rtrn_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned KeyType[] map_types_rtrn {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_syms_rtrn_length")]
        public int map_syms_rtrn_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned KeySymMap[] map_syms_rtrn {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_acts_rtrn_count_length")]
        public int map_acts_rtrn_count_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned uint8[] map_acts_rtrn_count {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_acts_rtrn_acts_length")]
        public int map_acts_rtrn_acts_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned Action[] map_acts_rtrn_acts {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_behaviors_rtrn_length")]
        public int map_behaviors_rtrn_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned SetBehavior[] map_behaviors_rtrn {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_vmods_rtrn_length")]
        public int map_vmods_rtrn_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned uint8[] map_vmods_rtrn {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_explicit_rtrn_length")]
        public int map_explicit_rtrn_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned SetExplicit[] map_explicit_rtrn {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_modmap_rtrn_length")]
        public int map_modmap_rtrn_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned KeyModMap[] map_modmap_rtrn {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_map_map_vmodmap_rtrn_length")]
        public int map_vmodmap_rtrn_length ([CCode (array_length = false)]void[] map);
        [CCode (array_length = false)]
        public unowned KeyVmodMap[] map_vmodmap_rtrn {
            [CCode (cname = "xcb_xkb_get_map_map")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_map_cookie_t")]
    public struct GetMapCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_map_reply", instance_pos = 1.1)]
        public GetMapReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_compat_map_reply_t", free_function = "free")]
    public class GetCompatMapReply {
        public uint8 deviceID;
        public SetOfGroup groupsRtrn;
        public uint16 firstSIRtrn;
        public uint16 nSIRtrn;
        public uint16 nTotalSI;
        public int si_rtrn_length {
            [CCode (cname = "xcb_xkb_get_compat_map_si_rtrn_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned SymInterpret[] si_rtrn {
            [CCode (cname = "xcb_xkb_get_compat_map_si_rtrn")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_compat_map_group_rtrn_iterator")]
        _ModDefIterator _iterator ();
        public ModDefIterator iterator () {
            return (ModDefIterator) _iterator ();
        }
        public int group_rtrn_length {
            [CCode (cname = "xcb_xkb_get_compat_map_group_rtrn_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned ModDef[] group_rtrn {
            [CCode (cname = "xcb_xkb_get_compat_map_group_rtrn")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_compat_map_cookie_t")]
    public struct GetCompatMapCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_compat_map_reply", instance_pos = 1.1)]
        public GetCompatMapReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_indicator_state_reply_t", free_function = "free")]
    public class GetIndicatorStateReply {
        public uint8 deviceID;
        public uint32 state;
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_indicator_state_cookie_t")]
    public struct GetIndicatorStateCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_indicator_state_reply", instance_pos = 1.1)]
        public GetIndicatorStateReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_indicator_map_reply_t", free_function = "free")]
    public class GetIndicatorMapReply {
        public uint8 deviceID;
        public uint32 which;
        public uint32 realIndicators;
        public uint8 nIndicators;
        [CCode (cname = "xcb_xkb_get_indicator_map_maps_iterator")]
        _IndicatorMapIterator _iterator ();
        public IndicatorMapIterator iterator () {
            return (IndicatorMapIterator) _iterator ();
        }
        public int maps_length {
            [CCode (cname = "xcb_xkb_get_indicator_map_maps_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned IndicatorMap[] maps {
            [CCode (cname = "xcb_xkb_get_indicator_map_maps")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_indicator_map_cookie_t")]
    public struct GetIndicatorMapCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_indicator_map_reply", instance_pos = 1.1)]
        public GetIndicatorMapReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_names_reply_t", free_function = "free")]
    public class GetNamesReply {
        public uint8 deviceID;
        public NameDetail which;
        public Keycode minKeyCode;
        public Keycode maxKeyCode;
        public uint8 nTypes;
        public SetOfGroup groupNames;
        public Vmod virtualMods;
        public Keycode firstKey;
        public uint8 nKeys;
        public uint32 indicators;
        public uint8 nRadioGroups;
        public uint8 nKeyAliases;
        public uint16 nKTLevels;
        [CCode (cname = "xcb_xkb_get_names_value_list_type_names_length")]
        public int value_list_type_names_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned Atom[] value_list_type_names {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_n_levels_per_type_length")]
        public int value_list_n_levels_per_type_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned uint8[] value_list_n_levels_per_type {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_alignment_pad_length")]
        public int value_list_alignment_pad_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned uint8[] value_list_alignment_pad {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_kt_level_names_length")]
        public int value_list_kt_level_names_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned Atom[] value_list_kt_level_names {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_indicator_names_length")]
        public int value_list_indicator_names_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned Atom[] value_list_indicator_names {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_virtual_mod_names_length")]
        public int value_list_virtual_mod_names_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned Atom[] value_list_virtual_mod_names {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_groups_length")]
        public int value_list_groups_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned Atom[] value_list_groups {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_key_names_length")]
        public int value_list_key_names_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned KeyName[] value_list_key_names {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_key_aliases_length")]
        public int value_list_key_aliases_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned KeyAlias[] value_list_key_aliases {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_names_value_list_radio_group_names_length")]
        public int value_list_radio_group_names_length ([CCode (array_length = false)]void[] value_list);
        [CCode (array_length = false)]
        public unowned Atom[] value_list_radio_group_names {
            [CCode (cname = "xcb_xkb_get_names_value_list")]
            get;
        }
    }

    [Compact, CCode (cname = "xcb_xkb_get_names_value_list_t", free_function = "free")]
    public struct NamesValueList
    {
        [CCode (cname = "xcb_xkb_get_names_value_list_unpack")]
        private static void _unpack(void* value_list,uint8 n_types, uint32 indicators, Vmod virtual_mods,
                                             SetOfGroup group_names, uint8 nKeys, uint8 nKeyAliases, uint8 radioGroups, NameDetail which, out NamesValueList unpacked);
        public static inline NamesValueList unpack (GetNamesReply reply)
        {
            void* value_list = (void*) reply.value_list_alignment_pad;
            NamesValueList ret;
            _unpack(value_list,reply.nTypes, reply.indicators, reply.virtualMods, reply.groupNames, reply.nKeys, reply.nKeyAliases, reply.nRadioGroups, reply.which, out ret);
            return ret;
        }
        public unowned Atom symbolsName;
        [CCode (array_length = false)]
        public unowned Atom[] group_names;
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_names_cookie_t")]
    public struct GetNamesCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_names_reply", instance_pos = 1.1)]
        public GetNamesReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_per_client_flags_reply_t", free_function = "free")]
    public class PerClientFlagsReply {
        public uint8 deviceID;
        public PerClientFlag supported;
        public PerClientFlag value;
        public BoolCtrl autoCtrls;
        public BoolCtrl autoCtrlsValues;
    }

    [SimpleType, CCode (cname = "xcb_xkb_per_client_flags_cookie_t")]
    public struct PerClientFlagsCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_per_client_flags_reply", instance_pos = 1.1)]
        public PerClientFlagsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_list_components_reply_t", free_function = "free")]
    public class ListComponentsReply {
        public uint8 deviceID;
        public uint16 nKeymaps;
        public uint16 nKeycodes;
        public uint16 nTypes;
        public uint16 nCompatMaps;
        public uint16 nSymbols;
        public uint16 nGeometries;
        public uint16 extra;
        public int keymaps_length {
            [CCode (cname = "xcb_xkb_list_components_keymaps_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Listing[] keymaps {
            [CCode (cname = "xcb_xkb_list_components_keymaps")]
            get;
        }
        public int keycodes_length {
            [CCode (cname = "xcb_xkb_list_components_keycodes_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Listing[] keycodes {
            [CCode (cname = "xcb_xkb_list_components_keycodes")]
            get;
        }
        public int types_length {
            [CCode (cname = "xcb_xkb_list_components_types_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Listing[] types {
            [CCode (cname = "xcb_xkb_list_components_types")]
            get;
        }
        public int compat_maps_length {
            [CCode (cname = "xcb_xkb_list_components_compat_maps_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Listing[] compat_maps {
            [CCode (cname = "xcb_xkb_list_components_compat_maps")]
            get;
        }
        public int symbols_length {
            [CCode (cname = "xcb_xkb_list_components_symbols_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Listing[] symbols {
            [CCode (cname = "xcb_xkb_list_components_symbols")]
            get;
        }
        [CCode (cname = "xcb_xkb_list_components_geometries_iterator")]
        _ListingIterator _iterator ();
        public ListingIterator iterator () {
            return (ListingIterator) _iterator ();
        }
        public int geometries_length {
            [CCode (cname = "xcb_xkb_list_components_geometries_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Listing[] geometries {
            [CCode (cname = "xcb_xkb_list_components_geometries")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_list_components_cookie_t")]
    public struct ListComponentsCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_list_components_reply", instance_pos = 1.1)]
        public ListComponentsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_kbd_by_name_reply_t", free_function = "free")]
    public class GetKbdByNameReply {
        public uint8 deviceID;
        public Keycode minKeyCode;
        public Keycode maxKeyCode;
        public bool loaded;
        public bool newKeyboard;
        public Gbndetail found;
        public Gbndetail reported;
        [CCode (cname = "xcb_xkb_get_kbd_by_name_replies_si_rtrn_length")]
        public int replies_si_rtrn_length ([CCode (array_length = false)]void[] replies);
        [CCode (array_length = false)]
        public unowned SymInterpret[] replies_si_rtrn {
            [CCode (cname = "xcb_xkb_get_kbd_by_name_replies")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_kbd_by_name_replies_group_rtrn_length")]
        public int replies_group_rtrn_length ([CCode (array_length = false)]void[] replies);
        [CCode (array_length = false)]
        public unowned ModDef[] replies_group_rtrn {
            [CCode (cname = "xcb_xkb_get_kbd_by_name_replies")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_kbd_by_name_replies_maps_length")]
        public int replies_maps_length ([CCode (array_length = false)]void[] replies);
        [CCode (array_length = false)]
        public unowned IndicatorMap[] replies_maps {
            [CCode (cname = "xcb_xkb_get_kbd_by_name_replies")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_kbd_by_name_cookie_t", has_construct_function = false)]
    public struct GetKbdByNameCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_kbd_by_name_reply", instance_pos = 1.1)]
        public GetKbdByNameReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_get_device_info_reply_t", free_function = "free")]
    public class GetDeviceInfoReply {
        public uint8 deviceID;
        public XIFeature present;
        public XIFeature supported;
        public XIFeature unsupported;
        public uint16 nDeviceLedFBs;
        public uint8 firstBtnWanted;
        public uint8 nBtnsWanted;
        public uint8 firstBtnRtrn;
        public uint8 nBtnsRtrn;
        public uint8 totalBtns;
        public bool hasOwnState;
        public uint16 dfltKbdFB;
        public uint16 dfltLedFB;
        public Atom devType;
        public uint16 nameLen;
        public int name_length {
            [CCode (cname = "xcb_xkb_get_device_info_name_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned String8[] name {
            [CCode (cname = "xcb_xkb_get_device_info_name")]
            get;
        }
        public int btn_actions_length {
            [CCode (cname = "xcb_xkb_get_device_info_btn_actions_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned Action[] btn_actions {
            [CCode (cname = "xcb_xkb_get_device_info_btn_actions")]
            get;
        }
        [CCode (cname = "xcb_xkb_get_device_info_leds_iterator")]
        _DeviceLedInfoIterator _iterator ();
        public DeviceLedInfoIterator iterator () {
            return (DeviceLedInfoIterator) _iterator ();
        }
        public int leds_length {
            [CCode (cname = "xcb_xkb_get_device_info_leds_length")]
            get;
        }
        [CCode (array_length = false)]
        public unowned DeviceLedInfo[] leds {
            [CCode (cname = "xcb_xkb_get_device_info_leds")]
            get;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_device_info_cookie_t")]
    public struct GetDeviceInfoCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_device_info_reply", instance_pos = 1.1)]
        public GetDeviceInfoReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [Compact, CCode (cname = "xcb_xkb_set_debugging_flags_reply_t", free_function = "free")]
    public class SetDebuggingFlagsReply {
        public uint32 currentFlags;
        public uint32 currentCtrls;
        public uint32 supportedFlags;
        public uint32 supportedCtrls;
    }

    [SimpleType, CCode (cname = "xcb_xkb_set_debugging_flags_cookie_t")]
    public struct SetDebuggingFlagsCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_set_debugging_flags_reply", instance_pos = 1.1)]
        public SetDebuggingFlagsReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [CCode (cname = "xcb_atom_t", has_type_id = false)]
    public struct Atom : Xcb.Atom {
        [CCode (cname = "xcb_xkb_get_named_indicator", instance_pos = 4.4)]
        public GetNamedIndicatorCookie get_named_indicator (Xcb.Connection connection, DeviceSpec deviceSpec, LedClass ledClass, Idspec ledID);
        [CCode (cname = "xcb_xkb_set_named_indicator", instance_pos = 4.4)]
        public VoidCookie set_named_indicator (Xcb.Connection connection, DeviceSpec deviceSpec, LedClass ledClass, Idspec ledID, bool setState, bool on, bool setMap, bool createMap, Imflag map_flags, ImgroupsWhich map_whichGroups, SetOfGroups map_groups, ImmodsWhich map_whichMods, uint8 map_realMods, Vmod map_vmods, BoolCtrl map_ctrls);
        [CCode (cname = "xcb_xkb_set_named_indicator_checked", instance_pos = 4.4)]
        public VoidCookie set_named_indicator_checked (Xcb.Connection connection, DeviceSpec deviceSpec, LedClass ledClass, Idspec ledID, bool setState, bool on, bool setMap, bool createMap, Imflag map_flags, ImgroupsWhich map_whichGroups, SetOfGroups map_groups, ImmodsWhich map_whichMods, uint8 map_realMods, Vmod map_vmods, BoolCtrl map_ctrls);
    }

    [Compact, CCode (cname = "xcb_xkb_get_named_indicator_reply_t", free_function = "free")]
    public class GetNamedIndicatorReply {
        public uint8 deviceID;
        public Atom indicator;
        public bool found;
        public bool on;
        public bool realIndicator;
        public uint8 ndx;
        public Imflag map_flags;
        public ImgroupsWhich map_whichGroups;
        public SetOfGroups map_groups;
        public ImmodsWhich map_whichMods;
        public uint8 map_mods;
        public uint8 map_realMods;
        public Vmod map_vmod;
        public BoolCtrl map_ctrls;
        public bool supported;
    }

    [SimpleType, CCode (cname = "xcb_xkb_get_named_indicator_cookie_t")]
    public struct GetNamedIndicatorCookie : VoidCookie {
        [CCode (cname = "xcb_xkb_get_named_indicator_reply", instance_pos = 1.1)]
        public GetNamedIndicatorReply reply (Xcb.Connection connection, out Xcb.GenericError? error = null);
    }

    [CCode (cname = "xcb_xkb_const_t", cprefix =  "XCB_XKB_CONST_", has_type_id = false)]
    public enum Const {
        MAX_LEGAL_KEY_CODE,
        PER_KEY_BIT_ARRAY_SIZE,
        KEY_NAME_LENGTH
    }

    [CCode (cname = "guint8", cprefix =  "XCB_XKB_", has_type_id = false)]
    public enum EventType {
        NEW_KEYBOARD_NOTIFY,
        MAP_NOTIFY,
        STATE_NOTIFY,
        CONTROLS_NOTIFY,
        INDICATOR_STATE_NOTIFY,
        INDICATOR_MAP_NOTIFY,
        NAMES_NOTIFY,
        COMPAT_MAP_NOTIFY,
        BELL_NOTIFY,
        ACTION_MESSAGE,
        ACCESS_X_NOTIFY,
        EXTENSION_DEVICE_NOTIFY
    }

    [Flags, CCode (cname = "xcb_xkb_nkn_detail_t", cprefix =  "XCB_XKB_NKN_DETAIL_", has_type_id = false)]
    public enum Nkndetail {
        KEYCODES,
        GEOMETRY,
        DEVICE_ID
    }

    [Flags, CCode (cname = "xcb_xkb_axn_detail_t", cprefix =  "XCB_XKB_AXNDETAIL_", has_type_id = false)]
    public enum Axndetail {
        SK_PRESS,
        SK_ACCEPT,
        SK_REJECT,
        SK_RELEASE,
        BK_ACCEPT,
        BK_REJECT,
        AXK_WARNING
    }

    [Flags, CCode (cname = "xcb_xkb_map_part_t", cprefix =  "XCB_XKB_MAP_PART_", has_type_id = false)]
    public enum MapPart {
        KEY_TYPES,
        KEY_SYMS,
        MODIFIER_MAP,
        EXPLICIT_COMPONENTS,
        KEY_ACTIONS,
        KEY_BEHAVIORS,
        VIRTUAL_MODS,
        VIRTUAL_MOD_MAP
    }

    [Flags, CCode (cname = "xcb_xkb_set_map_flags_t", cprefix =  "XCB_XKB_SET_MAP_FLAGS_", has_type_id = false)]
    public enum SetMapFlags {
        RESIZE_TYPES,
        RECOMPUTE_ACTIONS
    }

    [Flags, CCode (cname = "xcb_xkb_state_part_t", cprefix =  "XCB_XKB_STATE_PART_", has_type_id = false)]
    public enum StatePart {
        MODIFIER_STATE,
        MODIFIER_BASE,
        MODIFIER_LATCH,
        MODIFIER_LOCK,
        GROUP_STATE,
        GROUP_BASE,
        GROUP_LATCH,
        GROUP_LOCK,
        COMPAT_STATE,
        GRAB_MODS,
        COMPAT_GRAB_MODS,
        LOOKUP_MODS,
        COMPAT_LOOKUP_MODS,
        POINTER_BUTTONS
    }

    [Flags, CCode (cname = "xcb_xkb_bool_ctrl_t", cprefix =  "XCB_XKB_BOOL_CTRL_", has_type_id = false)]
    public enum BoolCtrl {
        REPEAT_KEYS,
        SLOW_KEYS,
        BOUNCE_KEYS,
        STICKY_KEYS,
        MOUSE_KEYS,
        MOUSE_KEYS_ACCEL,
        ACCESS_X_KEYS,
        ACCESS_X_TIMEOUT_MASK,
        ACCESS_X_FEEDBACK_MASK,
        AUDIBLE_BELL_MASK,
        OVERLAY_1_MASK,
        OVERLAY_2_MASK,
        IGNORE_GROUP_LOCK_MASK
    }

    [Flags, CCode (cname = "xcb_xkb_control_t", cprefix =  "XCB_XKB_CONTROL_", has_type_id = false)]
    public enum Control {
        GROUPS_WRAP,
        INTERNAL_MODS,
        IGNORE_LOCK_MODS,
        PER_KEY_REPEAT,
        CONTROLS_ENABLED
    }

    [Flags, CCode (cname = "xcb_xkb_ax_option_t", cprefix =  "XCB_XKB_AXOPTION_", has_type_id = false)]
    public enum Axoption {
        SK_PRESS_FB,
        SK_ACCEPT_FB,
        FEATURE_FB,
        SLOW_WARN_FB,
        INDICATOR_FB,
        STICKY_KEYS_FB,
        TWO_KEYS,
        LATCH_TO_LOCK,
        SK_RELEASE_FB,
        SK_REJECT_FB,
        BK_REJECT_FB,
        DUMB_BELL
    }

    [SimpleType, CCode (cname = "xcb_xkb_device_spec_t", has_type_id = false)]
    public struct DeviceSpec : uint16 {
    }

    [CCode (cname = "xcb_xkb_led_class_result_t", cprefix =  "XCB_XKB_LED_CLASS_RESULT_", has_type_id = false)]
    public enum LedClassResult {
        KBD_FEEDBACK_CLASS,
        LED_FEEDBACK_CLASS
    }

    [CCode (cname = "xcb_xkb_led_class_t", cprefix =  "XCB_XKB_LED_CLASS_", has_type_id = false)]
    public enum LedClass {
        KBD_FEEDBACK_CLASS,
        LED_FEEDBACK_CLASS,
        DFLT_XI_CLASS,
        ALL_XI_CLASSES
    }

    [SimpleType, CCode (cname = "xcb_xkb_led_class_spec_t", has_type_id = false)]
    public struct LedClassSpec : uint16 {
    }

    [CCode (cname = "xcb_xkb_bell_class_result_t", cprefix =  "XCB_XKB_BELL_CLASS_RESULT_", has_type_id = false)]
    public enum BellClassResult {
        KBD_FEEDBACK_CLASS,
        BELL_FEEDBACK_CLASS
    }

    [CCode (cname = "xcb_xkb_bell_class_t", cprefix =  "XCB_XKB_BELL_CLASS_", has_type_id = false)]
    public enum BellClass {
        KBD_FEEDBACK_CLASS,
        BELL_FEEDBACK_CLASS,
        DFLT_XI_CLASS
    }

    [SimpleType, CCode (cname = "xcb_xkb_bell_class_spec_t", has_type_id = false)]
    public struct BellClassSpec : uint16 {
    }

    [CCode (cname = "xcb_xkb_id_t", cprefix =  "XCB_XKB_ID_", has_type_id = false)]
    public enum Id {
        USE_CORE_KBD,
        USE_CORE_PTR,
        DFLT_XI_CLASS,
        DFLT_XI_ID,
        ALL_XI_CLASS,
        ALL_XI_ID,
        XI_NONE
    }

    [SimpleType, CCode (cname = "xcb_xkb_id_spec_t", has_type_id = false)]
    public struct Idspec : uint16 {
    }

    [CCode (cname = "xcb_xkb_group_t", cprefix =  "XCB_XKB_GROUP_", has_type_id = false)]
    public enum Group {
        [CCode (cname = "XCB_XKEYBOARD_GROUP_XKB_1")]
        ONE,
        [CCode (cname = "XCB_XKEYBOARD_GROUP_XKB_2")]
        TWO,
        [CCode (cname = "XCB_XKEYBOARD_GROUP_XKB_3")]
        THREE,
        [CCode (cname = "XCB_XKEYBOARD_GROUP_XKB_4")]
        FOUR
    }

    [CCode (cname = "xcb_xkb_groups_t", cprefix =  "XCB_XKB_GROUPS_", has_type_id = false)]
    public enum Groups {
        ANY,
        ALL
    }

    [Flags, CCode (cname = "xcb_xkb_set_of_group_t", cprefix =  "XCB_XKB_SET_OF_GROUP_", has_type_id = false)]
    public enum SetOfGroup {
        GROUP_1,
        GROUP_2,
        GROUP_3,
        GROUP_4
    }

    [Flags, CCode (cname = "xcb_xkb_set_of_groups_t", cprefix =  "XCB_XKB_SET_OF_GROUPS_", has_type_id = false)]
    public enum SetOfGroups {
        ANY
    }

    [CCode (cname = "xcb_xkb_groups_wrap_t", cprefix =  "XCB_XKB_GROUPS_WRAP_", has_type_id = false)]
    public enum GroupsWrap {
        WRAP_INTO_RANGE,
        CLAMP_INTO_RANGE,
        REDIRECT_INTO_RANGE
    }

    [Flags, CCode (cname = "xcb_xkb_v_mods_high_t", cprefix =  "XCB_XKB_VMODS_HIGH_", has_type_id = false)]
    public enum VmodsHigh {
        [CCode (cname = "XCB_XKEYBOARD_VMODS_HIGH_XKB_15")]
        E15,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_HIGH_XKB_14")]
        E14,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_HIGH_XKB_13")]
        E13,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_HIGH_XKB_12")]
        E12,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_HIGH_XKB_11")]
        E11,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_HIGH_XKB_10")]
        E10,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_HIGH_XKB_9")]
        NINE,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_HIGH_XKB_8")]
        HEIGHT
    }

    [Flags, CCode (cname = "xcb_xkb_v_mods_low_t", cprefix =  "XCB_XKB_VMODS_LOW_", has_type_id = false)]
    public enum VmodsLow {
        [CCode (cname = "XCB_XKEYBOARD_VMODS_LOW_XKB_7")]
        SEVEN,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_LOW_XKB_6")]
        SIX,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_LOW_XKB_5")]
        FIVE,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_LOW_XKB_4")]
        FOUR,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_LOW_XKB_3")]
        THREE,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_LOW_XKB_2")]
        TWO,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_LOW_XKB_1")]
        ONE,
        [CCode (cname = "XCB_XKEYBOARD_VMODS_LOW_XKB_0")]
        E0
    }

    [Flags, CCode (cname = "xcb_xkb_v_mod_t", cprefix =  "XCB_XKB_VMOD_", has_type_id = false)]
    public enum Vmod {
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_15")]
        E15,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_14")]
        E14,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_13")]
        E13,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_12")]
        E12,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_11")]
        E11,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_10")]
        E10,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_9")]
        NINE,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_8")]
        HEIGHT,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_7")]
        SEVEN,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_6")]
        SIX,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_5")]
        FIVE,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_4")]
        FOUR,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_3")]
        THREE,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_2")]
        TWO,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_1")]
        ONE,
        [CCode (cname = "XCB_XKEYBOARD_VMOD_XKB_0")]
        E0
    }

    [Flags, CCode (cname = "xcb_xkb_explicit_t", cprefix =  "XCB_XKB_EXPLICIT_", has_type_id = false)]
    public enum Explicit {
        V_MOD_MAP,
        BEHAVIOR,
        AUTO_REPEAT,
        INTERPRET,
        KEY_TYPE_4,
        KEY_TYPE_3,
        KEY_TYPE_2,
        KEY_TYPE_1
    }

    [CCode (cname = "xcb_xkb_sym_interpret_match_t", cprefix =  "XCB_XKB_SYM_INTERPRET_MATCH_", has_type_id = false)]
    public enum SymInterpretMatch {
        NONE_OF,
        ANY_OF_OR_NONE,
        ANY_OF,
        ALL_OF,
        EXACTLY
    }

    [CCode (cname = "xcb_xkb_sym_interp_match_t", cprefix =  "XCB_XKB_SYM_INTERP_MATCH_", has_type_id = false)]
    public enum SymInterpMatch {
        LEVEL_ONE_ONLY,
        OP_MASK
    }

    [Flags, CCode (cname = "xcb_xkb_im_flag_t", cprefix =  "XCB_XKB_IMFLAG_", has_type_id = false)]
    public enum Imflag {
        NO_EXPLICIT,
        NO_AUTOMATIC,
        LED_DRIVES_KB
    }

    [Flags, CCode (cname = "xcb_xkb_im_mods_which_t", cprefix =  "XCB_XKB_IMMODS_WHICH_", has_type_id = false)]
    public enum ImmodsWhich {
        USE_COMPAT,
        USE_EFFECTIVE,
        USE_LOCKED,
        USE_LATCHED,
        USE_BASE
    }

    [Flags, CCode (cname = "xcb_xkb_im_groups_which_t", cprefix =  "XCB_XKB_IMGROUPS_WHICH_", has_type_id = false)]
    public enum ImgroupsWhich {
        USE_COMPAT,
        USE_EFFECTIVE,
        USE_LOCKED,
        USE_LATCHED,
        USE_BASE
    }

    [SimpleType, CCode (cname = "xcb_xkb_indicator_map_iterator_t")]
    struct _IndicatorMapIterator
    {
        int rem;
        int index;
        unowned IndicatorMap? data;
    }

    [CCode (cname = "xcb_xkb_indicator_map_iterator_t")]
    public struct IndicatorMapIterator
    {
        [CCode (cname = "xcb_xkb_indicator_map_next")]
        void _next ();

        public inline unowned IndicatorMap?
        next_value ()
        {
            if (((_IndicatorMapIterator)this).rem > 0)
            {
                unowned IndicatorMap? d = ((_IndicatorMapIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_indicator_map_t", has_type_id = false)]
    public struct IndicatorMap {
        public Imflag flags;
        public ImgroupsWhich whichGroups;
        public SetOfGroup groups;
        public ImmodsWhich whichMods;
        public uint8 mods;
        public uint8 realMods;
        public Vmod vmods;
        public BoolCtrl ctrls;
    }

    [Flags, CCode (cname = "xcb_xkb_cm_detail_t", cprefix =  "XCB_XKB_CMDETAIL_", has_type_id = false)]
    public enum Cmdetail {
        SYM_INTERP,
        GROUP_COMPAT
    }

    [Flags, CCode (cname = "xcb_xkb_name_detail_t", cprefix =  "XCB_XKB_NAME_DETAIL_", has_type_id = false)]
    public enum NameDetail {
        KEYCODES,
        GEOMETRY,
        SYMBOLS,
        PHYS_SYMBOLS,
        TYPES,
        COMPAT,
        KEY_TYPE_NAMES,
        KT_LEVEL_NAMES,
        INDICATOR_NAMES,
        KEY_NAMES,
        KEY_ALIASES,
        VIRTUAL_MOD_NAMES,
        GROUP_NAMES,
        RG_NAMES
    }

    [Flags, CCode (cname = "xcb_xkb_gbn_detail_t", cprefix =  "XCB_XKB_GBN_DETAIL_", has_type_id = false)]
    public enum Gbndetail {
        TYPES,
        COMPAT_MAP,
        CLIENT_SYMBOLS,
        SERVER_SYMBOLS,
        INDICATOR_MAPS,
        KEY_NAMES,
        GEOMETRY,
        OTHER_NAMES,
    }
    public inline Gbndetail gbn_detail_all()
    {
        return (Gbndetail.TYPES | Gbndetail.COMPAT_MAP | Gbndetail.CLIENT_SYMBOLS | Gbndetail.SERVER_SYMBOLS | Gbndetail.INDICATOR_MAPS | Gbndetail.KEY_NAMES | Gbndetail.GEOMETRY | Gbndetail.OTHER_NAMES);
    }
    [Flags, CCode (cname = "xcb_xkb_xi_feature_t", cprefix =  "XCB_XKB_XIFEATURE_", has_type_id = false)]
    public enum XIFeature {
        KEYBOARDS,
        BUTTON_ACTIONS,
        INDICATOR_NAMES,
        INDICATOR_MAPS,
        INDICATOR_STATE
    }

    [Flags, CCode (cname = "xcb_xkb_per_client_flag_t", cprefix =  "XCB_XKB_PER_CLIENT_FLAG_", has_type_id = false)]
    public enum PerClientFlag {
        DETECTABLE_AUTO_REPEAT,
        GRABS_USE_XKB_STATE,
        AUTO_RESET_CONTROLS,
        LOOKUP_STATE_WHEN_GRABBED,
        SEND_EVENT_USES_XKB_STATE
    }

    [SimpleType, CCode (cname = "xcb_xkb_mod_def_iterator_t")]
    struct _ModDefIterator
    {
        int rem;
        int index;
        unowned ModDef? data;
    }

    [CCode (cname = "xcb_xkb_mod_def_iterator_t")]
    public struct ModDefIterator
    {
        [CCode (cname = "xcb_xkb_mod_def_next")]
        void _next ();

        public inline unowned ModDef?
        next_value ()
        {
            if (((_ModDefIterator)this).rem > 0)
            {
                unowned ModDef? d = ((_ModDefIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_mod_def_t", has_type_id = false)]
    public struct ModDef {
        public uint8 mask;
        public uint8 realMods;
        public Vmod vmods;
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_name_iterator_t")]
    struct _KeyNameIterator
    {
        int rem;
        int index;
        unowned KeyName? data;
    }

    [CCode (cname = "xcb_xkb_key_name_iterator_t")]
    public struct KeyNameIterator
    {
        [CCode (cname = "xcb_xkb_key_name_next")]
        void _next ();

        public inline unowned KeyName?
        next_value ()
        {
            if (((_KeyNameIterator)this).rem > 0)
            {
                unowned KeyName? d = ((_KeyNameIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_name_t", has_type_id = false)]
    public struct KeyName {
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_alias_iterator_t")]
    struct _KeyAliasIterator
    {
        int rem;
        int index;
        unowned KeyAlias? data;
    }

    [CCode (cname = "xcb_xkb_key_alias_iterator_t")]
    public struct KeyAliasIterator
    {
        [CCode (cname = "xcb_xkb_key_alias_next")]
        void _next ();

        public inline unowned KeyAlias?
        next_value ()
        {
            if (((_KeyAliasIterator)this).rem > 0)
            {
                unowned KeyAlias? d = ((_KeyAliasIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_alias_t", has_type_id = false)]
    public struct KeyAlias {
    }

    [SimpleType, CCode (cname = "xcb_xkb_counted_string_16_iterator_t")]
    struct _CountedString16Iterator
    {
        int rem;
        int index;
        unowned CountedString16? data;
    }

    [CCode (cname = "xcb_xkb_counted_string_16_iterator_t")]
    public struct CountedString16Iterator
    {
        [CCode (cname = "xcb_xkb_counted_string_16_next")]
        void _next ();

        public inline unowned CountedString16?
        next_value ()
        {
            if (((_CountedString16Iterator)this).rem > 0)
            {
                unowned CountedString16? d = ((_CountedString16Iterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_counted_string_16_t", has_type_id = false)]
    public struct CountedString16 {
        public uint16 length;
    }

    [SimpleType, CCode (cname = "xcb_xkb_kt_map_entry_iterator_t")]
    struct _KtmapEntryIterator
    {
        int rem;
        int index;
        unowned KtmapEntry? data;
    }

    [CCode (cname = "xcb_xkb_kt_map_entry_iterator_t")]
    public struct KtmapEntryIterator
    {
        [CCode (cname = "xcb_xkb_kt_map_entry_next")]
        void _next ();

        public inline unowned KtmapEntry?
        next_value ()
        {
            if (((_KtmapEntryIterator)this).rem > 0)
            {
                unowned KtmapEntry? d = ((_KtmapEntryIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_kt_map_entry_t", has_type_id = false)]
    public struct KtmapEntry {
        public bool active;
        public uint8 mods_mask;
        public uint8 level;
        public uint8 mods_mods;
        public Vmod mods_vmods;
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_type_iterator_t")]
    struct _KeyTypeIterator
    {
        int rem;
        int index;
        unowned KeyType? data;
    }

    [CCode (cname = "xcb_xkb_key_type_iterator_t")]
    public struct KeyTypeIterator
    {
        [CCode (cname = "xcb_xkb_key_type_next")]
        void _next ();

        public inline unowned KeyType?
        next_value ()
        {
            if (((_KeyTypeIterator)this).rem > 0)
            {
                unowned KeyType? d = ((_KeyTypeIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_type_t", has_type_id = false)]
    public struct KeyType {
        public uint8 mods_mask;
        public uint8 mods_mods;
        public Vmod mods_vmods;
        public uint8 numLevels;
        public uint8 nMapEntries;
        public bool hasPreserve;
        [CCode (cname = "xcb_xkb_key_type_preserve_iterator")]
        _ModDefIterator _iterator ();
        public ModDefIterator iterator () {
            return (ModDefIterator) _iterator ();
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_sym_map_iterator_t")]
    struct _KeySymMapIterator
    {
        int rem;
        int index;
        unowned KeySymMap? data;
    }

    [CCode (cname = "xcb_xkb_key_sym_map_iterator_t")]
    public struct KeySymMapIterator
    {
        [CCode (cname = "xcb_xkb_key_sym_map_next")]
        void _next ();

        public inline unowned KeySymMap?
        next_value ()
        {
            if (((_KeySymMapIterator)this).rem > 0)
            {
                unowned KeySymMap? d = ((_KeySymMapIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_sym_map_t", has_type_id = false)]
    public struct KeySymMap {
        public uint8 groupInfo;
        public uint8 width;
        public uint16 nSyms;
    }

    [SimpleType, CCode (cname = "xcb_xkb_common_behavior_iterator_t")]
    struct _CommonBehaviorIterator
    {
        int rem;
        int index;
        unowned CommonBehavior? data;
    }

    [CCode (cname = "xcb_xkb_common_behavior_iterator_t")]
    public struct CommonBehaviorIterator
    {
        [CCode (cname = "xcb_xkb_common_behavior_next")]
        void _next ();

        public inline unowned CommonBehavior?
        next_value ()
        {
            if (((_CommonBehaviorIterator)this).rem > 0)
            {
                unowned CommonBehavior? d = ((_CommonBehaviorIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_common_behavior_t", has_type_id = false)]
    public struct CommonBehavior {
        public uint8 type;
        public uint8 data;
    }

    [SimpleType, CCode (cname = "xcb_xkb_default_behavior_iterator_t")]
    struct _DefaultBehaviorIterator
    {
        int rem;
        int index;
        unowned DefaultBehavior? data;
    }

    [CCode (cname = "xcb_xkb_default_behavior_iterator_t")]
    public struct DefaultBehaviorIterator
    {
        [CCode (cname = "xcb_xkb_default_behavior_next")]
        void _next ();

        public inline unowned DefaultBehavior?
        next_value ()
        {
            if (((_DefaultBehaviorIterator)this).rem > 0)
            {
                unowned DefaultBehavior? d = ((_DefaultBehaviorIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_default_behavior_t", has_type_id = false)]
    public struct DefaultBehavior {
        public uint8 type;
    }

    [SimpleType, CCode (cname = "xcb_xkb_lock_behavior_t", has_type_id = false)]
    public struct LockBehavior : DefaultBehavior {
    }

    [SimpleType, CCode (cname = "xcb_xkb_radio_group_behavior_iterator_t")]
    struct _RadioGroupBehaviorIterator
    {
        int rem;
        int index;
        unowned RadioGroupBehavior? data;
    }

    [CCode (cname = "xcb_xkb_radio_group_behavior_iterator_t")]
    public struct RadioGroupBehaviorIterator
    {
        [CCode (cname = "xcb_xkb_radio_group_behavior_next")]
        void _next ();

        public inline unowned RadioGroupBehavior?
        next_value ()
        {
            if (((_RadioGroupBehaviorIterator)this).rem > 0)
            {
                unowned RadioGroupBehavior? d = ((_RadioGroupBehaviorIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_radio_group_behavior_t", has_type_id = false)]
    public struct RadioGroupBehavior {
        public uint8 type;
        public uint8 group;
    }

    [SimpleType, CCode (cname = "xcb_xkb_overlay_behavior_iterator_t")]
    struct _OverlayBehaviorIterator
    {
        int rem;
        int index;
        unowned OverlayBehavior? data;
    }

    [CCode (cname = "xcb_xkb_overlay_behavior_iterator_t")]
    public struct OverlayBehaviorIterator
    {
        [CCode (cname = "xcb_xkb_overlay_behavior_next")]
        void _next ();

        public inline unowned OverlayBehavior?
        next_value ()
        {
            if (((_OverlayBehaviorIterator)this).rem > 0)
            {
                unowned OverlayBehavior? d = ((_OverlayBehaviorIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_overlay_behavior_t", has_type_id = false)]
    public struct OverlayBehavior {
        public uint8 type;
        public Keycode key;
    }

    [SimpleType, CCode (cname = "xcb_xkb_permament_lock_behavior_t", has_type_id = false)]
    public struct PermamentLockBehavior : LockBehavior {
    }

    [SimpleType, CCode (cname = "xcb_xkb_permament_radio_group_behavior_t", has_type_id = false)]
    public struct PermamentRadioGroupBehavior : RadioGroupBehavior {
    }

    [SimpleType, CCode (cname = "xcb_xkb_permament_overlay_behavior_t", has_type_id = false)]
    public struct PermamentOverlayBehavior : OverlayBehavior {
    }

    [SimpleType, CCode (cname = "xcb_xkb_behavior_t", has_type_id = false)]
    public struct Behavior {
        public CommonBehavior common;
        public DefaultBehavior default;
        public LockBehavior lock;
        public RadioGroupBehavior radioGroup;
        public OverlayBehavior overlay1;
        public OverlayBehavior overlay2;
        public PermamentLockBehavior permamentLock;
        public PermamentRadioGroupBehavior permamentRadioGroup;
        public PermamentOverlayBehavior permamentOverlay1;
        public PermamentOverlayBehavior permamentOverlay2;
        public uint8 type;
    }

    [CCode (cname = "xcb_xkb_behavior_type_t", cprefix =  "XCB_XKB_BEHAVIOR_TYPE_", has_type_id = false)]
    public enum BehaviorType {
        DEFAULT,
        LOCK,
        RADIO_GROUP,
        OVERLAY_1,
        OVERLAY_2,
        PERMAMENT_LOCK,
        PERMAMENT_RADIO_GROUP,
        PERMAMENT_OVERLAY_1,
        PERMAMENT_OVERLAY_2
    }

    [SimpleType, CCode (cname = "xcb_xkb_set_behavior_iterator_t")]
    struct _SetBehaviorIterator
    {
        int rem;
        int index;
        unowned SetBehavior? data;
    }

    [CCode (cname = "xcb_xkb_set_behavior_iterator_t")]
    public struct SetBehaviorIterator
    {
        [CCode (cname = "xcb_xkb_set_behavior_next")]
        void _next ();

        public inline unowned SetBehavior?
        next_value ()
        {
            if (((_SetBehaviorIterator)this).rem > 0)
            {
                unowned SetBehavior? d = ((_SetBehaviorIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_set_behavior_t", has_type_id = false)]
    public struct SetBehavior {
        public Keycode keycode;
        public Behavior behavior;
    }

    [SimpleType, CCode (cname = "xcb_xkb_set_explicit_iterator_t")]
    struct _SetExplicitIterator
    {
        int rem;
        int index;
        unowned SetExplicit? data;
    }

    [CCode (cname = "xcb_xkb_set_explicit_iterator_t")]
    public struct SetExplicitIterator
    {
        [CCode (cname = "xcb_xkb_set_explicit_next")]
        void _next ();

        public inline unowned SetExplicit?
        next_value ()
        {
            if (((_SetExplicitIterator)this).rem > 0)
            {
                unowned SetExplicit? d = ((_SetExplicitIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_set_explicit_t", has_type_id = false)]
    public struct SetExplicit {
        public Keycode keycode;
        public Explicit explicit;
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_mod_map_iterator_t")]
    struct _KeyModMapIterator
    {
        int rem;
        int index;
        unowned KeyModMap? data;
    }

    [CCode (cname = "xcb_xkb_key_mod_map_iterator_t")]
    public struct KeyModMapIterator
    {
        [CCode (cname = "xcb_xkb_key_mod_map_next")]
        void _next ();

        public inline unowned KeyModMap?
        next_value ()
        {
            if (((_KeyModMapIterator)this).rem > 0)
            {
                unowned KeyModMap? d = ((_KeyModMapIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_mod_map_t", has_type_id = false)]
    public struct KeyModMap {
        public Keycode keycode;
        public uint8 mods;
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_v_mod_map_iterator_t")]
    struct _KeyVmodMapIterator
    {
        int rem;
        int index;
        unowned KeyVmodMap? data;
    }

    [CCode (cname = "xcb_xkb_key_v_mod_map_iterator_t")]
    public struct KeyVmodMapIterator
    {
        [CCode (cname = "xcb_xkb_key_v_mod_map_next")]
        void _next ();

        public inline unowned KeyVmodMap?
        next_value ()
        {
            if (((_KeyVmodMapIterator)this).rem > 0)
            {
                unowned KeyVmodMap? d = ((_KeyVmodMapIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_v_mod_map_t", has_type_id = false)]
    public struct KeyVmodMap {
        public Keycode keycode;
        public Vmod vmods;
    }

    [SimpleType, CCode (cname = "xcb_xkb_kt_set_map_entry_iterator_t")]
    struct _KtsetMapEntryIterator
    {
        int rem;
        int index;
        unowned KtsetMapEntry? data;
    }

    [CCode (cname = "xcb_xkb_kt_set_map_entry_iterator_t")]
    public struct KtsetMapEntryIterator
    {
        [CCode (cname = "xcb_xkb_kt_set_map_entry_next")]
        void _next ();

        public inline unowned KtsetMapEntry?
        next_value ()
        {
            if (((_KtsetMapEntryIterator)this).rem > 0)
            {
                unowned KtsetMapEntry? d = ((_KtsetMapEntryIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_kt_set_map_entry_t", has_type_id = false)]
    public struct KtsetMapEntry {
        public uint8 level;
        public uint8 realMods;
        public Vmod virtualMods;
    }

    [SimpleType, CCode (cname = "xcb_xkb_set_key_type_iterator_t")]
    struct _SetKeyTypeIterator
    {
        int rem;
        int index;
        unowned SetKeyType? data;
    }

    [CCode (cname = "xcb_xkb_set_key_type_iterator_t")]
    public struct SetKeyTypeIterator
    {
        [CCode (cname = "xcb_xkb_set_key_type_next")]
        void _next ();

        public inline unowned SetKeyType?
        next_value ()
        {
            if (((_SetKeyTypeIterator)this).rem > 0)
            {
                unowned SetKeyType? d = ((_SetKeyTypeIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_set_key_type_t", has_type_id = false)]
    public struct SetKeyType {
        public uint8 mask;
        public uint8 realMods;
        public Vmod virtualMods;
        public uint8 numLevels;
        public uint8 nMapEntries;
        public bool preserve;
        [CCode (cname = "xcb_xkb_set_key_type_preserve_entries_iterator")]
        _KtsetMapEntryIterator _iterator ();
        public KtsetMapEntryIterator iterator () {
            return (KtsetMapEntryIterator) _iterator ();
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_string8_t", has_type_id = false)]
    public struct String8 : char {
    }

    [SimpleType, CCode (cname = "xcb_xkb_outline_iterator_t")]
    struct _OutlineIterator
    {
        int rem;
        int index;
        unowned Outline? data;
    }

    [CCode (cname = "xcb_xkb_outline_iterator_t")]
    public struct OutlineIterator
    {
        [CCode (cname = "xcb_xkb_outline_next")]
        void _next ();

        public inline unowned Outline?
        next_value ()
        {
            if (((_OutlineIterator)this).rem > 0)
            {
                unowned Outline? d = ((_OutlineIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_outline_t", has_type_id = false)]
    public struct Outline {
        public uint8 nPoints;
        public uint8 cornerRadius;
        [CCode (cname = "xcb_xkb_outline_points_iterator")]
        _PointIterator _iterator ();
        public PointIterator iterator () {
            return (PointIterator) _iterator ();
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_shape_iterator_t")]
    struct _ShapeIterator
    {
        int rem;
        int index;
        unowned Shape? data;
    }

    [CCode (cname = "xcb_xkb_shape_iterator_t")]
    public struct ShapeIterator
    {
        [CCode (cname = "xcb_xkb_shape_next")]
        void _next ();

        public inline unowned Shape?
        next_value ()
        {
            if (((_ShapeIterator)this).rem > 0)
            {
                unowned Shape? d = ((_ShapeIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_shape_t", has_type_id = false)]
    public struct Shape {
        public Atom name;
        public uint8 nOutlines;
        public uint8 primaryNdx;
        public uint8 approxNdx;
        [CCode (cname = "xcb_xkb_shape_outlines_iterator")]
        _OutlineIterator _iterator ();
        public OutlineIterator iterator () {
            return (OutlineIterator) _iterator ();
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_iterator_t")]
    struct _KeyIterator
    {
        int rem;
        int index;
        unowned Key? data;
    }

    [CCode (cname = "xcb_xkb_key_iterator_t")]
    public struct KeyIterator
    {
        [CCode (cname = "xcb_xkb_key_next")]
        void _next ();

        public inline unowned Key?
        next_value ()
        {
            if (((_KeyIterator)this).rem > 0)
            {
                unowned Key? d = ((_KeyIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_key_t", has_type_id = false)]
    public struct Key {
        public int16 gap;
        public uint8 shapeNdx;
        public uint8 colorNdx;
    }

    [SimpleType, CCode (cname = "xcb_xkb_overlay_key_iterator_t")]
    struct _OverlayKeyIterator
    {
        int rem;
        int index;
        unowned OverlayKey? data;
    }

    [CCode (cname = "xcb_xkb_overlay_key_iterator_t")]
    public struct OverlayKeyIterator
    {
        [CCode (cname = "xcb_xkb_overlay_key_next")]
        void _next ();

        public inline unowned OverlayKey?
        next_value ()
        {
            if (((_OverlayKeyIterator)this).rem > 0)
            {
                unowned OverlayKey? d = ((_OverlayKeyIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_overlay_key_t", has_type_id = false)]
    public struct OverlayKey {
    }

    [SimpleType, CCode (cname = "xcb_xkb_overlay_row_iterator_t")]
    struct _OverlayRowIterator
    {
        int rem;
        int index;
        unowned OverlayRow? data;
    }

    [CCode (cname = "xcb_xkb_overlay_row_iterator_t")]
    public struct OverlayRowIterator
    {
        [CCode (cname = "xcb_xkb_overlay_row_next")]
        void _next ();

        public inline unowned OverlayRow?
        next_value ()
        {
            if (((_OverlayRowIterator)this).rem > 0)
            {
                unowned OverlayRow? d = ((_OverlayRowIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_overlay_row_t", has_type_id = false)]
    public struct OverlayRow {
        public uint8 rowUnder;
        public uint8 nKeys;
        [CCode (cname = "xcb_xkb_overlay_row_keys_iterator")]
        _OverlayKeyIterator _iterator ();
        public OverlayKeyIterator iterator () {
            return (OverlayKeyIterator) _iterator ();
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_overlay_iterator_t")]
    struct _OverlayIterator
    {
        int rem;
        int index;
        unowned Overlay? data;
    }

    [CCode (cname = "xcb_xkb_overlay_iterator_t")]
    public struct OverlayIterator
    {
        [CCode (cname = "xcb_xkb_overlay_next")]
        void _next ();

        public inline unowned Overlay?
        next_value ()
        {
            if (((_OverlayIterator)this).rem > 0)
            {
                unowned Overlay? d = ((_OverlayIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_overlay_t", has_type_id = false)]
    public struct Overlay {
        public Atom name;
        public uint8 nRows;
        [CCode (cname = "xcb_xkb_overlay_rows_iterator")]
        _OverlayRowIterator _iterator ();
        public OverlayRowIterator iterator () {
            return (OverlayRowIterator) _iterator ();
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_row_iterator_t")]
    struct _RowIterator
    {
        int rem;
        int index;
        unowned Row? data;
    }

    [CCode (cname = "xcb_xkb_row_iterator_t")]
    public struct RowIterator
    {
        [CCode (cname = "xcb_xkb_row_next")]
        void _next ();

        public inline unowned Row?
        next_value ()
        {
            if (((_RowIterator)this).rem > 0)
            {
                unowned Row? d = ((_RowIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_row_t", has_type_id = false)]
    public struct Row {
        public int16 top;
        public int16 left;
        public uint8 nKeys;
        public bool vertical;
        [CCode (cname = "xcb_xkb_row_keys_iterator")]
        _KeyIterator _iterator ();
        public KeyIterator iterator () {
            return (KeyIterator) _iterator ();
        }
    }

    [CCode (cname = "xcb_xkb_doodad_type_t", cprefix =  "XCB_XKB_DOODAD_TYPE_", has_type_id = false)]
    public enum DoodadType {
        OUTLINE,
        SOLID,
        TEXT,
        INDICATOR,
        LOGO
    }

    [SimpleType, CCode (cname = "xcb_xkb_listing_iterator_t")]
    struct _ListingIterator
    {
        int rem;
        int index;
        unowned Listing? data;
    }

    [CCode (cname = "xcb_xkb_listing_iterator_t")]
    public struct ListingIterator
    {
        [CCode (cname = "xcb_xkb_listing_next")]
        void _next ();

        public inline unowned Listing?
        next_value ()
        {
            if (((_ListingIterator)this).rem > 0)
            {
                unowned Listing? d = ((_ListingIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_listing_t", has_type_id = false)]
    public struct Listing {
        public uint16 flags;
        public uint16 length;
    }

    [SimpleType, CCode (cname = "xcb_xkb_device_led_info_iterator_t")]
    struct _DeviceLedInfoIterator
    {
        int rem;
        int index;
        unowned DeviceLedInfo? data;
    }

    [CCode (cname = "xcb_xkb_device_led_info_iterator_t")]
    public struct DeviceLedInfoIterator
    {
        [CCode (cname = "xcb_xkb_device_led_info_next")]
        void _next ();

        public inline unowned DeviceLedInfo?
        next_value ()
        {
            if (((_DeviceLedInfoIterator)this).rem > 0)
            {
                unowned DeviceLedInfo? d = ((_DeviceLedInfoIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_device_led_info_t", has_type_id = false)]
    public struct DeviceLedInfo {
        public LedClass ledClass;
        public Idspec ledID;
        public uint32 namesPresent;
        public uint32 mapsPresent;
        public uint32 physIndicators;
        public uint32 state;
        [CCode (cname = "xcb_xkb_device_led_info_maps_iterator")]
        _IndicatorMapIterator _iterator ();
        public IndicatorMapIterator iterator () {
            return (IndicatorMapIterator) _iterator ();
        }
    }

    [CCode (cname = "xcb_xkb_error_t", cprefix =  "XCB_XKB_ERROR_", has_type_id = false)]
    public enum Error {
        BAD_DEVICE,
        BAD_CLASS,
        BAD_ID
    }

    [Compact, CCode (cname = "xcb_xkb_keyboard_error_t", has_type_id = false)]
    public class KeyboardError : Xcb.GenericError {
        public uint32 value;
        public uint16 minorOpcode;
        public uint8 majorOpcode;
    }

    [Flags, CCode (cname = "xcb_xkb_sa_t", cprefix =  "XCB_XKB_SA_", has_type_id = false)]
    public enum Sa {
        CLEAR_LOCKS,
        LATCH_TO_LOCK,
        USE_MOD_MAP_MODS,
        GROUP_ABSOLUTE
    }

    [CCode (cname = "xcb_xkb_sa_type_t", cprefix =  "XCB_XKB_SATYPE_", has_type_id = false)]
    public enum Satype {
        NO_ACTION,
        SET_MODS,
        LATCH_MODS,
        LOCK_MODS,
        SET_GROUP,
        LATCH_GROUP,
        LOCK_GROUP,
        MOVE_PTR,
        PTR_BTN,
        LOCK_PTR_BTN,
        SET_PTR_DFLT,
        ISO_LOCK,
        TERMINATE,
        SWITCH_SCREEN,
        SET_CONTROLS,
        LOCK_CONTROLS,
        ACTION_MESSAGE,
        REDIRECT_KEY,
        DEVICE_BTN,
        LOCK_DEVICE_BTN,
        DEVICE_VALUATOR
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_no_action_iterator_t")]
    struct _SanoActionIterator
    {
        int rem;
        int index;
        unowned SanoAction? data;
    }

    [CCode (cname = "xcb_xkb_sa_no_action_iterator_t")]
    public struct SanoActionIterator
    {
        [CCode (cname = "xcb_xkb_sa_no_action_next")]
        void _next ();

        public inline unowned SanoAction?
        next_value ()
        {
            if (((_SanoActionIterator)this).rem > 0)
            {
                unowned SanoAction? d = ((_SanoActionIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_no_action_t", has_type_id = false)]
    public struct SanoAction {
        public Satype type;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_set_mods_iterator_t")]
    struct _SasetModsIterator
    {
        int rem;
        int index;
        unowned SasetMods? data;
    }

    [CCode (cname = "xcb_xkb_sa_set_mods_iterator_t")]
    public struct SasetModsIterator
    {
        [CCode (cname = "xcb_xkb_sa_set_mods_next")]
        void _next ();

        public inline unowned SasetMods?
        next_value ()
        {
            if (((_SasetModsIterator)this).rem > 0)
            {
                unowned SasetMods? d = ((_SasetModsIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_set_mods_t", has_type_id = false)]
    public struct SasetMods {
        public Satype type;
        public Sa flags;
        public uint8 mask;
        public uint8 realMods;
        public VmodsHigh vmodsHigh;
        public VmodsLow vmodsLow;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_latch_mods_t", has_type_id = false)]
    public struct SalatchMods : SasetMods {
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_lock_mods_t", has_type_id = false)]
    public struct SalockMods : SasetMods {
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_set_group_iterator_t")]
    struct _SasetGroupIterator
    {
        int rem;
        int index;
        unowned SasetGroup? data;
    }

    [CCode (cname = "xcb_xkb_sa_set_group_iterator_t")]
    public struct SasetGroupIterator
    {
        [CCode (cname = "xcb_xkb_sa_set_group_next")]
        void _next ();

        public inline unowned SasetGroup?
        next_value ()
        {
            if (((_SasetGroupIterator)this).rem > 0)
            {
                unowned SasetGroup? d = ((_SasetGroupIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_set_group_t", has_type_id = false)]
    public struct SasetGroup {
        public Satype type;
        public Sa flags;
        public int8 group;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_latch_group_t", has_type_id = false)]
    public struct SalatchGroup : SasetGroup {
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_lock_group_t", has_type_id = false)]
    public struct SalockGroup : SasetGroup {
    }

    [Flags, CCode (cname = "xcb_xkb_sa_move_ptr_flag_t", cprefix =  "XCB_XKB_SAMOVE_PTR_FLAG_", has_type_id = false)]
    public enum SamovePtrFlag {
        NO_ACCELERATION,
        MOVE_ABSOLUTE_X,
        MOVE_ABSOLUTE_Y
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_move_ptr_iterator_t")]
    struct _SamovePtrIterator
    {
        int rem;
        int index;
        unowned SamovePtr? data;
    }

    [CCode (cname = "xcb_xkb_sa_move_ptr_iterator_t")]
    public struct SamovePtrIterator
    {
        [CCode (cname = "xcb_xkb_sa_move_ptr_next")]
        void _next ();

        public inline unowned SamovePtr?
        next_value ()
        {
            if (((_SamovePtrIterator)this).rem > 0)
            {
                unowned SamovePtr? d = ((_SamovePtrIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_move_ptr_t", has_type_id = false)]
    public struct SamovePtr {
        public Satype type;
        public SamovePtrFlag flags;
        public int8 xHigh;
        public uint8 xLow;
        public int8 yHigh;
        public uint8 yLow;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_ptr_btn_iterator_t")]
    struct _SaptrBtnIterator
    {
        int rem;
        int index;
        unowned SaptrBtn? data;
    }

    [CCode (cname = "xcb_xkb_sa_ptr_btn_iterator_t")]
    public struct SaptrBtnIterator
    {
        [CCode (cname = "xcb_xkb_sa_ptr_btn_next")]
        void _next ();

        public inline unowned SaptrBtn?
        next_value ()
        {
            if (((_SaptrBtnIterator)this).rem > 0)
            {
                unowned SaptrBtn? d = ((_SaptrBtnIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_ptr_btn_t", has_type_id = false)]
    public struct SaptrBtn {
        public Satype type;
        public uint8 flags;
        public uint8 count;
        public uint8 button;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_lock_ptr_btn_iterator_t")]
    struct _SalockPtrBtnIterator
    {
        int rem;
        int index;
        unowned SalockPtrBtn? data;
    }

    [CCode (cname = "xcb_xkb_sa_lock_ptr_btn_iterator_t")]
    public struct SalockPtrBtnIterator
    {
        [CCode (cname = "xcb_xkb_sa_lock_ptr_btn_next")]
        void _next ();

        public inline unowned SalockPtrBtn?
        next_value ()
        {
            if (((_SalockPtrBtnIterator)this).rem > 0)
            {
                unowned SalockPtrBtn? d = ((_SalockPtrBtnIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_lock_ptr_btn_t", has_type_id = false)]
    public struct SalockPtrBtn {
        public Satype type;
        public uint8 flags;
        public uint8 button;
    }

    [Flags, CCode (cname = "xcb_xkb_sa_set_ptr_dflt_flag_t", cprefix =  "XCB_XKB_SASET_PTR_DFLT_FLAG_", has_type_id = false)]
    public enum SasetPtrDfltFlag {
        DFLT_BTN_ABSOLUTE,
        AFFECT_DFLT_BUTTON
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_set_ptr_dflt_iterator_t")]
    struct _SasetPtrDfltIterator
    {
        int rem;
        int index;
        unowned SasetPtrDflt? data;
    }

    [CCode (cname = "xcb_xkb_sa_set_ptr_dflt_iterator_t")]
    public struct SasetPtrDfltIterator
    {
        [CCode (cname = "xcb_xkb_sa_set_ptr_dflt_next")]
        void _next ();

        public inline unowned SasetPtrDflt?
        next_value ()
        {
            if (((_SasetPtrDfltIterator)this).rem > 0)
            {
                unowned SasetPtrDflt? d = ((_SasetPtrDfltIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_set_ptr_dflt_t", has_type_id = false)]
    public struct SasetPtrDflt {
        public Satype type;
        public SasetPtrDfltFlag flags;
        public SasetPtrDfltFlag affect;
        public int8 value;
    }

    [Flags, CCode (cname = "xcb_xkb_sa_iso_lock_flag_t", cprefix =  "XCB_XKB_SAISO_LOCK_FLAG_", has_type_id = false)]
    public enum SaisoLockFlag {
        NO_LOCK,
        NO_UNLOCK,
        USE_MOD_MAP_MODS,
        GROUP_ABSOLUTE,
        ISO_DFLT_IS_GROUP
    }

    [Flags, CCode (cname = "xcb_xkb_sa_iso_lock_no_affect_t", cprefix =  "XCB_XKB_SAISO_LOCK_NO_AFFECT_", has_type_id = false)]
    public enum SaisoLockNoAffect {
        CTRLS,
        PTR,
        GROUP,
        MODS
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_iso_lock_iterator_t")]
    struct _SaisoLockIterator
    {
        int rem;
        int index;
        unowned SaisoLock? data;
    }

    [CCode (cname = "xcb_xkb_sa_iso_lock_iterator_t")]
    public struct SaisoLockIterator
    {
        [CCode (cname = "xcb_xkb_sa_iso_lock_next")]
        void _next ();

        public inline unowned SaisoLock?
        next_value ()
        {
            if (((_SaisoLockIterator)this).rem > 0)
            {
                unowned SaisoLock? d = ((_SaisoLockIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_iso_lock_t", has_type_id = false)]
    public struct SaisoLock {
        public Satype type;
        public SaisoLockFlag flags;
        public uint8 mask;
        public uint8 realMods;
        public int8 group;
        public SaisoLockNoAffect affect;
        public VmodsHigh vmodsHigh;
        public VmodsLow vmodsLow;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_terminate_iterator_t")]
    struct _SaterminateIterator
    {
        int rem;
        int index;
        unowned Saterminate? data;
    }

    [CCode (cname = "xcb_xkb_sa_terminate_iterator_t")]
    public struct SaterminateIterator
    {
        [CCode (cname = "xcb_xkb_sa_terminate_next")]
        void _next ();

        public inline unowned Saterminate?
        next_value ()
        {
            if (((_SaterminateIterator)this).rem > 0)
            {
                unowned Saterminate? d = ((_SaterminateIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_terminate_t", has_type_id = false)]
    public struct Saterminate {
        public Satype type;
    }

    [CCode (cname = "xcb_xkb_switch_screen_flag_t", cprefix =  "XCB_XKB_SWITCH_SCREEN_FLAG_", has_type_id = false)]
    public enum SwitchScreenFlag {
        APPLICATION,
        ABSOLUTE
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_switch_screen_iterator_t")]
    struct _SaswitchScreenIterator
    {
        int rem;
        int index;
        unowned SaswitchScreen? data;
    }

    [CCode (cname = "xcb_xkb_sa_switch_screen_iterator_t")]
    public struct SaswitchScreenIterator
    {
        [CCode (cname = "xcb_xkb_sa_switch_screen_next")]
        void _next ();

        public inline unowned SaswitchScreen?
        next_value ()
        {
            if (((_SaswitchScreenIterator)this).rem > 0)
            {
                unowned SaswitchScreen? d = ((_SaswitchScreenIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_switch_screen_t", has_type_id = false)]
    public struct SaswitchScreen {
        public Satype type;
        public uint8 flags;
        public int8 newScreen;
    }

    [Flags, CCode (cname = "xcb_xkb_bool_ctrls_high_t", cprefix =  "XCB_XKB_BOOL_CTRLS_HIGH_", has_type_id = false)]
    public enum BoolCtrlsHigh {
        ACCESS_X_FEEDBACK,
        AUDIBLE_BELL,
        OVERLAY_1,
        OVERLAY_2,
        IGNORE_GROUP_LOCK
    }

    [Flags, CCode (cname = "xcb_xkb_bool_ctrls_low_t", cprefix =  "XCB_XKB_BOOL_CTRLS_LOW_", has_type_id = false)]
    public enum BoolCtrlsLow {
        REPEAT_KEYS,
        SLOW_KEYS,
        BOUNCE_KEYS,
        STICKY_KEYS,
        MOUSE_KEYS,
        MOUSE_KEYS_ACCEL,
        ACCESS_X_KEYS,
        ACCESS_X_TIMEOUT
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_set_controls_iterator_t")]
    struct _SasetControlsIterator
    {
        int rem;
        int index;
        unowned SasetControls? data;
    }

    [CCode (cname = "xcb_xkb_sa_set_controls_iterator_t")]
    public struct SasetControlsIterator
    {
        [CCode (cname = "xcb_xkb_sa_set_controls_next")]
        void _next ();

        public inline unowned SasetControls?
        next_value ()
        {
            if (((_SasetControlsIterator)this).rem > 0)
            {
                unowned SasetControls? d = ((_SasetControlsIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_set_controls_t", has_type_id = false)]
    public struct SasetControls {
        public Satype type;
        public BoolCtrlsHigh boolCtrlsHigh;
        public BoolCtrlsLow boolCtrlsLow;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_lock_controls_t", has_type_id = false)]
    public struct SalockControls : SasetControls {
    }

    [Flags, CCode (cname = "xcb_xkb_action_message_flag_t", cprefix =  "XCB_XKB_ACTION_MESSAGE_FLAG_", has_type_id = false)]
    public enum ActionMessageFlag {
        ON_PRESS,
        ON_RELEASE,
        GEN_KEY_EVENT
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_action_message_iterator_t")]
    struct _SaactionMessageIterator
    {
        int rem;
        int index;
        unowned SaactionMessage? data;
    }

    [CCode (cname = "xcb_xkb_sa_action_message_iterator_t")]
    public struct SaactionMessageIterator
    {
        [CCode (cname = "xcb_xkb_sa_action_message_next")]
        void _next ();

        public inline unowned SaactionMessage?
        next_value ()
        {
            if (((_SaactionMessageIterator)this).rem > 0)
            {
                unowned SaactionMessage? d = ((_SaactionMessageIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_action_message_t", has_type_id = false)]
    public struct SaactionMessage {
        public Satype type;
        public ActionMessageFlag flags;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_redirect_key_iterator_t")]
    struct _SaredirectKeyIterator
    {
        int rem;
        int index;
        unowned SaredirectKey? data;
    }

    [CCode (cname = "xcb_xkb_sa_redirect_key_iterator_t")]
    public struct SaredirectKeyIterator
    {
        [CCode (cname = "xcb_xkb_sa_redirect_key_next")]
        void _next ();

        public inline unowned SaredirectKey?
        next_value ()
        {
            if (((_SaredirectKeyIterator)this).rem > 0)
            {
                unowned SaredirectKey? d = ((_SaredirectKeyIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_redirect_key_t", has_type_id = false)]
    public struct SaredirectKey {
        public Satype type;
        public Keycode newkey;
        public uint8 mask;
        public uint8 realModifiers;
        public VmodsHigh vmodsMaskHigh;
        public VmodsLow vmodsMaskLow;
        public VmodsHigh vmodsHigh;
        public VmodsLow vmodsLow;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_device_btn_iterator_t")]
    struct _SadeviceBtnIterator
    {
        int rem;
        int index;
        unowned SadeviceBtn? data;
    }

    [CCode (cname = "xcb_xkb_sa_device_btn_iterator_t")]
    public struct SadeviceBtnIterator
    {
        [CCode (cname = "xcb_xkb_sa_device_btn_next")]
        void _next ();

        public inline unowned SadeviceBtn?
        next_value ()
        {
            if (((_SadeviceBtnIterator)this).rem > 0)
            {
                unowned SadeviceBtn? d = ((_SadeviceBtnIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_device_btn_t", has_type_id = false)]
    public struct SadeviceBtn {
        public Satype type;
        public uint8 flags;
        public uint8 count;
        public uint8 button;
        public uint8 device;
    }

    [Flags, CCode (cname = "xcb_xkb_lock_device_flags_t", cprefix =  "XCB_XKB_LOCK_DEVICE_FLAGS_", has_type_id = false)]
    public enum LockDeviceFlags {
        NO_LOCK,
        NO_UNLOCK
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_lock_device_btn_iterator_t")]
    struct _SalockDeviceBtnIterator
    {
        int rem;
        int index;
        unowned SalockDeviceBtn? data;
    }

    [CCode (cname = "xcb_xkb_sa_lock_device_btn_iterator_t")]
    public struct SalockDeviceBtnIterator
    {
        [CCode (cname = "xcb_xkb_sa_lock_device_btn_next")]
        void _next ();

        public inline unowned SalockDeviceBtn?
        next_value ()
        {
            if (((_SalockDeviceBtnIterator)this).rem > 0)
            {
                unowned SalockDeviceBtn? d = ((_SalockDeviceBtnIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_lock_device_btn_t", has_type_id = false)]
    public struct SalockDeviceBtn {
        public Satype type;
        public LockDeviceFlags flags;
        public uint8 button;
        public uint8 device;
    }

    [CCode (cname = "xcb_xkb_sa_val_what_t", cprefix =  "XCB_XKB_SAVAL_WHAT_", has_type_id = false)]
    public enum SavalWhat {
        IGNORE_VAL,
        SET_VAL_MIN,
        SET_VAL_CENTER,
        SET_VAL_MAX,
        SET_VAL_RELATIVE,
        SET_VAL_ABSOLUTE
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_device_valuator_iterator_t")]
    struct _SadeviceValuatorIterator
    {
        int rem;
        int index;
        unowned SadeviceValuator? data;
    }

    [CCode (cname = "xcb_xkb_sa_device_valuator_iterator_t")]
    public struct SadeviceValuatorIterator
    {
        [CCode (cname = "xcb_xkb_sa_device_valuator_next")]
        void _next ();

        public inline unowned SadeviceValuator?
        next_value ()
        {
            if (((_SadeviceValuatorIterator)this).rem > 0)
            {
                unowned SadeviceValuator? d = ((_SadeviceValuatorIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sa_device_valuator_t", has_type_id = false)]
    public struct SadeviceValuator {
        public Satype type;
        public uint8 device;
        public SavalWhat val1what;
        public uint8 val1index;
        public uint8 val1value;
        public SavalWhat val2what;
        public uint8 val2index;
        public uint8 val2value;
    }

    [SimpleType, CCode (cname = "xcb_xkb_si_action_iterator_t")]
    struct _SiactionIterator
    {
        int rem;
        int index;
        unowned Siaction? data;
    }

    [CCode (cname = "xcb_xkb_si_action_iterator_t")]
    public struct SiactionIterator
    {
        [CCode (cname = "xcb_xkb_si_action_next")]
        void _next ();

        public inline unowned Siaction?
        next_value ()
        {
            if (((_SiactionIterator)this).rem > 0)
            {
                unowned Siaction? d = ((_SiactionIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_si_action_t", has_type_id = false)]
    public struct Siaction {
        public Satype type;
    }

    [SimpleType, CCode (cname = "xcb_xkb_sym_interpret_iterator_t")]
    struct _SymInterpretIterator
    {
        int rem;
        int index;
        unowned SymInterpret? data;
    }

    [CCode (cname = "xcb_xkb_sym_interpret_iterator_t")]
    public struct SymInterpretIterator
    {
        [CCode (cname = "xcb_xkb_sym_interpret_next")]
        void _next ();

        public inline unowned SymInterpret?
        next_value ()
        {
            if (((_SymInterpretIterator)this).rem > 0)
            {
                unowned SymInterpret? d = ((_SymInterpretIterator)this).data;
                _next ();
                return d;
            }
            return null;
        }
    }

    [SimpleType, CCode (cname = "xcb_xkb_sym_interpret_t", has_type_id = false)]
    public struct SymInterpret {
        public Keysym sym;
        public uint8 mods;
        public uint8 match;
        public VmodsLow virtualMod;
        public uint8 flags;
        public Siaction action;
    }

    [SimpleType, CCode (cname = "xcb_xkb_action_t", has_type_id = false)]
    public struct Action {
        public SanoAction noaction;
        public SasetMods setmods;
        public SalatchMods latchmods;
        public SalockMods lockmods;
        public SasetGroup setgroup;
        public SalatchGroup latchgroup;
        public SalockGroup lockgroup;
        public SamovePtr moveptr;
        public SaptrBtn ptrbtn;
        public SalockPtrBtn lockptrbtn;
        public SasetPtrDflt setptrdflt;
        public SaisoLock isolock;
        public Saterminate terminate;
        public SaswitchScreen switchscreen;
        public SasetControls setcontrols;
        public SalockControls lockcontrols;
        public SaactionMessage message;
        public SaredirectKey redirect;
        public SadeviceBtn devbtn;
        public SalockDeviceBtn lockdevbtn;
        public SadeviceValuator devval;
        public Satype type;
    }

    [Compact, CCode (cname = "xcb_xkb_new_keyboard_notify_event_t", has_type_id = false)]
    public class NewKeyboardNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public uint8 oldDeviceID;
        public Keycode minKeyCode;
        public Keycode maxKeyCode;
        public Keycode oldMinKeyCode;
        public Keycode oldMaxKeyCode;
        public uint8 requestMajor;
        public uint8 requestMinor;
        public Nkndetail changed;
    }

    [Compact, CCode (cname = "xcb_xkb_map_notify_event_t", has_type_id = false)]
    public class MapNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public uint8 ptrBtnActions;
        public MapPart changed;
        public Keycode minKeyCode;
        public Keycode maxKeyCode;
        public uint8 firstType;
        public uint8 nTypes;
        public Keycode firstKeySym;
        public uint8 nKeySyms;
        public Keycode firstKeyAct;
        public uint8 nKeyActs;
        public Keycode firstKeyBehavior;
        public uint8 nKeyBehavior;
        public Keycode firstKeyExplicit;
        public uint8 nKeyExplicit;
        public Keycode firstModMapKey;
        public uint8 nModMapKeys;
        public Keycode firstVModMapKey;
        public uint8 nVModMapKeys;
        public Vmod virtualMods;
    }

    [Compact, CCode (cname = "xcb_xkb_state_notify_event_t", has_type_id = false)]
    public class StateNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public uint8 mods;
        public uint8 baseMods;
        public uint8 latchedMods;
        public uint8 lockedMods;
        public Group group;
        public int16 baseGroup;
        public int16 latchedGroup;
        public Group lockedGroup;
        public uint8 compatState;
        public uint8 grabMods;
        public uint8 compatGrabMods;
        public uint8 lookupMods;
        public uint8 compatLoockupMods;
        public uint16 ptrBtnState;
        public StatePart changed;
        public Keycode keycode;
        public uint8 eventType;
        public uint8 requestMajor;
        public uint8 requestMinor;
    }

    [Compact, CCode (cname = "xcb_xkb_controls_notify_event_t", has_type_id = false)]
    public class ControlsNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public uint8 numGroups;
        public Control changedControls;
        public BoolCtrl enabledControls;
        public BoolCtrl enabledControlChanges;
        public Keycode keycode;
        public uint8 eventType;
        public uint8 requestMajor;
        public uint8 requestMinor;
    }

    [Compact, CCode (cname = "xcb_xkb_indicator_state_notify_event_t", has_type_id = false)]
    public class IndicatorStateNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public uint32 state;
        public uint32 stateChanged;
    }

    [Compact, CCode (cname = "xcb_xkb_indicator_map_notify_event_t", has_type_id = false)]
    public class IndicatorMapNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public uint32 state;
        public uint32 mapChanged;
    }

    [Compact, CCode (cname = "xcb_xkb_names_notify_event_t", has_type_id = false)]
    public class NamesNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public NameDetail changed;
        public uint8 firstType;
        public uint8 nTypes;
        public uint8 firstLevelName;
        public uint8 nLevelNames;
        public uint8 nRadioGroups;
        public uint8 nKeyAliases;
        public SetOfGroup changedGroupNames;
        public Vmod changedVirtualMods;
        public Keycode firstKey;
        public uint8 nKeys;
        public uint32 changedIndicators;
    }

    [Compact, CCode (cname = "xcb_xkb_compat_map_notify_event_t", has_type_id = false)]
    public class CompatMapNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public SetOfGroup changedGroups;
        public uint16 firstSI;
        public uint16 nSI;
        public uint16 nTotalSI;
    }

    [Compact, CCode (cname = "xcb_xkb_bell_notify_event_t", has_type_id = false)]
    public class BellNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public BellClassResult bellClass;
        public uint8 bellID;
        public uint8 percent;
        public uint16 pitch;
        public uint16 duration;
        public Atom name;
        public Window window;
        public bool eventOnly;
    }

    [Compact, CCode (cname = "xcb_xkb_action_message_event_t", has_type_id = false)]
    public class ActionMessageEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public Keycode keycode;
        public bool press;
        public bool keyEventFollows;
        public uint8 mods;
        public Group group;
        public String8 message[8];
    }

    [Compact, CCode (cname = "xcb_xkb_access_x_notify_event_t", has_type_id = false)]
    public class AccessXnotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public Keycode keycode;
        public Axndetail detailt;
        public uint16 slowKeysDelay;
        public uint16 debounceDelay;
    }

    [Compact, CCode (cname = "xcb_xkb_extension_device_notify_event_t", has_type_id = false)]
    public class ExtensionDeviceNotifyEvent : Xcb.GenericEvent {
        public uint8 xkbType;
        public Timestamp time;
        public uint8 deviceID;
        public XIFeature reason;
        public LedClassResult ledClass;
        public uint16 ledID;
        public uint32 ledsDefined;
        public uint32 ledState;
        public uint8 firstButton;
        public uint8 nButtons;
        public XIFeature supported;
        public XIFeature unsupported;
    }
}
