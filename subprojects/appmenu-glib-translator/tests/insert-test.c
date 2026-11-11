/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"

static void size_state_change(GSimpleAction *action, GVariant *value, gpointer user_data)
{
	(void)user_data;
	g_simple_action_set_state(action, value);
}

G_GNUC_INTERNAL void test_func(void)
{
	GMenu *root    = g_menu_new();
	GMenu *section = g_menu_new();

	GMenuItem *small = g_menu_item_new("Small", "app.size");
	g_menu_item_set_attribute(small, G_MENU_ATTRIBUTE_TARGET, "s", "small");
	g_menu_append_item(section, small);
	g_object_unref(small);

	GMenuItem *large = g_menu_item_new("Large", "app.size");
	g_menu_item_set_attribute(large, G_MENU_ATTRIBUTE_TARGET, "s", "large");
	g_menu_append_item(section, large);
	g_object_unref(large);

	g_menu_append_section(root, NULL, G_MENU_MODEL(section));

	GSimpleActionGroup *action_group = g_simple_action_group_new();

	GSimpleAction *size_action = g_simple_action_new_stateful("size",
	                                                          G_VARIANT_TYPE_STRING,
	                                                          g_variant_new_string("small"));
	g_signal_connect(size_action, "change-state", G_CALLBACK(size_state_change), NULL);
	g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(size_action));

	Fixture f;
	f.source       = G_MENU_MODEL(root);
	f.action_group = g_object_ref_sink(action_group);
	f.exporter     = dbus_menu_exporter_new(f.source,
	                                        NULL,
	                                        NULL, /* prefix, group */
	                                        TEST_BUS_NAME,
	                                        TEST_OBJECT_PATH);
	test_wait();
	f.importer = dbus_menu_importer_new(TEST_BUS_NAME, TEST_OBJECT_PATH);
	test_wait();
	g_object_get(f.importer, "model", &f.target, NULL);

	GString *errors = g_string_new(NULL);

	/* --- 1. Before insert: source and target must NOT be equal. --- */
	gboolean equal = compare_models(f.source, f.target, errors, 0);
	if (equal)
		dump_and_report("radio-insert-remove/pre-insert (expected differ)", &f, errors);
	g_assert_false(equal);
	g_string_set_size(errors, 0);

	/* --- 2. Insert the action group under prefix "app". --- */
	dbus_menu_exporter_insert(f.exporter, "app", G_ACTION_GROUP(action_group));
	test_wait();

	/* --- 3. After insert: source and target MUST be equal. --- */
	equal = compare_models(f.source, f.target, errors, 0);
	if (!equal)
		dump_and_report("radio-insert-remove/post-insert (expected equal)", &f, errors);
	g_assert_true(equal);
	g_string_set_size(errors, 0);

	/* --- 4. Remove the action group. --- */
	dbus_menu_exporter_remove(f.exporter, "app");
	test_wait();

	/* --- 5. After remove: source and target must NOT be equal again. --- */
	equal = compare_models(f.source, f.target, errors, 0);
	if (equal)
		dump_and_report("radio-insert-remove/post-remove (expected differ)", &f, errors);
	g_assert_false(equal);

	g_string_free(errors, TRUE);
	g_object_unref(size_action);
	g_object_unref(action_group);
	fixture_teardown(&f);
	g_object_unref(section);
}