/*
 * appmenu-backend-wayfire.vala
 */

using GLib;

namespace Appmenu
{
    internal class WayfireClient : Object
    {
        public signal void view_focused  (int id, string title, string app_id, int pid);
        public signal void view_unmapped (int id);
        public signal void view_property (int view_id, string prop, string? value);
        public signal void view_minimized(int id, bool minimized);
        public signal void views_listed (Json.Array views);

        private enum ReplyType {
            VIEW_PROPERTY,
            LIST_VIEWS
        }

        private class PendingReply
        {
            public ReplyType type;
            public int    view_id;
            public string prop_name = "";
        }

        private SocketConnection conn;
        private OutputStream     ostream;
        private GLib.Queue<PendingReply> reply_queue;
        private Cancellable cancellable;

        private IOChannel channel;
        private uint watch_id = 0;
        private ByteArray read_buf;

        public WayfireClient() throws Error
        {
            string? wf_socket = Environment.get_variable("WAYFIRE_SOCKET");
            if (wf_socket == null || wf_socket == "")
                throw new IOError.NOT_FOUND("WAYFIRE_SOCKET is not set");

            var client = new SocketClient();
            conn = client.connect(new UnixSocketAddress(wf_socket));
            ostream     = conn.get_output_stream();
            cancellable = new Cancellable();
            reply_queue = new GLib.Queue<PendingReply>();
            read_buf    = new ByteArray();

            var socket = conn.get_socket();
            int fd = socket.get_fd();
            channel = new IOChannel.unix_new(fd);
            channel.set_encoding(null);
            channel.set_buffered(false);
            watch_id = channel.add_watch(
                IOCondition.IN | IOCondition.HUP | IOCondition.ERR,
                on_socket_event);

            var events = new Json.Array();
            events.add_string_element("view-focused");
            events.add_string_element("view-unmapped");
            events.add_string_element("view-minimized");

            var data = new Json.Object();
            data.set_array_member("events", events);
            send_msg("window-rules/events/watch", data);
        }

        ~WayfireClient()
        {
            if (watch_id > 0)
                Source.remove(watch_id);
            if (cancellable != null)
                cancellable.cancel();
        }

        private void send_msg(string method, Json.Object? data)
        {
            var root = new Json.Object();
            root.set_string_member("method", method);
            if (data != null)
                root.set_object_member("data", data);

            var node = new Json.Node(Json.NodeType.OBJECT);
            node.set_object(root);
            var gen = new Json.Generator();
            gen.set_root(node);

            size_t len;
            string s = gen.to_data(out len);

            uint8[] pkt = new uint8[4 + (int) len];
            uint32 L = (uint32) len;
            pkt[0] = (uint8)( L        & 0xff);
            pkt[1] = (uint8)((L >>  8) & 0xff);
            pkt[2] = (uint8)((L >> 16) & 0xff);
            pkt[3] = (uint8)((L >> 24) & 0xff);
            for (int i = 0; i < (int) len; i++)
                pkt[4 + i] = (uint8) s[i];

            try {
                size_t written;
                ostream.write_all(pkt, out written);
                ostream.flush(cancellable);
            } catch (Error e) {
                warning("Wayfire send failed: %s", e.message);
            }
        }

        private bool on_socket_event(IOChannel source, IOCondition cond)
        {
            if ((cond & (IOCondition.HUP | IOCondition.ERR)) != 0)
                return false;

            while (true) {
                char[] tmp = new char[4096];
                size_t n;
                IOStatus status;
                try {
                    status = source.read_chars(tmp, out n);
                } catch (Error e) {
                    warning("Wayfire read failed: %s", e.message);
                    return false;
                }
                if (status == IOStatus.AGAIN || n == 0)
                    break;
                if (status != IOStatus.NORMAL)
                    return false;

                uint8[] chunk = new uint8[(int) n];
                for (size_t i = 0; i < n; i++)
                    chunk[i] = (uint8) tmp[i];
                read_buf.append(chunk);
            }

            while (read_buf.len >= 4) {
                unowned uint8* data = read_buf.data;
                uint32 L = (uint32) data[0]
                         | ((uint32) data[1] <<  8)
                         | ((uint32) data[2] << 16)
                         | ((uint32) data[3] << 24);

                if (read_buf.len < 4 + L)
                    break;

                uint8[] body = new uint8[(int) L + 1];
                Memory.copy(body, data + 4, L);
                body[L] = 0;
                read_buf.remove_range(0, 4 + (uint) L);

                try {
                    var parser = new Json.Parser();
                    if (!parser.load_from_data((string) body, (ssize_t) L))
                        continue;
                    var root_node = parser.get_root();
                    if (root_node.get_node_type() == Json.NodeType.ARRAY)
                        process_array_reply(root_node.get_array());
                    else
                        process_message(root_node.get_object());
                } catch (Error e) {
                    warning("Wayfire parse failed: %s", e.message);
                }
            }
            return true;
        }

        private void process_message(Json.Object msg)
        {
            if (msg.has_member("event")) {
                string ev = msg.get_string_member("event");
                if      (ev == "view-focused")  dispatch_focused(msg);
                else if (ev == "view-unmapped") dispatch_unmapped(msg);
                else if (ev == "view-minimized") dispatch_minimized(msg);
                return;
            }

            var reply = reply_queue.pop_head();
            if (reply == null) {
                warning("Wayfire: unexpected reply");
                return;
            }

            string? value = null;
            if (msg.has_member("value") && !msg.get_member("value").is_null())
                value = msg.get_string_member("value");

            view_property(reply.view_id, reply.prop_name, value);
        }
        private void process_array_reply(Json.Array arr)
        {
            var reply = reply_queue.pop_head();
            if (reply == null || reply.type != ReplyType.LIST_VIEWS) {
                warning("Wayfire: unexpected array reply");
                return;
            }
            views_listed(arr);
        }

        private void dispatch_focused(Json.Object msg)
        {
            if (!msg.has_member("view") || msg.get_member("view").is_null())
                return;

            var view = msg.get_object_member("view");
            if (!view.has_member("id"))
                return;
            int id = (int) view.get_int_member("id");
            int pid = (int) view.get_int_member("pid");
            string title = "";
            string app_id = "";
            if (view.has_member("title") && !view.get_member("title").is_null())
                title = view.get_string_member("title");
            if (view.has_member("app-id") && !view.get_member("app-id").is_null())
                app_id = view.get_string_member("app-id");

            view_focused(id, title, app_id, pid);
        }

        private void dispatch_unmapped(Json.Object msg)
        {
            if (!msg.has_member("view") || msg.get_member("view").is_null())
                return;

            var view = msg.get_object_member("view");
            if (!view.has_member("id"))
                return;
            view_unmapped((int) view.get_int_member("id"));
        }

        private void dispatch_minimized(Json.Object msg)
        {
            if (!msg.has_member("view") || msg.get_member("view").is_null())
                return;

            var view = msg.get_object_member("view");
            if (!view.has_member("id"))
                return;

            int id = (int) view.get_int_member("id");
            bool minimized = view.has_member("minimized") ? view.get_boolean_member("minimized") : true;
            view_minimized(id, minimized);
        }

        public void get_view_property(int view_id, string prop)
        {
            var data = new Json.Object();
            data.set_int_member("id", view_id);
            data.set_string_member("property", prop);
            send_msg("window-rules/get-view-property", data);

            var r = new PendingReply();
            r.type      = VIEW_PROPERTY;
            r.view_id   = view_id;
            r.prop_name = prop;
            reply_queue.push_tail(r);
        }

        public void list_views()
        {
            send_msg("window-rules/list-views", null);

            var r = new PendingReply();
            r.type = ReplyType.LIST_VIEWS;
            reply_queue.push_tail(r);
        }
    }

    internal class BackendWayfire : Backend
    {
        private static string[] PROP_NAMES = {
            "kde-appmenu-service-name",
            "kde-appmenu-object-path",
            "gtk-shell-app-menu-path",
            "gtk-shell-application-object-path",
            "gtk-shell-menubar-path",
            "gtk-shell-unique-bus-name",
            "gtk-shell-window-object-path"
        };

        private class ViewProps
        {
            public string  title = "";
            public string  app_id = "";
            public int pid = 0;
            public string? kde_service_name     = null;
            public string? kde_object_path      = null;
            public string? gtk_unique_bus_name  = null;
            public string? gtk_app_menu_path    = null;
            public string? gtk_menubar_path     = null;
            public string? gtk_application_path = null;
            public string? gtk_window_path      = null;
        }

        private WayfireClient? wayfire = null;
        private ValaPanel.Matcher matcher;

        private int        active_view_id = -1;
        private ViewProps? active_props   = null;
        private int        pending_props  = 0;

        private int  menu_update_delay = 500;
        private uint delayed_menu_update_id = 0;

        public static BackendWayfire get_backend()
        {
            return new BackendWayfire();
        }
        public static bool is_applicable() {
            string? wf_socket = Environment.get_variable("WAYFIRE_SOCKET");
            if (!(wf_socket == null || wf_socket == ""))
                return true;
            return false;
        }
        construct
        {
            matcher = ValaPanel.Matcher.get();

            try {
                var wf = new WayfireClient();
                wf.view_focused  .connect(on_view_focused);
                wf.view_unmapped .connect(on_view_unmapped);
                wf.view_minimized .connect(on_view_minimized);
                wf.view_property .connect(on_view_property);
                wf.views_listed  .connect(on_views_listed);
                wayfire = wf;
            } catch (Error e) {
                critical("Wayfire backend init failed: %s", e.message);
            }
        }

        public override void update_helper()
        {
            var ctx = new GLib.AppLaunchContext();
            helper = null;

            if (type == ModelType.MENUMODEL && active_props != null)
                helper = get_menu_model_helper(ctx, active_props);
            else if (type == ModelType.DBUSMENU && active_props != null)
                helper = get_dbus_menu_helper(ctx, active_props);
            else if (type == ModelType.DESKTOP)
                helper = new DesktopHelper(ctx);
            else if (type == ModelType.STUB && active_props != null) {
                helper = get_stub_helper(ctx, active_props);
            }
            if(helper != null)
                helper.content_updated.connect(()=>{
                    active_model_changed();
                });
        }

        private void on_view_focused(int id, string title, string app_id, int pid)
        {
            if (id == active_view_id)
                return;

            reset_menu_update_timeout();

            active_view_id = id;
            active_props = new ViewProps();
            active_props.title  = title;
            active_props.app_id = app_id;
            active_props.pid = pid;
            pending_props = PROP_NAMES.length;

            if (wayfire == null)
                return;

            foreach (var p in PROP_NAMES)
                wayfire.get_view_property(id, p);
        }

        private void on_view_property(int id, string prop, string? value)
        {
            if (active_props == null || id != active_view_id)
                return;

            switch (prop) {
                case "kde-appmenu-service-name":           active_props.kde_service_name     = value; break;
                case "kde-appmenu-object-path":            active_props.kde_object_path      = value; break;
                case "gtk-shell-unique-bus-name":          active_props.gtk_unique_bus_name  = value; break;
                case "gtk-shell-app-menu-path":            active_props.gtk_app_menu_path    = value; break;
                case "gtk-shell-menubar-path":             active_props.gtk_menubar_path     = value; break;
                case "gtk-shell-application-object-path":  active_props.gtk_application_path = value; break;
                case "gtk-shell-window-object-path":       active_props.gtk_window_path      = value; break;
                default: return;
            }

            if (pending_props > 0)
                pending_props--;

            if (pending_props == 0) {
                lookup_menu(active_props);
                update_helper();
                active_model_changed();
            }
        }

        private DesktopAppInfo? resolve_app(ViewProps props)
        {
            if (props == null || props.app_id == "")
                return null;
            return matcher.match_arbitrary(props.title,
                                        props.app_id,
                                        props.app_id,
                                        props.pid);
        }
        private string get_title(DesktopAppInfo? info, ViewProps props) {
            if (info != null)
                return info.get_name();
            if (props.app_id != null)
                return props.app_id;
            return props.title;
        }
        private void on_view_unmapped(int id)
        {
            if (id != active_view_id)
                return;

            active_view_id = -1;
            active_props   = null;
            pending_props  = 0;
            type           = ModelType.NONE;

            reset_menu_update_timeout();
            delayed_menu_update_id = Timeout.add((uint) menu_update_delay, () => {
                delayed_menu_update_id = 0;
                if (wayfire != null) {
                    wayfire.list_views();
                } else {
                    update_helper();
                }
                return false;
            });
        }

        private void on_view_minimized(int id, bool minimized)
        {
            if (id != active_view_id || !minimized)
                return;

            active_view_id = -1;
            active_props   = null;
            pending_props  = 0;
            type           = ModelType.NONE;

            reset_menu_update_timeout();
            delayed_menu_update_id = Timeout.add((uint) menu_update_delay, () => {
                delayed_menu_update_id = 0;
                if (wayfire != null) {
                    wayfire.list_views();
                } else {
                    type = ModelType.DESKTOP;
                    update_helper();
                }
                return false;
            });
        }

        private void on_views_listed(Json.Array views)
        {
            if (active_view_id != -1)
                return;

            bool has_toplevel = false;
            views.foreach_element((arr, index, node) => {
                var view = node.get_object();
                string role = view.has_member("role") ? view.get_string_member("role") : "";
                bool mapped = view.has_member("mapped") ? view.get_boolean_member("mapped") : false;
                bool minimized = view.has_member("minimized") ? view.get_boolean_member("minimized") : false;

                // Exclude layer-shell ("desktop-environment"), unmapped, and minimized windows
                if (role == "toplevel" && mapped && !minimized) {
                    has_toplevel = true;
                }
            });

            type = has_toplevel ? ModelType.NONE : ModelType.DESKTOP;
            update_helper();
            active_model_changed();
        }

        private void reset_menu_update_timeout()
        {
            if (delayed_menu_update_id > 0) {
                Source.remove(delayed_menu_update_id);
                delayed_menu_update_id = 0;
            }
        }

        private void lookup_menu(ViewProps props)
        {
            if (props.kde_service_name != null && props.kde_object_path != null) {
                type = ModelType.DBUSMENU;
                return;
            }

            if (props.gtk_unique_bus_name != null &&
               (props.gtk_menubar_path != null || props.gtk_app_menu_path != null)) {
                type = ModelType.MENUMODEL;
                return;
            }

            type = ModelType.STUB;
        }

        private MenuModelHelper get_menu_model_helper(GLib.AppLaunchContext ctx, ViewProps props)
        {
            DesktopAppInfo? app = resolve_app(props);
            string title = get_title(app, props);
            return new MenuModelHelper(ctx,
                props.gtk_unique_bus_name,
                props.gtk_app_menu_path,
                props.gtk_menubar_path,
                props.gtk_application_path,
                props.gtk_window_path,
                props.gtk_menubar_path,
                title,
                app);
        }

        private DBusMenuHelper get_dbus_menu_helper(GLib.AppLaunchContext ctx, ViewProps props)
        {
            DesktopAppInfo? app = resolve_app(props);
            string title = get_title(app, props);
            return new DBusMenuHelper(ctx,
                props.kde_service_name,
                (ObjectPath) props.kde_object_path,
                title,
                app);
        }

        private DBusAppMenu get_stub_helper(GLib.AppLaunchContext ctx, ViewProps props)
        {
            DesktopAppInfo? app = resolve_app(props);
            string title = get_title(app, props);
            return new DBusAppMenu(ctx, title, props.pid, app);
        }
    }
}
