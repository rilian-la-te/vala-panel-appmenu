[CCode (cprefix = "X", cheader_filename = "X11/Xlib-xcb.h")]
namespace X.xcb
{
    [CCode (cname = "XEventQueueOwner")]
    public enum EventQueueOwner
    {
        [CCode (cname = "XlibOwnsEventQueue")]
        XLIB,
        [CCode (cname = "XCBOwnsEventQueue")]
        XCB
    }
    [Compact, CCode (cname = "Display", free_function = "XCloseDisplay")]
    public class Display : X.Display
    {
        public Xcb.Connection connection
        {
            [CCode (cname = "XGetXCBConnection")]
            get;
        }
        [CCode (cname = "XOpenDisplay")]
        public Display(string? name = null);
        [CCode (cname = "XSetEventQueueOwner")]
        void set_event_queue_owner(EventQueueOwner owner);
    }
}
[CCode (cprefix = "X", lower_case_cprefix = "", cheader_filename = "X11/Xlib.h")]
namespace X
{
    [CCode (cname = "PropertyNotify")]
    public const int PropertyNotify;
}
[CCode (cprefix = "Xkb", lower_case_cprefix = "", cheader_filename = "X11/XKBlib.h")]
namespace X.Xkb
{
    [Compact]
    [CCode (free_function = "")]
    public struct AnyEvent {
        int xkb_type;
    }
    [Compact]
    [CCode (free_function = "")]
    public struct StateNotifyEvent {
        int group;
    }
    [Compact]
    [CCode (free_function = "")]
    public struct Event {
        int type;
        AnyEvent any;
        StateNotifyEvent state;
    }
    [CCode (cname = "XkbEventCode")]
    public const int EventCode;
    [CCode (cname = "XkbStateNotify")]
    public const int StateNotify;
    [CCode (cname = "XkbNewKeyboardNotify")]
    public const int NewKeyboardNotify;
}
