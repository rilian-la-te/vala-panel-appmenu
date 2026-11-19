/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include "comparator.h"

typedef struct
{
	guint signal_count;
	guint last_revision;
} LayoutUpdatedTracker;

static void layout_updated_handler(GDBusConnection *connection, const gchar *sender_name,
                                   const gchar *object_path, const gchar *interface_name,
                                   const gchar *signal_name, GVariant *parameters,
                                   gpointer user_data)
{
	LayoutUpdatedTracker *tracker = user_data;

	guint revision = 0;
	gint32 parent  = 0;
	g_variant_get(parameters, "(ui)", &revision, &parent);

	tracker->last_revision = revision;
	tracker->signal_count++;
}

typedef struct
{
	gboolean done;
	guint    revision;
} GetLayoutRevisionData;

static void get_layout_revision_ready(GObject *source_object, GAsyncResult *result,
                                      gpointer user_data)
{
	GetLayoutRevisionData *data = user_data;

	GError *error   = NULL;
	GVariant *reply = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object), result, &error);
	g_assert_no_error(error);

	GVariant *revision_variant = g_variant_get_child_value(reply, 0);
	data->revision = g_variant_get_uint32(revision_variant);
	g_variant_unref(revision_variant);
	g_variant_unref(reply);

	data->done = TRUE;
}

/* GetLayout(parentId, recursionDepth, propertyNames) -> (u revision, (ia{sv}av) layout). */
static guint get_layout_revision(GDBusConnection *connection)
{
	GetLayoutRevisionData data = { FALSE, 0 };

	g_dbus_connection_call(connection,
	                       TEST_BUS_NAME,
	                       TEST_OBJECT_PATH,
	                       "com.canonical.dbusmenu",
	                       "GetLayout",
	                       g_variant_new("(ii@as)", 0, -1, g_variant_new_strv(NULL, 0)),
	                       G_VARIANT_TYPE("(u(ia{sv}av))"),
	                       G_DBUS_CALL_FLAGS_NONE,
	                       -1,
	                       NULL,
	                       get_layout_revision_ready,
	                       &data);

	/* Pump the harness main loop until the reply arrives (bounded to ~1 s). */
	for (guint i = 0; i < 20 && !data.done; i++)
		test_wait_ms(50);
	g_assert_true(data.done);

	return data.revision;
}

void test_func(void)
{
	GMenu *root     = g_menu_new();
	GMenu *section1 = g_menu_new();
	g_menu_append(section1, "New", "app.new");
	g_menu_append_section(root, NULL, G_MENU_MODEL(section1));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root), g_simple_action_group_new());
	test_wait();

	GError *error               = NULL;
	GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	g_assert_no_error(error);

	LayoutUpdatedTracker tracker = { 0 };

	guint sub = g_dbus_connection_signal_subscribe(connection,
	                                               TEST_BUS_NAME,
	                                               "com.canonical.dbusmenu",
	                                               "LayoutUpdated",
	                                               TEST_OBJECT_PATH,
	                                               NULL,
	                                               G_DBUS_SIGNAL_FLAGS_NONE,
	                                               layout_updated_handler,
	                                               &tracker,
	                                               NULL);
	g_assert_cmpuint(sub, >, 0);

	/* --- 1. GetLayout: remember the revision currently reported by the server. --- */
	const guint revision_before = get_layout_revision(connection);

	/* --- 2. Change the menu model. --- */
	g_menu_append(section1, "Open", "app.open");

	test_wait_ms(300);
	g_assert_cmpuint(tracker.signal_count, ==, 1);

	/* --- 3. LayoutUpdated must carry a revision greater than GetLayout's. --- */
	g_assert_cmpuint(tracker.last_revision, >, revision_before);

	test_wait_ms(100);
	g_assert_cmpuint(tracker.signal_count, ==, 1);

	g_dbus_connection_signal_unsubscribe(connection, sub);
	g_object_unref(connection);

	fixture_teardown(&f);
	g_object_unref(section1);
} 