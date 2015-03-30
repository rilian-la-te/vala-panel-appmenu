[CCode (cprefix = "XKB_X11_", lower_case_cprefix = "xkb_x11_", cheader_filename = "xkbcommon/xkbcommon-x11.h")]
namespace Xkb.X11
{
    public const uint16 MIN_MAJOR_XKB_VERSION;
    public const uint16 MIN_MINOR_XKB_VERSION;
    [Flags, CCode (cname = "xkb_x11_setup_xkb_extension_flags", cprefix = "XKB_X11_SETUP_XKB_EXTENSION_")]
    public enum ExtensionFlags
    {
        NO_FLAGS
    }
    public bool setup_xkb_extension (Xcb.Connection connection,uint16 major_ver = MIN_MAJOR_XKB_VERSION, uint16 minor_ver = MIN_MINOR_XKB_VERSION, ExtensionFlags flags = ExtensionFlags.NO_FLAGS, out uint16 major_v_out = null, out uint16 minor_v_out = null, out uint8 base_event_out = null, out uint8 base_event_error = null);
    public int32 get_core_keyboard_device_id(Xcb.Connection connection);
    public Xkb.Keymap keymap_new_from_device(Xkb.Context context, Xcb.Connection connection, int32 device_id, Xkb.KeymapCompileFlags flags = Xkb.KeymapCompileFlags.NO_FLAGS);
    public Xkb.State state_new_from_device(Xkb.Keymap keymap, Xcb.Connection connection, int32 device_id);
}
