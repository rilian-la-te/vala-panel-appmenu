/*
 * Copyright (C) 2012  Nicolas Bruguier
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *  Nicolas Bruguier <gandalfn@club-internet.fr>
 */

[CCode (cheader_filename = "xcb/xcb.h")]
namespace Xcb
{
    /**
     * Current protocol version
     */
    [CCode (cname = "X_PROTOCOL")]
    public const int PROTOCOL;

    /**
     * Current minor version
     */
    [CCode (cname = "X_PROTOCOL_REVISION")]
    public const int PROTOCOL_REVISION;

    /**
     * X_TCP_PORT + display number = server port for TCP transport
     */
    [CCode (cname = "X_TCP_PORT")]
    public const int TCP_PORT;

    /**
     * Generic iterator.
     *
     * A generic iterator structure.
     */
    [Compact, CCode (cname = "xcb_generic_iterator_t", free_function = "free")]
    public class GenericIterator
    {
        /**
         * Data of the current iterator
         */
        public void* data;
        /**
         * Remaining elements
         */
        public int rem;
        /**
         * Index of the current iterator
         */
        public int index;
    }

    /**
     * Generic reply.
     *
     * A generic reply structure.
     */
    [Compact, CCode (cname = "xcb_generic_reply_t", free_function = "free")]
    public class GenericReply
    {
        /**
         * Type of the response
         */
        public uint8 response_type;
        /**
         * Sequence number
         */
        public uint16 sequence;
        /**
         * Length of the response
         */
        public uint32 length;
    }

    /**
     * Generic error.
     *
     * A generic error class.
     */
    [Compact, CCode (cname = "xcb_generic_error_t", free_function = "free")]
    public class GenericError
    {
        /**
         * Type of the response
         */
        public uint8   response_type;
        /**
         * Error code
         */
        public uint8   error_code;
        /**
         * Sequence number
         */
        public uint16  sequence;
        /**
         * Resource ID for requests with side effects only
         */
        public uint32  resource_id;
        /**
         * Minor opcode of the failed request
         */
        public uint16  minor_code;
        /**
         * Major opcode of the failed request
         */
        public uint8   major_code;
        /**
         * full sequence
         */
        public uint32  full_sequence;
    }

    /**
     * Generic event
     *
     * A generic event class.
     */
    [Compact, CCode (cname = "xcb_generic_event_t", free_function = "free")]
    public class GenericEvent
    {
        /**
         * Type of the response
         */
        public uint8  response_type;
        /**
         * Sequence number
         */
        public uint16 sequence;
        /**
         * full sequence
         */
        public uint32 full_sequence;
    }

    /**
     * GE event
     *
     * An event as sent by the XGE extension. The length field specifies the
     * number of 4-byte blocks trailing the struct.
     */
    [Compact, CCode (cname = "xcb_ge_event_t", free_function = "free")]
    public class GEEvent
    {
        /**
         * Type of the response
         */
        public uint8  response_type;
        /**
         * Sequence number
         */
        public uint16 sequence;
        public uint32 length;
        public uint16 event_type;
        /**
         * full sequence
         */
        public uint32 full_sequence;
    }

    /**
     * Generic cookie.
     *
     * A generic cookie structure.
     */
    [SimpleType, CCode (cname = "xcb_void_cookie_t")]
    public struct VoidCookie
    {
        /**
         * Sequence number
         */
        public uint sequence;
    }

    /**
     * NONE is the universal null resource or null atom parameter value for
     * many core X requests
     */
    [CCode (cname = "XCB_NONE")]
    public const uint8 NONE;

    /**
     * COPY_FROM_PARENT can be used for many xcb_create_window parameters
     */
    [CCode (cname = "XCB_COPY_FROM_PARENT")]
    public const uint8 COPY_FROM_PARENT;

    /**
     * CURRENT_TIME can be used in most requests that take an Timestamp
     */
    [CCode (cname = "XCB_CURRENT_TIME")]
    public const uint8 CURRENT_TIME;

    /**
     * NO_SYMBOL fills in unused entries in Keysym tables
     */
    [CCode (cname = "XCB_NO_SYMBOL")]
    public const uint8 NO_SYMBOL;

    /**
     * Container for authorization information.
     *
     * A container for authorization information to be sent to the X server.
     */
    [CCode (cname = "xcb_auth_info_t")]
    public struct AuthInfo
    {
        [CCode (array_length_cname = "namelen")]
        public char[] name;
        [CCode (array_length_cname = "datalen")]
        public char[] data;
    }

    /**
     * XCB Connection
     *
     * A class that contain all data that  XCB needs to communicate with an X server.
     */
    [Compact, CCode (cname = "xcb_connection_t", free_function = "xcb_disconnect")]
    public class BaseConnection
    {
        [CCode (cname = "int")]
        public enum Status
        {
            [CCode (cname = "0")]
            OK,
            /**
             * connection errors because of socket, pipe and other stream errors.
             */
            [CCode (cname = "XCB_CONN_ERROR")]
            ERROR,
            /**
             * connection shutdown because of extension not sppported
             */
            [CCode (cname = "XCB_CONN_CLOSED_EXT_NOTSUPPORTED")]
            CLOSED_EXT_NOTSUPPORTED,
            /**
             * malloc(), calloc() and realloc() error upon failure, for eg ENOMEM
             */
            [CCode (cname = "XCB_CONN_CLOSED_MEM_INSUFFICIENT")]
            CLOSED_MEM_INSUFFICIENT,
            /**
             * Connection closed, exceeding request length that server accepts.
             */
            [CCode (cname = "XCB_CONN_CLOSED_REQ_LEN_EXCEED")]
            CLOSED_REQ_LEN_EXCEED,
            /**
             * Connection closed, error during parsing display string.
             */
            [CCode (cname = "XCB_CONN_CLOSED_PARSE_ERR")]
            CLOSED_PARSE_ERR
        }

        /**
         * Test whether the connection has shut down due to a fatal error.
         *
         * Some errors that occur in the context of an Connection
         * are unrecoverable. When such an error occurs, the
         * connection is shut down and further operations on the
         * Connection have no effect.
         */
        public Status status {
            [CCode (cname = "xcb_connection_has_error")]
            get;
        }

        /**
         * Access the screen list returned by the server.
         *
         * Accessor for the data returned by the server when the Connection
         * was initialized. This data includes the list of available screens;
         */
        public unowned Roots? roots {
            [CCode (cname = "xcb_get_setup")]
            get;
        }

        /**
         * Access the data returned by the server.
         *
         * Accessor for the data returned by the server when the Connection
         * was initialized. This data includes
         * - the server's required format for images,
         * - the server's maximum request length (in the absence of the
         * BIG-REQUESTS extension),
         * - and other assorted information.
         *
         * See the X protocol specification for more details.
         */
        public unowned Setup? setup {
            [CCode (cname = "xcb_get_setup")]
            get;
        }

        /**
         * Access the file descriptor of the connection.
         *
         * Accessor for the file descriptor that was passed to the
         * connect_to_fd call that returned Connection.
         */
        public int file_descriptor {
            [CCode (cname = "xcb_get_file_descriptor")]
            get;
        }

        /**
         * The maximum request length that this server accepts.
         *
         * In the absence of the BIG-REQUESTS extension, returns the
         * maximum request length field from the connection setup data, which
         * may be as much as 65535. If the server supports BIG-REQUESTS, then
         * the maximum request length field from the reply to the
         * BigRequestsEnable request will be returned instead.
         *
         * Note that this length is measured in four-byte units, making the
         * theoretical maximum lengths roughly 256kB without BIG-REQUESTS and
         * 16GB with.
         */
        public uint32 maximum_request_length {
            [CCode (cname = "xcb_get_maximum_request_length")]
            get;
        }

        /**
         * Connects to the X server.
         *
         * Connects to the X server specified by displayname. If
         * displayname is ``null``, uses the value of the DISPLAY environment
         * variable. If a particular screen on that server is preferred, the
         * int screen (if not ``null``) will be set to that screen; otherwise
         * the screen will be set to 0.
         *
         * @param displayname The name of the display.
         * @param screen A preferred screen number.
         *
         * @return A new Connection.
         */
        [CCode (cname = "xcb_connect")]
        public BaseConnection (string? displayname = null, out int screen = null);

        /**
         * Connects to the X server, using an authorization information.
         *
         * Connects to the X server specified by displayname, using the
         * authorization auth. If a particular screen on that server is
         * preferred, the int pointed to by screen will be set to that screen;
         * otherwise screen will be set to 0.
         *
         * @param display The name of the display.
         * @param auth The authorization information.
         * @param screen A pointer to a preferred screen number.
         *
         * @return A newly allocated xcb_connection_t structure.
         */
        [CCode (cname = "xcb_connect_to_display_with_auth_info")]
        public BaseConnection.with_auth_info (string? display, AuthInfo? auth, out int screen = null);

        /**
         * Connects to the X server.
         *
         * Connects to an X server, given the open socket fd and the
         * AuthInfo auth_info. The file descriptor fd is
         * bidirectionally connected to an X server. If the connection
         * should be unauthenticated, auth_info must be ``null``.

         * @param fd The file descriptor.
         * @param auth_info Authentication data.
         *
         * @return A new Connection.
         */
        [CCode (cname = "xcb_connect_to_fd")]
        public BaseConnection.to_fd (int fd, AuthInfo? auth_info);

        /**
         * Forces any buffered output to be written to the server.
         *
         * Forces any buffered output to be written to the server. Blocks
         * until the write is complete.
         *
         * @return > 0 on success, <= 0 otherwise.
         */
        [CCode (cname = "xcb_flush")]
        public int flush ();

        /**
         * Caches reply information from QueryExtension requests.
         *
         * This function is the primary interface to the "extension cache",
         * which caches reply information from QueryExtension
         * requests. Invoking this function may cause a call to
         * xcb_query_extension to retrieve extension information from the
         * server, and may block until extension data is received from the
         * server.
         *
         * @param ext The extension data.
         *
         * @return The QueryExtensionReply for the extension.
         */
        [CCode (cname = "xcb_get_extension_data")]
        public unowned Xcb.QueryExtensionReply? get_extension_data(ref Extension ext);

        /**
         * Prefetch of extension data into the extension cache
         *
         * This function allows a "prefetch" of extension data into the
         * extension cache. Invoking the function may cause a call to
         * xcb_query_extension, but will not block waiting for the
         * reply. xcb_get_extension_data will return the prefetched data after
         * possibly blocking while it is retrieved.
         *
         * @param ext The extension data.
         */
        [CCode (cname = "xcb_prefetch_extension_data")]
        public void prefetch_extension_data(ref Extension ext);

        /**
         * Prefetch the maximum request length without blocking.
         *
         * Without blocking, does as much work as possible toward computing
         * the maximum request length accepted by the X server.
         *
         * Invoking this function may cause a call to xcb_big_requests_enable,
         * but will not block waiting for the reply.
         * maximum_request_length will return the prefetched data
         * after possibly blocking while the reply is retrieved.
         *
         * Note that in order for this function to be fully non-blocking, the
         * application must previously have called
         * prefetch_extension_data(xcb_big_requests_id) and the reply
         * must have already arrived.
         */
        [CCode (cname = "xcb_prefetch_maximum_request_length")]
        public void prefetch_maximum_request_length ();

        /**
         * Returns the next event or error from the server.
         *
         * Returns the next event or error from the server, or returns null in
         * the event of an I/O error. Blocks until either an event or error
         * arrive, or an I/O error occurs.
         *
         * @return The next event from the server.
         */
        [CCode (cname = "xcb_wait_for_event")]
        public GenericEvent? wait_for_event();


        /**
         * Returns the next event or error from the server.
         *
         * Returns the next event or error from the server, if one is
         * available, or returns ``null`` otherwise. If no event is available, that
         * might be because an I/O error like connection close occurred while
         * attempting to read the next event, in which case the connection is
         * shut down when this function returns.
         *
         * @return The next event from the server.
         */
        [CCode (cname = "xcb_poll_for_event")]
        public GenericEvent? poll_for_event();


        /**
         * Returns the next event without reading from the connection.
         *
         * This is a version of poll_for_event that only examines the
         * event queue for new events. The function doesn't try to read new
         * events from the connection if no queued events are found.
         *
         * This function is useful for callers that know in advance that all
         * interesting events have already been read from the connection. For
         * example, callers might use wait_for_reply and be interested
         * only of events that preceded a specific reply.
         *
         * @return The next already queued event from the server.
         */
        [CCode (cname = "xcb_poll_for_queued_event")]
        public GenericEvent? poll_for_queued_event();

        /**
         * Return the error for a request, or ``null`` if none can ever arrive.
         *
         * The xcb_void_cookie_t cookie supplied to this function must have resulted
         * from a call to xcb_[request_name]_checked().  This function will block
         * until one of two conditions happens.  If an error is received, it will be
         * returned.  If a reply to a subsequent request has already arrived, no error
         * can arrive for this request, so this function will return ``null``.
         *
         * Note that this function will perform a sync if needed to ensure that the
         * sequence number will advance beyond that provided in cookie; this is a
         * convenience to avoid races in determining whether the sync is needed.
         *
         * @param cookie The request cookie.
         *
         * @return The error for the request, or ``null`` if none can ever arrive.
         */
        [CCode (cname = "xcb_request_check")]
        public GenericError? request_check(VoidCookie cookie);

        /**
         * Discards the reply for a request.
         *
         * Discards the reply for a request. Additionally, any error generated
         * by the request is also discarded (unless it was an _unchecked request
         * and the error has already arrived).
         *
         * This function will not block even if the reply is not yet available.
         *
         * Note that the sequence really does have to come from an xcb cookie;
         * this function is not designed to operate on socket-handoff replies.
         *
         * @param sequence The request sequence number from a cookie.
         */
        [CCode (cname = "xcb_discard_reply")]
        public void discard_reply(uint sequence);

        [CCode (cname = "xcb_send_request", cheader_filename = "xcb/xcbext.h")]
        public uint send_request(int flags, void* vector, ProtocolRequest request);

        public delegate void ReturnSocketFunc ();
        /**
         * Allows external code to ask XCB for permission to
         * take over the write side of the socket and send raw data with
         * writev. take_socket provides the sequence number of the last
         * request XCB sent. The caller of xcb_take_socket must supply a
         * callback which XCB can call when it wants the write side of the
         * socket back to make a request. This callback synchronizes with the
         * external socket owner and flushes any output queues if appropriate.
         * If you are sending requests which won't cause a reply, please note the
         * comment for writev which explains some sequence number wrap issues.
         */
        [CCode (cname = "xcb_take_socket", cheader_filename = "xcb/xcbext.h")]
        public int take_socket(ReturnSocketFunc return_socket, int flags, out uint64 sent);

        /**
         * You must own the write-side of the socket (you've called
         * take_socket, and haven't returned from return_socket yet) to call
         * writev. Also, the iovec must have at least 1 byte of data in it.
         * You have to make sure that xcb can detect sequence number wraps correctly.
         * This means that the first request you send after take_socket must cause a
         * reply (e.g. just insert a GetInputFocus request). After every (1 << 16) - 1
         * requests without a reply, you have to insert a request which will cause a
         * reply. You can again use GetInputFocus for this. You do not have to wait for
         * any of the GetInputFocus replies, but can instead handle them via
         * discard_reply().
         */
        [CCode (cname = "xcb_writev", cheader_filename = "xcb/xcbext.h")]
        public int writev(ref Posix.iovector vector, int count, uint64 requests);

        [CCode (cname = "xcb_wait_for_reply", cheader_filename = "xcb/xcbext.h")]
        public GenericReply wait_for_reply (uint request, out GenericError error);

        [CCode (cname = "xcb_poll_for_reply", cheader_filename = "xcb/xcbext.h")]
        public int poll_for_reply (uint request, out GenericReply reply, out GenericError error);
    }

    /**
     * List of screens class
     */
    [Compact, Immutable, CCode (cname = "xcb_setup_t")]
    public class Roots
    {
        [SimpleType, CCode (cname = "xcb_screen_iterator_t")]
        struct _Iterator
        {
            int rem;
            int index;
            unowned Screen? data;
        }

        /**
         * An iterator over list of screen
         */
        [CCode (cname = "xcb_screen_iterator_t")]
        public struct Iterator
        {
            [CCode (cname = "xcb_screen_next")]
            void _next ();

            /**
             * Gets the next screen in the screen list.
             */
            public inline unowned Screen?
            next_value ()
            {
                if (((_Iterator)this).rem > 0)
                {
                    unowned Screen d = ((_Iterator)this).data;
                    _next ();
                    return d;
                }

                return null;
            }
        }

        /**
         * The number of screen in list
         */
        public int length {
            [CCode (cname = "xcb_setup_roots_length")]
            get;
        }

        /**
         * Return iterator that can be used for simple iteration over screen list.
         *
         * @return iterator that can be used for simple iteration over screen list.
         */
        [CCode (cname = "xcb_setup_roots_iterator")]
        unowned _Iterator _iterator ();
        public Iterator iterator ()
        {
            return (Iterator)_iterator ();
        }

        /**
         * Returns the screen corresponding to screen_num
         *
         * @param screen_num screen number
         *
         * @return the screen corresponding to screen_num
         */
        public unowned Screen?
        get (int screen_num)
            requires (screen_num < length)
        {
            int index = 0;
            foreach (unowned Screen? screen in this)
            {
                if (index == screen_num)
                    return screen;
                index++;
            }

            return null;
        }
    }

    [CCode (cname = "xcb_extension_t", has_destroy_function = false, cheader_filename = "xcb/xcbext.h")]
    public struct Extension
    {
        public string name;
        public int global_id;
    }

    [CCode (cname = "xcb_protocol_request_t", cheader_filename = "xcb/xcbext.h")]
    public struct ProtocolRequest
    {
        public size_t count;
        public unowned Extension? ext;
        public uint8 opcode;
        public uint8 isvoid;
    }

    [Flags, CCode (cname = "xcb_send_request_flags_t", cprefix =  "XCB_REQUEST_", cheader_filename = "xcb/xcbext.h")]
    public enum RequestFlags
    {
        [CCode (cname = "0")]
        NONE,
        CHECKED,
        RAW,
        DISCARD_REPLY
    }

    [CCode (cname = "xcb_popcount", cheader_filename = "xcb/xcbext.h")]
    public static int popcount (uint32 mask);

    [CCode (cname = "xcb_sumof", cheader_filename = "xcb/xcbext.h")]
    public static int sumof(uint8[] list);
}
