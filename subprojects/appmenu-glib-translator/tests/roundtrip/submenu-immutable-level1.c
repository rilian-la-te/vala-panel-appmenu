/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include "comparator.h"
#include "gio/gmenu.h"

typedef struct
{
	guint layout_updated_count;
	guint items_changed_count;
} SignalCounters;

static void layout_updated_handler(GDBusConnection *connection, const gchar *sender_name,
                                   const gchar *object_path, const gchar *interface_name,
                                   const gchar *signal_name, GVariant *parameters,
                                   gpointer user_data)
{
	(void)connection;
	(void)sender_name;
	(void)object_path;
	(void)interface_name;
	(void)signal_name;
	(void)parameters;

	SignalCounters *counters = user_data;
	counters->layout_updated_count++;
}

static void items_changed_handler(GMenuModel *model, gint position, gint removed, gint added,
                                  gpointer user_data)
{
	(void)model;
	(void)position;
	(void)removed;
	(void)added;

	SignalCounters *counters = user_data;
	counters->items_changed_count++;
}

G_GNUC_INTERNAL void test_func(void)
{
	GMenu *root     = g_menu_new();
	GMenu *submenu1 = g_menu_new();
	GMenu *submenu2 = g_menu_new();
	g_menu_append(submenu1, "Item 0", "app.item1");

	g_menu_append(submenu2, "Item 1", "app.item1");
	g_menu_append(submenu2, "Item 2", "app.item2");

	g_menu_append_submenu(root, "Submenu", G_MENU_MODEL(submenu1));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root), g_simple_action_group_new());

	GString *errors = g_string_new(NULL);

	gboolean equal = compare_models(f.source, f.target, errors, 0);
	if (!equal)
		dump_and_report("submenu-replace/initial (expected equal)", &f, errors);
	g_assert_true(equal);
	g_string_set_size(errors, 0);

	GError *error               = NULL;
	GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	g_assert_no_error(error);

	SignalCounters counters = { 0, 0 };

	guint dbus_sub = g_dbus_connection_signal_subscribe(connection,
	                                                    TEST_BUS_NAME,
	                                                    "com.canonical.dbusmenu",
	                                                    "LayoutUpdated",
	                                                    TEST_OBJECT_PATH,
	                                                    NULL,
	                                                    G_DBUS_SIGNAL_FLAGS_NONE,
	                                                    layout_updated_handler,
	                                                    &counters,
	                                                    NULL);
	g_assert_cmpuint(dbus_sub, >, 0);

	GMenuModel *imported_section = g_menu_model_get_item_link(f.target, 0, G_MENU_LINK_SECTION);
	g_assert_nonnull(imported_section);

	gulong section_sub = g_signal_connect(f.target,
	                                      "items-changed",
	                                      G_CALLBACK(items_changed_handler),
	                                      &counters);
	g_assert_cmpuint(section_sub, >, 0);

	g_menu_remove(root, 0);
	dump_models("stage 1", f.source, f.target);
	test_wait_ms(25);
	g_menu_append_submenu(root, "Submenu", G_MENU_MODEL(submenu2));
	dump_models("stage 2", f.source, f.target);
	test_wait_ms(1000);

	g_assert_cmpuint(counters.layout_updated_count, ==, 1);
	g_assert_cmpuint(counters.items_changed_count, ==, 0);
	dump_models("stage 3", f.source, f.target);
	equal = compare_models(f.source, f.target, errors, 0);
	if (!equal)
		dump_and_report("submenu-replace/post-replace (expected equal)", &f, errors);
	g_assert_true(equal);

	g_string_free(errors, TRUE);
	g_signal_handler_disconnect(f.target, section_sub);
	g_object_unref(imported_section);
	g_dbus_connection_signal_unsubscribe(connection, dbus_sub);
	g_object_unref(connection);

	fixture_teardown(&f);
	g_object_unref(submenu1);
	g_object_unref(submenu2);
}
