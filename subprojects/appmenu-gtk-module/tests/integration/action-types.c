/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

static void count_action_types(GActionGroup *g,
                               int *stateless,
                               int *boolean_stateful)
{
	*stateless        = 0;
	*boolean_stateful = 0;

	gchar **names = g_action_group_list_actions(g);
	for (guint i = 0; names && names[i]; i++)
	{
		GVariant *st = g_action_group_get_action_state(g, names[i]);
		if (!st)
		{
			(*stateless)++;
			continue;
		}
		if (g_variant_is_of_type(st, G_VARIANT_TYPE_BOOLEAN))
			(*boolean_stateful)++;
		g_variant_unref(st);
	}
	g_strfreev(names);
}

void test_func(void)
{
	E2E e;
	e2e_setup(&e);

	int src_stateless = 0, src_bool = 0;
	int imp_stateless = 0, imp_bool = 0;

	count_action_types(G_ACTION_GROUP(e.actions), &src_stateless, &src_bool);
	count_action_types(e.imported_actions, &imp_stateless, &imp_bool);

	CHECK(&e,
	      "types/stateless-count",
	      src_stateless !=0 && imp_stateless !=0);

	e2e_teardown(&e);
}