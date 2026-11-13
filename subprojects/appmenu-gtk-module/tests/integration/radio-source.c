/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

const char *test_path = "/e2e/radio/source-importer";

void test_func(void)
{
	E2E e;
	e2e_setup(&e);
	assert_models(&e, "radio/s2i/models");

	char *small_act = imported_action_for_label(e.imported_model, "Small");
	char *large_act = imported_action_for_label(e.imported_model, "Large");
	CHECK(&e, "radio/s2i/found-small", small_act != NULL);
	CHECK(&e, "radio/s2i/found-large", large_act != NULL);
	if (!small_act || !large_act)
	{
		g_free(small_act);
		g_free(large_act);
		e2e_teardown(&e);
		return;
	}

	CHECK(&e,
	      "radio/s2i/initial-small-selected",
	      radio_state_is(e.imported_actions, small_act, DBUS_MENU_ACTION_RADIO_SELECTED));
	CHECK(&e,
	      "radio/s2i/initial-large-unselected",
	      radio_state_is(e.imported_actions, large_act, DBUS_MENU_ACTION_RADIO_UNSELECTED));

	g_action_group_activate_action(G_ACTION_GROUP(e.actions),
	                               "size",
	                               g_variant_new_string("large"));
	settle(250);

	GVariant *st = g_action_group_get_action_state(G_ACTION_GROUP(e.actions), "size");
	CHECK(&e,
	      "radio/s2i/src-large",
	      st && g_strcmp0(g_variant_get_string(st, NULL), "large") == 0);
	if (st)
		g_variant_unref(st);

	CHECK(&e,
	      "radio/s2i/after-small-unselected",
	      radio_state_is(e.imported_actions, small_act, DBUS_MENU_ACTION_RADIO_UNSELECTED));
	CHECK(&e,
	      "radio/s2i/after-large-selected",
	      radio_state_is(e.imported_actions, large_act, DBUS_MENU_ACTION_RADIO_SELECTED));

	g_free(small_act);
	g_free(large_act);
	e2e_teardown(&e);
}