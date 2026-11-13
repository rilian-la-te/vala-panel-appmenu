/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

const char *test_path = "/e2e/radio/importer-source";

void test_func(void)
{
	E2E e;
	e2e_setup(&e);

	char *small_act = imported_action_for_label(e.imported_model, "Small");
	char *large_act = imported_action_for_label(e.imported_model, "Large");
	CHECK(&e, "radio/i2s/found-small", small_act != NULL);
	CHECK(&e, "radio/i2s/found-large", large_act != NULL);
	if (!small_act || !large_act)
	{
		g_free(small_act);
		g_free(large_act);
		e2e_teardown(&e);
		return;
	}

	CHECK(&e,
	      "radio/i2s/initial-large-unselected",
	      radio_state_is(e.imported_actions, large_act, DBUS_MENU_ACTION_RADIO_UNSELECTED));

	g_action_group_activate_action(e.imported_actions,
	                               large_act,
	                               g_variant_new_string(DBUS_MENU_ACTION_RADIO_SELECTED));
	settle(300);

	/* 1) source переключился на large */
	GVariant *st = g_action_group_get_action_state(G_ACTION_GROUP(e.actions), "size");
	CHECK(&e,
	      "radio/i2s/src-large",
	      st && g_strcmp0(g_variant_get_string(st, NULL), "large") == 0);
	if (st)
		g_variant_unref(st);

	/* 2) roundtrip: Large стал selected, Small — unselected */
	CHECK(&e,
	      "radio/i2s/after-large-selected",
	      radio_state_is(e.imported_actions, large_act, DBUS_MENU_ACTION_RADIO_SELECTED));
	CHECK(&e,
	      "radio/i2s/after-small-unselected",
	      radio_state_is(e.imported_actions, small_act, DBUS_MENU_ACTION_RADIO_UNSELECTED));

	g_free(small_act);
	g_free(large_act);
	e2e_teardown(&e);
}