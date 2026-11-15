/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include "comparator.h"

typedef struct
{
	guint signal_count;
} LayoutUpdatedCounter;

static void layout_updated_handler(GDBusConnection *connection, const gchar *sender_name,
                                   const gchar *object_path, const gchar *interface_name,
                                   const gchar *signal_name, GVariant *parameters,
                                   gpointer user_data)
{
	LayoutUpdatedCounter *counter = user_data;
	counter->signal_count++;
}

void test_func(void)
{
	GMenu *root     = g_menu_new();
	GMenu *section1 = g_menu_new();
	g_menu_append(section1, "New", "app.new");
	g_menu_append_section(root, NULL, G_MENU_MODEL(section1));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root), g_simple_action_group_new());

	GError *error               = NULL;
	GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	g_assert_no_error(error);

	LayoutUpdatedCounter counter = { 0 };

	guint sub = g_dbus_connection_signal_subscribe(connection,
	                                               TEST_BUS_NAME,
	                                               "com.canonical.dbusmenu",
	                                               "LayoutUpdated",
	                                               TEST_OBJECT_PATH,
	                                               NULL,
	                                               G_DBUS_SIGNAL_FLAGS_NONE,
	                                               layout_updated_handler,
	                                               &counter,
	                                               NULL);
	g_assert_cmpuint(sub, >, 0);

	g_menu_append(section1, "Open", "app.open");
	g_menu_append(section1, "Save", "app.save");
	GMenu *section2 = g_menu_new();
	g_menu_append(section2, "Quit", "app.quit");
	g_menu_append_section(root, NULL, G_MENU_MODEL(section2));
	g_object_unref(section2);

	test_wait_ms(300);
	g_assert_cmpuint(counter.signal_count, ==, 1);

	test_wait_ms(100);
	g_assert_cmpuint(counter.signal_count, ==, 1);

	g_menu_append(section1, "Close", "app.close");

	test_wait_ms(300);
	g_assert_cmpuint(counter.signal_count, ==, 2);

	test_wait_ms(100);
	g_assert_cmpuint(counter.signal_count, ==, 2);

	g_dbus_connection_signal_unsubscribe(connection, sub);
	g_object_unref(connection);

	fixture_teardown(&f);
	g_object_unref(section1);
}
