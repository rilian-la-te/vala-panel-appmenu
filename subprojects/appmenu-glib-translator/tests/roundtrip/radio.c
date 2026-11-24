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
	fixture_setup(&f, G_MENU_MODEL(root), action_group);

	GString *errors = g_string_new(NULL);

	/* --- Initial state ("small"). --- */
	gboolean ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("radio-selection-change/initial", &f, errors);
	g_assert_true(ok);
	g_string_set_size(errors, 0);

	/* --- string state change. --- */
	g_action_group_change_action_state(G_ACTION_GROUP(action_group),
	                                   "size",
	                                   g_variant_new_string("large"));
	test_wait();

	/* --- string state change works? --- */
	GVariant *state = g_action_group_get_action_state(G_ACTION_GROUP(action_group), "size");
	g_assert_nonnull(state);
	g_assert_cmpstr(g_variant_get_string(state, NULL), ==, "large");
	g_variant_unref(state);

	/* --- Compare models again. --- */
	ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("radio-selection-change/after-change", &f, errors);
	g_assert_true(ok);

	g_string_free(errors, TRUE);
	g_object_unref(size_action);
	g_object_unref(action_group);
	fixture_teardown(&f);
	g_object_unref(section);
}