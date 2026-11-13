/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

const char *test_path = "/e2e/regular/action-fires";

void test_func(void)
{
	E2E e;
	e2e_setup(&e);

	char *act = imported_action_for_label(e.imported_model, "New");
	CHECK(&e, "reg/found", act != NULL);
	if (!act)
	{
		e2e_teardown(&e);
		return;
	}

	if (!g_action_group_has_action(e.imported_actions, act))
	{
		g_free(act);
		e2e_teardown(&e);
		g_test_skip("importer did not expose a synthesized action for 'New'");
		return;
	}

	CHECK(&e, "reg/zero-fires", g_new_fires == 0);
	g_action_group_activate_action(e.imported_actions, act, NULL);
	settle(250);
	CHECK(&e, "reg/one-fire", g_new_fires == 1);

	g_free(act);
	e2e_teardown(&e);
}