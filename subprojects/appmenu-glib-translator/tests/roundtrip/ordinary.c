/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"

static void plain_action_activate(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	(void)action;
	(void)parameter;
	int *activations = user_data;
	(*activations)++;
}

/* The importer synthesizes one stateless action per plain menu item,
 * named "id-<n>" after the DBusMenu item id, just like for checkboxes
 * but without any boolean state. Pressing it must do nothing but fire
 * "activate" on the source action. */
static void check_importer_action(GActionGroup *group, const char *id)
{
	g_assert_true(g_action_group_has_action(group, id));
	g_assert_true(g_action_group_get_action_enabled(group, id));

	const GVariantType *parameter_type = g_action_group_get_action_parameter_type(group, id);
	g_assert_null(parameter_type);

	GVariant *state = g_action_group_get_action_state(group, id);
	g_assert_null(state);
}

G_GNUC_INTERNAL void test_func(void)
{
	GMenu *root    = g_menu_new();
	GMenu *section = g_menu_new();

	GMenuItem *activate = g_menu_item_new("Activate", "app.activate");
	g_menu_append_item(section, activate);
	g_object_unref(activate);

	g_menu_append_section(root, NULL, G_MENU_MODEL(section));

	GSimpleActionGroup *action_group = g_simple_action_group_new();

	/* Plain stateless action with no parameter type: activating it is a
	 * simple "button press", there is no state to toggle or mirror. */
	int activations = 0;
	GSimpleAction *activate_action = g_simple_action_new("activate", NULL);
	g_signal_connect(activate_action, "activate", G_CALLBACK(plain_action_activate), &activations);
	g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(activate_action));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root), action_group);

	/* Grab the importer's model AND its synthesized action group. */
	GMenuModel *importer_model   = NULL;
	GActionGroup *importer_group = NULL;
	g_object_get(f.importer, "model", &importer_model, "action-group", &importer_group, NULL);
	g_assert_nonnull(importer_model);
	g_assert_nonnull(importer_group);
	g_autofree char* name = extract_bare_action(f.target, "Activate");

	/* --- Initial state: the item's action is present on both sides,
	 * stateless, enabled, and has never been activated. --- */
	check_importer_action(importer_group, name);
	g_assert_cmpint(activations, ==, 0);

	/* --- Activate the importer's synthesized action without a parameter:
	 * this is the "user clicked the menu item" case, and the activation
	 * must propagate back to the source action and fire its handler. --- */
	g_action_group_activate_action(importer_group, name, NULL);
	test_wait();

	check_importer_action(importer_group, name);
	g_assert_cmpint(activations, ==, 1);

	/* --- And once more through the importer: a stateless action must
	 * fire on every activation, not only on the first one. --- */
	g_action_group_activate_action(importer_group, name, NULL);
	test_wait();

	check_importer_action(importer_group, name);
	g_assert_cmpint(activations, ==, 2);

	/* --- Activate the source action directly as well: a stateless action
	 * has no state for the importer to mirror, but the handler must still
	 * fire and the importer's action must stay intact. --- */
	g_action_group_activate_action(G_ACTION_GROUP(action_group), "activate", NULL);
	test_wait();

	check_importer_action(importer_group, name);
	g_assert_cmpint(activations, ==, 3);

	g_object_unref(importer_group);
	g_object_unref(importer_model);
	g_object_unref(activate_action);
	g_object_unref(action_group);
	fixture_teardown(&f);
	g_object_unref(section);
} 