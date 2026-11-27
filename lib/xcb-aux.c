// SPDX-License-Identifier: LGPL-3.0-or-later

/*
 * An EWMH over XCB replacement for libwnck subset. It keeps only the window manager
 * state the appmenu backend needs - an ordered client list and the active
 * window - by watching the root window properties an EWMH compliant window
 * manager maintains, and exposes the handful of wnck accessors the backend
 * calls (window type, pid, WM_CLASS pair, transient for, parent).
 *
 * Reading X replies without losing events is the delicate part. xcb offers no
 * way to ask whether a request that produces a reply has been answered:
 * xcb_request_check() aborts when it runs into one. So we never let a reply sit
 * in the socket unread. Every query uses the checked request form and its reply
 * is taken with xcb_get_property_reply(). A burst of queries can still be sent
 * before any reply is read, because libxcb keeps track of them itself, so a full
 * window scan costs a single round trip while events that arrive meanwhile stay
 * queued for us instead of being swallowed by a reply reader.
 */

#include "xcb-aux.h"

#include "matcher.h"

#include <glib-unix.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>

typedef enum {
	WINDOW_TYPE_NORMAL,
	WINDOW_TYPE_DIALOG,
	WINDOW_TYPE_MENU,
	WINDOW_TYPE_TOOLBAR,
	WINDOW_TYPE_SPLASHSCREEN,
	WINDOW_TYPE_UTILITY,
	WINDOW_TYPE_DOCK,
	WINDOW_TYPE_DESKTOP,
	WINDOW_TYPE_DROPDOWN_MENU,
	WINDOW_TYPE_POPUP_MENU,
	WINDOW_TYPE_TOOLTIP,
	WINDOW_TYPE_NOTIFICATION,
	WINDOW_TYPE_COMBO,
	WINDOW_TYPE_DND,
	WINDOW_TYPE_UNKNOWN
} WindowType;

#define VPA_WINDOW_NONE ((uint32_t)0)

/* The signals a Backend instance owns. Declaring them on the shared helper lets
 * every process that opens the connection connect its own handlers and hear
 * about the same window manager activity. */
#define WM_WINDOW_OPENED         "wm-window-opened"
#define WM_ACTIVE_WINDOW_CHANGED "wm-active-window-changed"
#define WM_WINDOW_CLOSED         "wm-window-closed"

/* How many requests a batch sends before reading the replies back. Comfortably
 * below any socket buffer, so a scan cannot stall on an unwritable socket. */
#define QUERY_BATCH 48

/* The window manager signals are declared on the listener's own type, because
 * GObject only lets an instance hear signals its type owns. Declaring them once
 * per type is enough: every Backend (and anything else that registers itself as
 * a listener) receives them through g_signal_emit(). */
static GQuark signal_opened;
static GQuark signal_closed;
static GQuark signal_activity;

static void ensure_signals(GType type)
{
	const struct {
		GQuark *cache;
		const char *name;
		uint n_args;
		GType arg;
	} signals[] = {
		{&signal_opened, WM_WINDOW_OPENED, 1, G_TYPE_UINT},
		{&signal_closed, WM_WINDOW_CLOSED, 1, G_TYPE_UINT},
		{&signal_activity, WM_ACTIVE_WINDOW_CHANGED, 0, G_TYPE_NONE},
	};

	for (guint i = 0; i < G_N_ELEMENTS(signals); i++) {
		if (*signals[i].cache != 0)
			continue;
		/* g_signal_lookup() also finds signals the listener inherited from one of
		 * its ancestors, which are exactly the ones it can hear. A signal that is
		 * only defined on some unrelated type would never reach us, so it is left
		 * alone and the helper simply stays quiet about that kind of event. */
		GQuark known = g_signal_lookup(signals[i].name, type);
		if (known != 0) {
			*signals[i].cache = known;
			continue;
		}
		*signals[i].cache = g_signal_new(signals[i].name, type, G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
		                                  G_TYPE_NONE, signals[i].n_args, signals[i].arg);
	}
}

/* The main loop source that feeds connection events to a helper. It sits next to the
 * helper rather than inside it because GLib owns the beginning of a GSource. Declared
 * before XcbAux so that the helper can hold a pointer to it. */
typedef struct {
	GSource source;   /* must stay first: GLib reads its members from the cast pointer */
	XcbAux *aux;
	GPollFD poll_fd;
} XcbWatch;

struct XcbAux {
	xcb_connection_t *conn;
	xcb_ewmh_connection_t ewmh;
	int screen;
	xcb_window_t root;
	gboolean has_client_list;
	guint generation; /* counts client list scans, to spot windows that fell out */

	GHashTable *windows; /* xcb_window_t -> window_state */
	GList *order;        /* client list, most recently activated first */
	GList *listeners;    /* GObject instances that hear the window manager signals */

	xcb_window_t wm_window;   /* the window that claims to be the window manager */
	xcb_atom_t net_wm_type;   /* _NET_WM_WINDOW_TYPE */
	xcb_atom_t transient_for; /* _NET_WM_TRANSIENT_FOR */
	xcb_atom_t wm_pid;        /* _NET_WM_PID */

	xcb_window_t active; /* last active window we know of */
	xcb_window_t active_reported; /* last focus we announced, 0 until we have one */
	XcbWatch *watch;   /* the main loop source watching the xcb fd, NULL if not watching */
	guint refs;
	gboolean lost;
};

typedef struct {
	gboolean known; /* the window exists as far as the server is concerned */
	gboolean present; /* reported to the listeners and not yet reported closed */
	guint generation;  /* last client list scan that named this window */
	xcb_window_t transient_for;
	WindowType type;
} WindowState;

static XcbAux *shared = NULL;

/* --------------------------------------------------------------- properties */

static xcb_atom_t intern_name(xcb_connection_t *conn, const char *name)
{
	xcb_intern_atom_cookie_t cookie =
	    xcb_intern_atom(conn, 0, (uint16_t) strlen(name), name);
	xcb_generic_error_t *error     = NULL;
	xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, &error);
	xcb_atom_t atom                = 0;
	if (reply != NULL) {
		atom = reply->atom;
		free(reply);
	}
	g_clear_pointer(&error, free);
	return atom;
}

/* Checked GetProperty. Its reply must be taken with aux_reply() before any
 * event is read, otherwise the request would still be unanswered and reading an
 * event would block forever waiting for it. */
static xcb_get_property_cookie_t ask_prop(xcb_connection_t *conn, xcb_window_t window,
                                          xcb_atom_t property, uint32_t long_length)
{
	return xcb_get_property(conn, 0, window, property, XCB_ATOM_ANY, 0, long_length);
}

static xcb_get_property_reply_t *aux_reply(xcb_connection_t *conn, xcb_get_property_cookie_t cookie)
{
	xcb_generic_error_t *error      = NULL;
	xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, cookie, &error);
	g_clear_pointer(&error, free);
	return reply;
}

/* Resolves an atom to its name. The caller frees the result with g_free(). Like
 * every other query here this is a checked request, so its reply has to be read
 * before any event is looked at. */
static char *atom_name(xcb_connection_t *conn, xcb_atom_t atom)
{
	xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(conn, atom);
	xcb_generic_error_t *error        = NULL;
	xcb_get_atom_name_reply_t *reply  = xcb_get_atom_name_reply(conn, cookie, &error);
	char *name                        = NULL;
	if (reply != NULL) {
		name = g_strndup(xcb_get_atom_name_name(reply), xcb_get_atom_name_name_length(reply));
		free(reply);
	}
	g_clear_pointer(&error, free);
	return name;
}

static uint32_t prop_card32(const xcb_get_property_reply_t *reply)
{
	if (reply == NULL || reply->format != 32 ||
	    xcb_get_property_value_length(reply) < (int) sizeof(uint32_t))
		return 0;
	return *(const uint32_t *) xcb_get_property_value(reply);
}

/* Only text properties may be handed to GTK as strings. */
static gboolean prop_is_text(xcb_connection_t *conn, const xcb_get_property_reply_t *reply)
{
	if (reply->format != 8 || reply->type == XCB_NONE)
		return FALSE;
	static const char *const names[] = {"STRING", "UTF8_STRING", "COMPOUND_TEXT", NULL};
	g_autofree char *type            = atom_name(conn, reply->type);
	if (type == NULL)
		return FALSE;
	return g_strv_contains(names, type);
}

char *vala_panel_xcb_aux_get_utf8_prop(void *connection, uint32_t window, const char *prop)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self || window == VPA_WINDOW_NONE || !prop || prop[0] == '\0')
		return NULL;

	xcb_atom_t atom = intern_name(self->conn, prop);
	if (atom == 0)
		return NULL;

	xcb_get_property_reply_t *reply = aux_reply(self->conn, ask_prop(self->conn, window, atom, 0x4000));
	char *value                     = NULL;
	if (reply != NULL && prop_is_text(self->conn, reply) && xcb_get_property_value_length(reply) > 0) {
		const char *raw = xcb_get_property_value(reply);
		if (raw[0] != '\0')
			value = g_strdup(raw);
	}
	g_clear_pointer(&reply, free);
	return value;
}

static uint32_t read_card32(XcbAux *self, xcb_window_t window, xcb_atom_t atom)
{
	if (atom == 0)
		return 0;
	xcb_get_property_reply_t *reply = aux_reply(self->conn, ask_prop(self->conn, window, atom, 1));
	uint32_t value                  = prop_card32(reply);
	g_clear_pointer(&reply, free);
	return value;
}

/* -------------------------------------------------------------------- state */

static WindowState *peek_state(XcbAux *self, xcb_window_t window)
{
	return g_hash_table_lookup(self->windows, GUINT_TO_POINTER(window));
}

/* Selects the events a tracked window has to deliver itself. A child's unmap and
 * destroy are leave notifications sent to its parent, so they cannot be heard from
 * the root selection alone; property change is selected here as well because an
 * unmapped window is reported through it too. AddToSaveSet stops the server from
 * destroying the window when our connection goes away, which would otherwise make
 * a perfectly normal shutdown look like the application closed its window. */
static void watch_window(XcbAux *self, xcb_window_t window)
{
	if (window == XCB_NONE || window == self->root || window == self->wm_window)
		return;
	uint32_t events[] = {XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_STRUCTURE_NOTIFY };
	xcb_change_window_attributes(self->conn, window, XCB_CW_EVENT_MASK, events);
	xcb_change_save_set(self->conn, XCB_SET_MODE_INSERT, window);
}

static WindowState *need_state(XcbAux *self, xcb_window_t window)
{
	WindowState *state = peek_state(self, window);
	if (state == NULL) {
		/* A window entering the cache is assumed mapped; an unmap or a missing
		 * client list entry marks it gone afterwards. */
		state        = g_new0(WindowState, 1);
		state->known = TRUE;
		state->type  = WINDOW_TYPE_UNKNOWN;
		g_hash_table_insert(self->windows, GUINT_TO_POINTER(window), state);
		watch_window(self, window);
	}
	return state;
}

static void order_add(XcbAux *self, xcb_window_t window)
{
	/* The list holds GUINT_TO_POINTER wrapped xids. A bare xcb_window_t must never
	 * be stored instead: window ids are small integers, so a raw pointer would
	 * collide with another id and g_list_free() would try to free it. */
	gpointer data = GUINT_TO_POINTER(window);
	GList *link   = g_list_find(self->order, data);
	if (link != NULL) {
		if (link == self->order)
			return;
		self->order = g_list_delete_link(self->order, link);
	}
	self->order = g_list_prepend(self->order, data);
}

static void order_remove(XcbAux *self, xcb_window_t window)
{
	GList *link = g_list_find(self->order, GUINT_TO_POINTER(window));
	if (link != NULL)
		self->order = g_list_delete_link(self->order, link);
}

/* Drops windows that are neither listed nor waiting to be reported. */
/* ---------------------------------------------------------------- state */
/* A window manager that reparents its clients puts a frame window between the
 * root and the client. Client list entries already name clients, but the active
 * window property and map/unmap events speak about frames, so those paths have
 * to look one level down. */
static xcb_window_t frame_client(XcbAux *self, xcb_window_t window)
{
	xcb_query_tree_cookie_t cookie = xcb_query_tree(self->conn, window);
	xcb_generic_error_t *error     = NULL;
	xcb_query_tree_reply_t *tree   = xcb_query_tree_reply(self->conn, cookie, &error);
	xcb_window_t client            = XCB_NONE;
	if (tree != NULL) {
		if (xcb_query_tree_children_length(tree) == 1 && tree->parent == self->root)
			client = *xcb_query_tree_children(tree);
		free(tree);
	}
	g_clear_pointer(&error, free);
	return client;
}

static gboolean is_wm_child(XcbAux *self, xcb_window_t window)
{
	xcb_query_tree_cookie_t cookie = xcb_query_tree(self->conn, window);
	xcb_generic_error_t *error     = NULL;
	xcb_query_tree_reply_t *tree   = xcb_query_tree_reply(self->conn, cookie, &error);
	gboolean child                 = FALSE;
	if (tree != NULL) {
		child = tree->parent == self->wm_window;
		free(tree);
	}
	g_clear_pointer(&error, free);
	return child;
}

/* _NET_SUPPORTING_WM_CHECK names the window manager's own window. It counts only
 * when that window repeats the check about itself, which keeps ordinary clients
 * out of the desktop role. */
static void find_wm_window(XcbAux *self)
{
    xcb_window_t wm = read_card32(self, self->root, self->ewmh._NET_SUPPORTING_WM_CHECK);
    self->wm_window = (wm != XCB_NONE &&
                       read_card32(self, wm, self->ewmh._NET_SUPPORTING_WM_CHECK) == wm)
                          ? wm : XCB_NONE;
}

/* ---------------------------------------------------------------- accessors */

/* Maps an _NET_WM_WINDOW_TYPE atom name onto the wnck style enumerator. */
static WindowType window_type_from_name(const char *name)
{
	static const struct {
		const char *atom;
		WindowType type;
	} known[] = {
		{"_NET_WM_WINDOW_TYPE_NORMAL", WINDOW_TYPE_NORMAL},
		{"_NET_WM_WINDOW_TYPE_DIALOG", WINDOW_TYPE_DIALOG},
		{"_NET_WM_WINDOW_TYPE_MENU", WINDOW_TYPE_MENU},
		{"_NET_WM_WINDOW_TYPE_TOOLBAR", WINDOW_TYPE_TOOLBAR},
		{"_NET_WM_WINDOW_TYPE_SPLASHSCREEN", WINDOW_TYPE_SPLASHSCREEN},
		{"_NET_WM_WINDOW_TYPE_UTILITY", WINDOW_TYPE_UTILITY},
		{"_NET_WM_WINDOW_TYPE_DOCK", WINDOW_TYPE_DOCK},
		{"_NET_WM_WINDOW_TYPE_DESKTOP", WINDOW_TYPE_DESKTOP},
		{"_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", WINDOW_TYPE_DROPDOWN_MENU},
		{"_NET_WM_WINDOW_TYPE_POPUP_MENU", WINDOW_TYPE_POPUP_MENU},
		{"_NET_WM_WINDOW_TYPE_TOOLTIP", WINDOW_TYPE_TOOLTIP},
		{"_NET_WM_WINDOW_TYPE_NOTIFICATION", WINDOW_TYPE_NOTIFICATION},
		{"_NET_WM_WINDOW_TYPE_COMBO", WINDOW_TYPE_COMBO},
		{"_NET_WM_WINDOW_TYPE_DND", WINDOW_TYPE_DND},
	};
	if (name == NULL)
		return WINDOW_TYPE_UNKNOWN;
	for (guint i = 0; i < G_N_ELEMENTS(known); i++)
		if (g_strcmp0(name, known[i].atom) == 0)
			return known[i].type;
	return WINDOW_TYPE_UNKNOWN;
}

/* The type of a window as the backend expects to see it. wnck calls a window it
 * cannot classify NORMAL, which is what lets the backend tell an ordinary
 * client apart from a window that has simply gone away, so an unlisted or
 * unknown window is reported that way. */
static WindowType vala_panel_xcb_aux_get_window_type(void *connection, uint32_t window)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self || window == VPA_WINDOW_NONE)
		return WINDOW_TYPE_NORMAL;

	WindowState *state = peek_state(self, window);
	WindowType type = state != NULL ? state->type : WINDOW_TYPE_UNKNOWN;
	if (type == WINDOW_TYPE_UNKNOWN && self->net_wm_type != 0) {
		xcb_get_property_cookie_t cookie = ask_prop(self->conn, window, self->net_wm_type, 1);
		xcb_get_property_reply_t *reply  = aux_reply(self->conn, cookie);
		xcb_atom_t type_atom             = prop_card32(reply);
		g_clear_pointer(&reply, free);
		if (type_atom != 0) {
			/* atom_name() возвращает malloc'нутую строку — g_autofree её освободит. */
			g_autofree char *name = atom_name(self->conn, type_atom);
			type = window_type_from_name(name);
		}
		if (state != NULL && type != WINDOW_TYPE_UNKNOWN)
			state->type = type;
	}
	if (type == WINDOW_TYPE_UNKNOWN && self->wm_window == XCB_NONE)
		find_wm_window(self);
	if (type == WINDOW_TYPE_UNKNOWN && self->wm_window != XCB_NONE &&
	    (window == self->wm_window || is_wm_child(self, window)))
		type = WINDOW_TYPE_DESKTOP;
	return type == WINDOW_TYPE_UNKNOWN ? WINDOW_TYPE_NORMAL : type;
}

gboolean vala_panel_xcb_aux_is_desktop_window(void *connection, uint32_t window)
{
	return vala_panel_xcb_aux_get_window_type(connection, window) == WINDOW_TYPE_DESKTOP;
}

uint32_t vala_panel_xcb_aux_get_transient_for(void *connection, uint32_t window)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self || window == VPA_WINDOW_NONE)
		return VPA_WINDOW_NONE;

	WindowState *state  = peek_state(self, window);
	xcb_window_t parent = state != NULL ? state->transient_for : XCB_NONE;
	if (parent != XCB_NONE)
		return parent;

	parent = read_card32(self, window, self->transient_for);
	if (parent == XCB_NONE) {
		xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_transient_for(self->conn, window);
		xcb_generic_error_t *error       = NULL;
		xcb_window_t transient           = XCB_NONE;
		if (!xcb_icccm_get_wm_transient_for_reply(self->conn, cookie, &transient, &error))
			transient = XCB_NONE;
		g_clear_pointer(&error, free);
		parent = transient;
	}
	if (state != NULL)
		state->transient_for = parent;
	return parent;
}

uint32_t vala_panel_xcb_aux_get_parent(void *connection, uint32_t window)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self || window == VPA_WINDOW_NONE)
		return VPA_WINDOW_NONE;

	xcb_query_tree_cookie_t cookie = xcb_query_tree(self->conn, window);
	xcb_generic_error_t *error     = NULL;
	xcb_query_tree_reply_t *tree   = xcb_query_tree_reply(self->conn, cookie, &error);
	xcb_window_t parent            = XCB_NONE;
	if (tree != NULL) {
		parent = tree->parent;
		free(tree);
	}
	g_clear_pointer(&error, free);
	return parent == self->root ? VPA_WINDOW_NONE : parent;
}

char *vala_panel_xcb_aux_get_name(void *connection, uint32_t window)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self || window == VPA_WINDOW_NONE)
		return NULL;
	char *name = vala_panel_xcb_aux_get_utf8_prop(connection, window, "_NET_WM_NAME");
	if (name != NULL)
		return name;
	return vala_panel_xcb_aux_get_utf8_prop(connection, window, "WM_NAME");
}

int64_t vala_panel_xcb_aux_get_pid(void *connection, uint32_t window)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self || window == VPA_WINDOW_NONE)
		return 0;
	return (int64_t) (int32_t) read_card32(self, window, self->wm_pid);
}

void vala_panel_xcb_aux_get_wm_class(void *connection, uint32_t window, char **instance,
                                     char **group)
{
	XcbAux *self = (XcbAux *) connection;
	if (instance != NULL)
		*instance = NULL;
	if (group != NULL)
		*group = NULL;
	if (!self || window == VPA_WINDOW_NONE)
		return;

	xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_class(self->conn, window);
	xcb_icccm_get_wm_class_reply_t reply;
	if (!xcb_icccm_get_wm_class_reply(self->conn, cookie, &reply, NULL))
		return;
	if (instance != NULL && reply.instance_name != NULL)
		*instance = g_strdup(reply.instance_name);
	if (group != NULL && reply.class_name != NULL)
		*group = g_strdup(reply.class_name);
	xcb_icccm_get_wm_class_reply_wipe(&reply);
}

GAppInfo *vala_panel_xcb_aux_match_app(void *matcher, void *connection, uint32_t window)
{
	XcbAux *self = (XcbAux *) connection;
	if (!VALA_PANEL_IS_MATCHER(matcher) || !self || window == VPA_WINDOW_NONE)
		return NULL;

	g_autofree char *instance = NULL;
	g_autofree char *group    = NULL;
	vala_panel_xcb_aux_get_wm_class(connection, window, &instance, &group);
	g_autofree char *gtk_id = vala_panel_xcb_aux_get_utf8_prop(connection, window, "_GTK_APPLICATION_ID");
	int64_t pid             = vala_panel_xcb_aux_get_pid(connection, window);

	GDesktopAppInfo *info =
	    vala_panel_matcher_match_arbitrary((ValaPanelMatcher *) matcher, instance, group, gtk_id, pid);
	return G_APP_INFO(info);
}

/* ------------------------------------------------------------- client list */

/* Re-reads the client list. EWMH lists the oldest window first while wnck puts
 * the most recently activated one first, so the order is reversed. */
static gboolean reload_client_list(XcbAux *self)
{
	xcb_ewmh_get_windows_reply_t clients = {0, NULL, NULL};
	xcb_get_property_cookie_t cookie     = xcb_ewmh_get_client_list_stacking(&self->ewmh, self->screen);
	gboolean have =
	    xcb_ewmh_get_client_list_stacking_reply(&self->ewmh, cookie, &clients, NULL) != 0;
	if (!have) {
		xcb_ewmh_get_windows_reply_wipe(&clients);
		memset(&clients, 0, sizeof(clients));
		cookie = xcb_ewmh_get_client_list(&self->ewmh, self->screen);
		have   = xcb_ewmh_get_client_list_reply(&self->ewmh, cookie, &clients, NULL) != 0;
	}

	if (!have) {
		/* No window manager publishes a client list here (an EWMH _CLIENT_LIST is
		 * only ever set by a window manager). The windows we have seen mapped are
		 * all there is, so they stay in the list rather than being pruned. */
		self->has_client_list = FALSE;
		return FALSE;
	}

	self->has_client_list = TRUE;
	g_list_free(self->order);
	self->order = NULL;
	for (guint i = clients.windows_len; i-- > 0;) {
		xcb_window_t window = clients.windows[i];
		if (window == self->wm_window)
			continue;
		order_add(self, window);
		/* need_state() returns the existing record when there is one; a client list
		 * entry means the window manager vouches for this window. */
		need_state(self, window);
	}
	xcb_ewmh_get_windows_reply_wipe(&clients);

	/* The order was rebuilt above from exactly the windows the manager lists, so any
	 * window still in the state cache is one it no longer names and has therefore been
	 * withdrawn or destroyed. Dropping it from both structures here matters for more
	 * than tidiness: collect_opened() skips a window already marked present, so a dead
	 * window left in the cache would never be reported closed - and openbox recycles
	 * window ids, so a later window handed the same id could never be announced again. */
	g_autoptr(GList) dropped = NULL;
	GHashTableIter iter;
	gpointer key, value;
	/* A window seen by this scan is marked with the current generation, so anything
	 * still carrying an older one is a window the manager has stopped naming. Using a
	 * generation counter rather than "is the table empty" is what makes this safe on
	 * the very first refresh: pruning there would otherwise throw away the windows that
	 * need_state() above has just inserted, because those insertions happen during this
	 * same call and are indistinguishable from leftovers by any emptiness test. */
	const guint generation = ++self->generation;
	for (GList *l = self->order; l != NULL; l = g_list_next(l)) {
		WindowState *state = need_state(self, GPOINTER_TO_UINT(l->data));
		state->generation  = generation;
	}
	g_hash_table_iter_init(&iter, self->windows);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		WindowState *state = value;
		if (GPOINTER_TO_UINT(key) == self->wm_window || state->generation == generation)
			continue;
		/* A window the map path introduced and the manager has never listed is not
		 * something the manager can vouch for; reconcile_windows() only keeps those
		 * alive while there is no client list at all, so here they are simply released. */
		if (!state->present) {
			g_hash_table_iter_remove(&iter);
			continue;
		}
		/* Announced and now unlisted: it is closed. Marking it not known lets
		 * collect_closed() report it, and its state entry then goes with it. */
		state->known = FALSE;
		dropped      = g_list_prepend(dropped, key);
	}
	for (GList *l = dropped; l != NULL; l = g_list_next(l))
		order_remove(self, GPOINTER_TO_UINT(l->data));

	/* A window the manager no longer names. If it had been announced it is closed;
	 * its record is kept (flagged not present) so that a recycled window id can still
	 * be recognised as a new window instead of looking already known. */
	return TRUE;
}

static void note_active(XcbAux *self, xcb_window_t focused)
{
	if (focused != XCB_NONE) {
		xcb_window_t client = frame_client(self, focused);
		if (client != XCB_NONE)
			focused = client;
	}
	self->active = focused;
}

static void read_active_window(XcbAux *self)
{
    note_active(self, read_card32(self, self->root, self->ewmh._NET_ACTIVE_WINDOW));
}
static void refresh(XcbAux *self)
{
	find_wm_window(self);
	reload_client_list(self);
	read_active_window(self);
}

/* ------------------------------------------------------------- event handling */

static void on_property_notify(XcbAux *self, xcb_property_notify_event_t *event)
{
	/* The client list is only announced on the root window. */
	if (event->window == self->root) {
		if (event->atom == self->ewmh._NET_ACTIVE_WINDOW) {
			read_active_window(self);
		} else if (event->atom == self->ewmh._NET_CLIENT_LIST ||
		         event->atom == self->ewmh._NET_CLIENT_LIST_STACKING ||
		         event->atom == self->ewmh._NET_SUPPORTING_WM_CHECK)
			refresh(self);
		return;
	}

	/* A client list event on a non root window still has to be re-read: the
	 * stacking order can change without any property on the root changing. */
	if (event->atom == self->ewmh._NET_CLIENT_LIST_STACKING ||
	    event->atom == self->ewmh._NET_CLIENT_LIST) {
		reload_client_list(self);
		return;
	}

	WindowState *state = peek_state(self, event->window);
	if (state == NULL)
		return;
	if (event->atom == self->net_wm_type)
		state->type = WINDOW_TYPE_UNKNOWN;
	else if (event->atom == self->transient_for)
		state->transient_for = XCB_NONE;
}

static void on_map(XcbAux *self, xcb_map_notify_event_t *event)
{
	if (event->window == self->root || event->window == self->wm_window)
		return;
	xcb_window_t client = frame_client(self, event->window);
	xcb_window_t window = client != XCB_NONE ? client : event->window;

	/* A map notify only refreshes a window the cache already knows about. Windows
	 * enter the cache through the window manager's client list, which keeps
	 * override-redirect popups, menus and icon windows out of the list wnck would
	 * otherwise have to report as applications. Without a client list there is no
	 * such authority, so a mapped window does become a client. */
	WindowState *state = peek_state(self, window);
	if (state == NULL) {
		if (self->has_client_list)
			return;
		order_add(self, window);
		state = need_state(self, window);
	}
	/* Mapping can change what a window is: its type and transient parent are read
	 * again on demand rather than trusted from whatever it was before. */
	state->type          = WINDOW_TYPE_UNKNOWN;
	state->transient_for = XCB_NONE;

	if (window != event->window) {
		/* The frame itself is not something the backend should see. */
		order_remove(self, event->window);
	}
}

static void on_unmap(XcbAux *self, xcb_window_t window)
{
	if (window == self->root || window == XCB_NONE)
		return;
	xcb_window_t client = frame_client(self, window);
	xcb_window_t gone   = client != XCB_NONE ? client : window;

	/* An unmapped window means the client is gone. A window manager removes the
	 * _NET_CLIENT_LIST entry whether the application exited cleanly or was killed, but
	 * acting on the unmap here gives an abrupt exit a second path to be noticed rather
	 * than depending only on the list change arriving. */
	WindowState *state = peek_state(self, gone);
	if (state != NULL && state->present) {
		order_remove(self, gone);
		state->known = FALSE;
	}
	if (client != XCB_NONE)
		order_remove(self, window);
}

/* Consumes every event already queued. */
static void drain_events(XcbAux *self)
{
	xcb_generic_event_t *event = xcb_poll_for_event(self->conn);
	while (event != NULL) {
		switch (XCB_EVENT_RESPONSE_TYPE(event)) {
		case XCB_PROPERTY_NOTIFY:
			on_property_notify(self, (xcb_property_notify_event_t *) event);
			break;
		case XCB_MAP_NOTIFY:
			on_map(self, (xcb_map_notify_event_t *) event);
			break;
		case XCB_UNMAP_NOTIFY:
			on_unmap(self, ((xcb_unmap_notify_event_t *) event)->window);
			break;
		case XCB_DESTROY_NOTIFY:
			on_unmap(self, ((xcb_destroy_notify_event_t *) event)->window);
			break;
		case XCB_REPARENT_NOTIFY: {
			xcb_reparent_notify_event_t *reparent = (xcb_reparent_notify_event_t *) event;
			if (reparent->parent == self->root || reparent->parent == XCB_WINDOW_NONE)
				reload_client_list(self);
			break;
		}
		default:
			break;
		}
		free(event);
		if (xcb_connection_has_error(self->conn)) {
			self->lost = TRUE;
			return;
		}
		event = xcb_poll_for_event(self->conn);
	}
}

/* Asks about a batch of windows at once and reports which are still around.
 * Sending the requests together keeps one reconcile pass to a single round trip per
 * batch; asking one window at a time would stall on every reply.
 *
 * GetWindowAttributes is used rather than GetProperty. Asking whether a window id
 * exists is all that is being tested, and a property request cannot express that:
 * the protocol rejects a property argument of XCB_ATOM_ANY with BadValue (it is only
 * valid as the requested *type*), so such a request fails for every window and each
 * one would read as destroyed. A request for a real atom no window sets is not an
 * error at all - it comes back with type NONE and no value bytes - which is why the
 * property getters elsewhere here, which always pass an interned atom id, are fine.
 *
 * An unmapped window counts as absent, which is right for the only caller of this
 * probe: it runs when there is no client list to go by, so the windows being probed
 * were noticed because they mapped, and "no longer mapped" is exactly what "this client
 * went away" means there. Since watch_window() puts each window in the save set, a
 * closed application's id usually stays valid for a while, so an error reply on its own
 * would not be enough to notice. With a window manager running, liveness comes from its
 * client list instead, which is what keeps an iconified window alive.
 *
 * Replies are read in the order the requests were sent. libxcb hands back whatever
 * arrives next, so an error for one window would otherwise be taken as another's reply
 * and shift every result after it. */
static void probe_windows(XcbAux *self, const xcb_window_t *windows, guint n,
                          gboolean *results)
{
	xcb_get_window_attributes_cookie_t cookies[QUERY_BATCH];
	guint count = n < QUERY_BATCH ? n : QUERY_BATCH;

	for (guint i = 0; i < count; i++)
		cookies[i] = xcb_get_window_attributes(self->conn, windows[i]);

	for (guint i = 0; i < count; i++) {
		xcb_generic_error_t *error                       = NULL;
		xcb_get_window_attributes_reply_t *reply         =
		    xcb_get_window_attributes_reply(self->conn, cookies[i], &error);
		g_clear_pointer(&error, free);
		results[i] = reply != NULL && reply->map_state != XCB_MAP_STATE_UNMAPPED;
		g_clear_pointer(&reply, free);
	}
}

static void reconcile_windows(XcbAux *self)
{
	if (self->has_client_list)
		return;

	g_autoptr(GList) candidates = NULL;
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, self->windows);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		if (!value)
			continue;
		candidates = g_list_prepend(candidates, key);
	}
	candidates = g_list_reverse(candidates);

	GList *cursor = candidates;
	while (cursor != NULL) {
		xcb_window_t batch[QUERY_BATCH];
		gboolean answers[QUERY_BATCH];
		guint count = 0;

		while (cursor != NULL && count < QUERY_BATCH) {
			batch[count++] = GPOINTER_TO_UINT(cursor->data);
			cursor         = g_list_next(cursor);
		}

		probe_windows(self, batch, count, answers);
		for (guint i = 0; i < count; i++) {
			WindowState *state = need_state(self, batch[i]);
			state->known       = answers[i];
			if (!answers[i])
				order_remove(self, batch[i]);
		}
	}
}

/* Reports the windows that were reported as opened but are no longer present,
 * dropping them from the cache. Only windows currently marked present can be
 * closed, so a window that has just been discovered is never reported twice. */
static GList *collect_closed(XcbAux *self)
{
	GList *closed = NULL;

	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, self->windows);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		WindowState *state = value;
		if (state->known) {
			continue;
		}
		if (state->present) {
			closed = g_list_prepend(closed, key);
			/* The state entry is deliberately left in place, flagged closed. A window id
			 * can come back: openbox reuses frame ids, and if the record were removed here
			 * a window handed the recycled id would look brand new and never be announced.
			 * Keeping it lets collect_opened() treat "was gone, now listed again" as a new
			 * window. unref() releases the whole cache at once, so nothing leaks. */
			state->present = FALSE;
			state->known   = FALSE;
			continue;
		}
		/* A window we forget is no longer watched: drop the save set entry taken out
		 * for it so the server does not keep a reference to a window we ignore. */
		xcb_change_save_set(self->conn, XCB_SET_MODE_DELETE, GPOINTER_TO_UINT(key));
		g_hash_table_iter_remove(&iter);
	}
	return closed;
}

static GList *collect_opened(XcbAux *self)
{
	GList *opened = NULL;

	for (GList *l = self->order; l != NULL; l = g_list_next(l)) {
		xcb_window_t window = GPOINTER_TO_UINT(l->data);
		WindowState *state  = need_state(self, window);
		/* Membership in the manager's list is what says a window exists, so every listed
		 * window is marked known here - including one whose record was left over from an
		 * earlier life under a recycled id. A record that is not refreshed below is
		 * therefore stale, and reconcile_windows() or the client list prune clears it. */
		state->known = TRUE;
		if (state->present)
			continue;
		state->present = TRUE;
		opened         = g_list_prepend(opened, GUINT_TO_POINTER(window));
	}
	return g_list_reverse(opened);
}

static void emit_changes(XcbAux *self)
{
	reconcile_windows(self);

	g_autoptr(GList) closed = collect_closed(self);
	g_autoptr(GList) opened = collect_opened(self);

	gboolean announce_active = self->active != self->active_reported && self->active_reported != 0;
	self->active_reported    = self->active;

	for (GList *i = closed; i != NULL; i = g_list_next(i)) {
		xcb_window_t window = GPOINTER_TO_UINT(i->data);
		if (window == self->active)
			announce_active = TRUE;
	}

	/* Обработчик сигнала может добавить или убрать слушателя прямо во время
	 * эмиссии. Обходим снапшот, и пропускаем слушателя, которого уже нет в
	 * self->listeners (иначе следующая итерация пойдёт по освобождённому узлу). */
	g_autoptr(GList) snapshot = g_list_copy(self->listeners);
	for (GList *l = snapshot; l != NULL; l = g_list_next(l)) {
		GObject *listener = l->data;
		if (g_list_find(self->listeners, listener) == NULL)
			continue;
		for (GList *i = closed; i != NULL; i = g_list_next(i))
			if (signal_closed != 0)
				g_signal_emit(listener, signal_closed, 0, GPOINTER_TO_UINT(i->data));
		for (GList *i = opened; i != NULL; i = g_list_next(i))
			if (signal_opened != 0)
				g_signal_emit(listener, signal_opened, 0, GPOINTER_TO_UINT(i->data));
		if (announce_active && signal_activity != 0)
			g_signal_emit(listener, signal_activity, 0);
	}
}

/* ---------------------------------------------------------------- lifecycle */

/* Runs one batch of work whenever the connection has something to say. Replies
 * only ever come from requests issued here, and those always have their reply
 * read straight away, so everything readable at this point is an event. */
/* ------------------------------------------------------------------ watching
 *
 * The main loop watches the connection through a plain GSource rather than a
 * GIOChannel source. A GIOChannel source only fires on a condition it has just been
 * notified about, so bytes that reached the socket before it was registered - which is
 * normal, since selecting root events and starting applications happen while we are
 * still setting up - can sit unread in libxcb's buffer without any further fd read
 * readiness, and then the window list would never appear.
 *
 * prepare() closes that gap: it flushes any request we still owe the server and asks
 * whether the socket is readable right now, which is exactly how libxcb's own examples
 * integrate with an event loop. check() reports the readiness prepare() observed. The
 * loop therefore sleeps normally between events instead of being woken for nothing.
 *
 * The fd must never be read here; only xcb_poll_for_event() in dispatch may consume
 * from it, or libxcb would lose partial replies. */
static gboolean xcb_source_prepare(GSource *source, int *timeout)
{
	XcbWatch *watch = (XcbWatch *) source;
	XcbAux *self    = watch->aux;

	*timeout               = -1;
	watch->poll_fd.revents = 0;
	if (self->lost)
		return FALSE;

	/* Anything we asked for has to be on its way before it can be answered. */
	if (xcb_flush(self->conn) <= 0 && xcb_connection_has_error(self->conn)) {
		self->lost = TRUE;
		return FALSE;
	}

	/* A non blocking poll of length zero tells us whether data is already waiting, with
	 * no wakeup cost and no chance of consuming anything. */
	g_poll(&watch->poll_fd, 1, 0);
	return (watch->poll_fd.revents & (G_IO_IN | G_IO_HUP | G_IO_ERR)) != 0;
}

static gboolean xcb_source_check(GSource *source)
{
	XcbWatch *watch = (XcbWatch *) source;
	return (watch->poll_fd.revents & (G_IO_IN | G_IO_HUP | G_IO_ERR)) != 0;
}

static gboolean xcb_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
	(void) callback;
	(void) user_data;
	XcbAux *self = ((XcbWatch *) source)->aux;

	drain_events(self);
	emit_changes(self);

	if (xcb_connection_has_error(self->conn)) {
		/* No further event can arrive, so the watch goes away with them. */
		self->lost = TRUE;
	}
	if (self->lost) {
		/* The source removes itself; nothing may call back into the helper afterwards. */
		self->watch = NULL;
		return G_SOURCE_REMOVE;
	}
	return G_SOURCE_CONTINUE;
}

static GSourceFuncs xcb_source_funcs = {xcb_source_prepare, xcb_source_check,
                                        xcb_source_dispatch, NULL, NULL, NULL};

static void init_atoms(XcbAux *self)
{
	xcb_intern_atom_cookie_t *cookies = xcb_ewmh_init_atoms(self->conn, &self->ewmh);
	if (!xcb_ewmh_init_atoms_replies(&self->ewmh, cookies, NULL)) {
		g_warning("xcb-aux: the window manager does not announce EWMH support");
		return;
	}
	/* xcb-ewmh pre-interns most of what we need but leaves _NET_WM_TRANSIENT_FOR
	 * out, so it is interned by hand. */
	self->net_wm_type   = self->ewmh._NET_WM_WINDOW_TYPE;
	self->transient_for = intern_name(self->conn, "_NET_WM_TRANSIENT_FOR");
	self->wm_pid        = self->ewmh._NET_WM_PID;
}

static void init_screen(XcbAux *self)
{
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(self->conn));
	for (int i = 0; i < self->screen; i++)
		xcb_screen_next(&iter);
	self->root = iter.data != NULL ? iter.data->root : XCB_NONE;
}

XcbAux *vala_panel_xcb_aux_connect(void)
{
	if (shared != NULL) {
		shared->refs++;
		return shared;
	}

	XcbAux *self = g_new0(XcbAux, 1);
	self->screen = 0;
	self->conn   = xcb_connect(NULL, &self->screen);
	if (xcb_connection_has_error(self->conn) || self->screen < 0) {
		if (!xcb_connection_has_error(self->conn))
			xcb_disconnect(self->conn);
		g_free(self);
		return NULL;
	}

	init_screen(self);
	if (self->root == XCB_NONE) {
		xcb_disconnect(self->conn);
		g_free(self);
		return NULL;
	}
	init_atoms(self);

	self->windows   = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
	self->refs      = 1;
	self->wm_window = XCB_NONE;
	self->active    = XCB_NONE;
	shared          = self;
	return self;
}

void vala_panel_xcb_aux_unref(void *connection)
{
	XcbAux *self = (XcbAux *) connection;
	/* The helper keeps one process wide instance, so the handle a caller holds and
	 * the shared one are the same object; drop whichever was handed to us. */
	if (self == NULL || --self->refs > 0)
		return;

	/* Released before anything else, and before `shared` is cleared: remove_listener()
	 * finds the helper through `shared`, so tearing it down afterwards would leak every
	 * listener reference taken in add_listener(). */
	vala_panel_xcb_aux_remove_listener(NULL);
	shared = NULL;

	if (self->watch != NULL) {
		/* Detaches the source if it is still attached. The memory belongs to GLib, which
		 * frees it once nothing else holds a reference; our own ref was dropped right
		 * after attach, so this is the one that releases it. */
		g_source_destroy(&self->watch->source);
		self->watch = NULL;
	}
	g_list_free(self->order);
	g_hash_table_destroy(self->windows);
	xcb_ewmh_connection_wipe(&self->ewmh);
	xcb_disconnect(self->conn);
	g_free(self);
}

void vala_panel_xcb_aux_add_listener(void *connection, GObject *listener)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self || !G_IS_OBJECT(listener))
		return;
	if (g_list_find(self->listeners, listener) != NULL)
		return;

	/* Сначала догоняем состояние и даём уже зарегистрированным слушателям
	 * услышать всё, что изменилось с прошлого раза. Иначе refresh() здесь
	 * «украдёт» у них, например, wm-window-closed — а новый слушатель потом
	 * получит его для окна, о котором никогда не слышал wm-window-opened. */
	refresh(self);
	emit_changes(self);

	ensure_signals(G_TYPE_FROM_INSTANCE(listener));
	self->listeners = g_list_prepend(self->listeners, g_object_ref(listener));

	/* Праймим нового слушателя текущим состоянием. После emit_changes() список
	 * self->order содержит ровно те окна, что помечены present. */
	for (GList *l = self->order; l != NULL; l = g_list_next(l)) {
		xcb_window_t window = GPOINTER_TO_UINT(l->data);
		if (signal_opened != 0)
			g_signal_emit(listener, signal_opened, 0, window);
	}
	if (self->active != XCB_NONE && signal_activity != 0)
		g_signal_emit(listener, signal_activity, 0);
}

void vala_panel_xcb_aux_remove_listener(GObject *listener)
{
	/* The helper is looked up by name rather than passed in, because a listener has no
	 * handle to pass and unref() wants to clear everything while it is tearing the
	 * helper down. That lookup must still find the helper, so unref() releases the
	 * listeners before it clears `shared`; otherwise this becomes a no-op exactly when
	 * it matters most and the listener references are leaked. */
	XcbAux *self = shared;
	if (!self)
		return;
	if (listener == NULL) {
		for (GList *l = self->listeners; l != NULL; l = g_list_next(l))
			g_object_unref(l->data);
		g_list_free(self->listeners);
		self->listeners = NULL;
		return;
	}
	GList *link = g_list_find(self->listeners, listener);
	if (link == NULL)
		return;
	g_object_unref(link->data);
	self->listeners = g_list_delete_link(self->listeners, link);
}

void vala_panel_xcb_aux_watch_root(void *connection)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self || self->watch != NULL)
		return;

	/* SubstructureNotify covers the child map, reparent and configure events of the
	 * root. It deliberately does not cover the UnmapNotify and DestroyNotify for a
	 * child: those are leave notifications sent to the parent only, so watching the
	 * root is not enough to hear that a window went away - which matters most for an
	 * abrupt client death, where openbox leaves its _NET_CLIENT_LIST entry behind.
	 * watch_window() supplies them for every window the cache holds. */
	uint32_t mask[] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE};
	xcb_change_window_attributes(self->conn, self->root, XCB_CW_EVENT_MASK, mask);

	for (GList *l = self->order; l != NULL; l = g_list_next(l))
		watch_window(self, GPOINTER_TO_UINT(l->data));
	xcb_flush(self->conn);

	/* A GIOChannel source would miss bytes that arrived before it was registered; see
	 * the GSource above. g_source_new() allocates the XcbWatch itself, since its size is
	 * the source's, and releases that memory when the last reference goes - which is why
	 * XcbWatch is never allocated on its own. */
	g_autoptr(GSource) source = g_source_new((GSourceFuncs *) &xcb_source_funcs, sizeof(XcbWatch));
	self->watch       = (XcbWatch *) source;
	self->watch->aux  = self;
	self->watch->poll_fd.fd     = xcb_get_file_descriptor(self->conn);
	self->watch->poll_fd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
	g_source_add_poll(source, &self->watch->poll_fd);
	g_source_attach(source, g_main_context_default());
}

/* ------------------------------------------------------------- accessors 2 */
uint32_t vala_panel_xcb_aux_get_active_window(void *connection)
{
	XcbAux *self = (XcbAux *) connection;
	if (!self)
		return VPA_WINDOW_NONE;
	if (self->active == XCB_NONE)
		read_active_window(self);
	return self->active;
}

gboolean is_applicable(void)
{
	int screen_num = 0;
	xcb_connection_t *conn = xcb_connect(NULL, &screen_num);

	if (conn == NULL || xcb_connection_has_error(conn)) {
		if (conn != NULL)
			xcb_disconnect(conn);
		return FALSE;
	}
	xcb_query_extension_cookie_t cookie =
	    xcb_query_extension(conn, (uint16_t) (sizeof("XWAYLAND") - 1), "XWAYLAND");
	xcb_query_extension_reply_t *reply = xcb_query_extension_reply(conn, cookie, NULL);

	gboolean is_xwayland = FALSE;
	if (reply != NULL) {
		is_xwayland = reply->present != 0;
		free(reply);
	}

	xcb_disconnect(conn);
	return !is_xwayland;
}
