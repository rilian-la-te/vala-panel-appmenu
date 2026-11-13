/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

void test_func(void)
{
	E2E e;
	e2e_setup(&e);
	assert_models(&e, "checkbox/s2i/models");

	char *act = imported_action_for_label(e.imported_model, "Auto-save");
	CHECK(&e, "checkbox/s2i/found", act != NULL);
	if (!act)
	{
		e2e_teardown(&e);
		return;
	}

	GVariant *st = g_action_group_get_action_state(e.imported_actions, act);
	CHECK(&e,
	      "checkbox/s2i/initial",
	      st && g_variant_is_of_type(st, G_VARIANT_TYPE_BOOLEAN) &&
	          g_variant_get_boolean(st) == FALSE);
	if (st)
		g_variant_unref(st);

	g_action_group_activate_action(G_ACTION_GROUP(e.actions), "autosave", NULL);
	settle(300);

	st = g_action_group_get_action_state(G_ACTION_GROUP(e.actions), "autosave");
	CHECK(&e, "checkbox/s2i/src-true", st && g_variant_get_boolean(st));
	if (st)
		g_variant_unref(st);

	st = g_action_group_get_action_state(e.imported_actions, act);
	CHECK(&e, "checkbox/s2i/imp-true", st && g_variant_get_boolean(st));
	if (st)
		g_variant_unref(st);

	g_free(act);
	e2e_teardown(&e);
}