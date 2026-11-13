/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

const char *test_path = "/e2e/checkbox/importer-source";

void test_func(void)
{
	E2E e;
	e2e_setup(&e);

	char *act = imported_action_for_label(e.imported_model, "Auto-save");
	CHECK(&e, "checkbox/i2s/found", act != NULL);
	if (!act)
	{
		e2e_teardown(&e);
		return;
	}

	/* 1) активируем импортёр → source должен переключиться FALSE→TRUE */
	g_action_group_activate_action(e.imported_actions, act, NULL);
	settle(250);

	GVariant *st = g_action_group_get_action_state(G_ACTION_GROUP(e.actions), "autosave");
	CHECK(&e, "checkbox/i2s/src-true", st && g_variant_get_boolean(st));
	if (st)
		g_variant_unref(st);

	/* 2) и после roundtrip импортированный action тоже TRUE */
	st = g_action_group_get_action_state(e.imported_actions, act);
	CHECK(&e,
	      "checkbox/i2s/imp-true",
	      st && g_variant_is_of_type(st, G_VARIANT_TYPE_BOOLEAN) && g_variant_get_boolean(st));
	if (st)
		g_variant_unref(st);

	/* 3) второй клик → TRUE→FALSE симметрично */
	g_action_group_activate_action(e.imported_actions, act, NULL);
	settle(250);

	st = g_action_group_get_action_state(G_ACTION_GROUP(e.actions), "autosave");
	CHECK(&e, "checkbox/i2s/src-false", st && !g_variant_get_boolean(st));
	if (st)
		g_variant_unref(st);

	st = g_action_group_get_action_state(e.imported_actions, act);
	CHECK(&e, "checkbox/i2s/imp-false", st && !g_variant_get_boolean(st));
	if (st)
		g_variant_unref(st);

	g_free(act);
	e2e_teardown(&e);
}