/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"
#include "definitions.h"

static void size_state_change(GSimpleAction *action, GVariant *value, gpointer user_data)
{
	(void)user_data;
	g_simple_action_set_state(action, value);
}

static void check_source_state(GSimpleActionGroup *group, const char *expected)
{
	GVariant *state = g_action_group_get_action_state(G_ACTION_GROUP(group), "size");
	g_assert_nonnull(state);
	g_assert_true(g_variant_is_of_type(state, G_VARIANT_TYPE_STRING));
	g_assert_cmpstr(g_variant_get_string(state, NULL), ==, expected);
	g_variant_unref(state);
}

/* The importer synthesizes one string-stateful action per radio item,
 * named "id-<n>" after the DBusMenu item id. The state is either
 * DBUS_MENU_ACTION_RADIO_SELECTED or DBUS_MENU_ACTION_RADIO_UNSELECTED. */
static void check_importer_radio(GActionGroup *group, const char *id, gboolean selected)
{
	GVariant *state = g_action_group_get_action_state(group, id);
	g_assert_nonnull(state);
	g_assert_true(g_variant_is_of_type(state, G_VARIANT_TYPE_STRING));
	g_assert_cmpstr(g_variant_get_string(state, NULL),
	                ==,
	                selected ? DBUS_MENU_ACTION_RADIO_SELECTED
	                         : DBUS_MENU_ACTION_RADIO_UNSELECTED);
	g_variant_unref(state);
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

	/* Grab the importer's model AND its synthesized action group. */
	GMenuModel *importer_model   = NULL;
	GActionGroup *importer_group = NULL;
	g_object_get(f.importer, "model", &importer_model, "action-group", &importer_group, NULL);
	g_assert_nonnull(importer_model);
	g_assert_nonnull(importer_group);

	/* --- Initial state: "small" is selected via id-2 (Large is not). --- */
	check_source_state(action_group, "small");
	check_importer_radio(importer_group, "id-1", FALSE);
	check_importer_radio(importer_group, "id-2", TRUE);

	/* --- Activate id-1 from the importer side. ---
	 * This must propagate to the source "size" action AND flip id-2 to
	 * unselected in the same go. Otherwise the two radios desync and
	 * both end up selected/unselected together. */
	g_action_group_activate_action(importer_group,
	                               "id-1",
	                               g_variant_new_string(DBUS_MENU_ACTION_RADIO_SELECTED));
	test_wait();

	check_source_state(action_group, "large");
	check_importer_radio(importer_group, "id-1", TRUE);
	check_importer_radio(importer_group, "id-2", FALSE);

	/* --- Now activate id-2 from the importer side. --- */
	g_action_group_activate_action(importer_group,
	                               "id-2",
	                               g_variant_new_string(DBUS_MENU_ACTION_RADIO_SELECTED));
	test_wait();

	check_source_state(action_group, "small");
	check_importer_radio(importer_group, "id-1", FALSE);
	check_importer_radio(importer_group, "id-2", TRUE);

	g_object_unref(importer_group);
	g_object_unref(importer_model);
	g_object_unref(size_action);
	g_object_unref(action_group);
	fixture_teardown(&f);
	g_object_unref(section);
}