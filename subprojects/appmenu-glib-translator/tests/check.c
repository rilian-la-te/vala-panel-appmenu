/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"
#include "definitions.h"

static void checkbox_state_change(GSimpleAction *action, GVariant *value, gpointer user_data)
{
	(void)user_data;
	g_simple_action_set_state(action, value);
}

static void check_source_state(GSimpleActionGroup *group, gboolean expected)
{
	GVariant *state = g_action_group_get_action_state(G_ACTION_GROUP(group), "check");
	g_assert_nonnull(state);
	g_assert_true(g_variant_is_of_type(state, G_VARIANT_TYPE_BOOLEAN));
	g_assert_cmpint(g_variant_get_boolean(state), ==, expected);
	g_variant_unref(state);
}

/* The importer synthesizes one boolean-stateful action per checkbox item,
 * named "id-<n>" after the DBusMenu item id. The boolean state is the
 * checked state of the item. */
static void check_importer_checkbox(GActionGroup *group, const char *id, gboolean checked)
{
	GVariant *state = g_action_group_get_action_state(group, id);
	g_assert_nonnull(state);
	g_assert_true(g_variant_is_of_type(state, G_VARIANT_TYPE_BOOLEAN));
	g_assert_cmpint(g_variant_get_boolean(state), ==, checked);
	g_variant_unref(state);
}

G_GNUC_INTERNAL void test_func(void)
{
	GMenu *root    = g_menu_new();
	GMenu *section = g_menu_new();

	GMenuItem *check = g_menu_item_new("Check", "app.check");
	g_menu_append_item(section, check);
	g_object_unref(check);

	g_menu_append_section(root, NULL, G_MENU_MODEL(section));

	GSimpleActionGroup *action_group = g_simple_action_group_new();

	/* Boolean stateful action with no parameter type: activating it without
	 * a parameter toggles the state (FALSE <-> TRUE). */
	GSimpleAction *check_action = g_simple_action_new_stateful("check",
	                                                           NULL,
	                                                           g_variant_new_boolean(FALSE));
	g_signal_connect(check_action, "change-state", G_CALLBACK(checkbox_state_change), NULL);
	g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(check_action));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root), action_group);

	/* Grab the importer's model AND its synthesized action group. */
	GMenuModel *importer_model   = NULL;
	GActionGroup *importer_group = NULL;
	g_object_get(f.importer, "model", &importer_model, "action-group", &importer_group, NULL);
	g_assert_nonnull(importer_model);
	g_assert_nonnull(importer_group);

	/* --- Initial state: the checkbox is unchecked. --- */
	check_source_state(action_group, FALSE);
	check_importer_checkbox(importer_group, "id-1", FALSE);

	/* --- Activate the source action without a parameter: it toggles
	 * FALSE -> TRUE. The importer's synthesized checkbox action must
	 * follow and become TRUE as well. --- */
	g_action_group_activate_action(G_ACTION_GROUP(action_group), "check", NULL);
	test_wait();

	check_source_state(action_group, TRUE);
	check_importer_checkbox(importer_group, "id-1", TRUE);

	/* --- Activate the importer's synthesized action without a parameter:
	 * it toggles TRUE -> FALSE. This must propagate back to the source
	 * action and flip it to FALSE too. --- */
	g_action_group_activate_action(importer_group, "id-1", NULL);
	test_wait();

	check_source_state(action_group, FALSE);
	check_importer_checkbox(importer_group, "id-1", FALSE);

	/* --- And once more through the importer, to cover FALSE -> TRUE. --- */
	g_action_group_activate_action(importer_group, "id-1", NULL);
	test_wait();

	check_source_state(action_group, TRUE);
	check_importer_checkbox(importer_group, "id-1", TRUE);

	g_object_unref(importer_group);
	g_object_unref(importer_model);
	g_object_unref(check_action);
	g_object_unref(action_group);
	fixture_teardown(&f);
	g_object_unref(section);
}