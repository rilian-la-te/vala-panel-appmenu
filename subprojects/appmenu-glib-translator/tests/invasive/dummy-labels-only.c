/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* Drives the importer against a hand-written com.canonical.dbusmenu server that
 * has no exporter and no GMenuModel behind it: everything it serves is the one
 * layout variant handed to its constructor. See mock-server.c. The layout is built
 * with the shared fixture primitives and served through an InvasiveFixture; the
 * checks here are specific to this test - that plain labelled items reach the model
 * as exactly one section and each gets a plain stateless action on the importer's
 * own action group. */

#include "comparator.h"
#include "invasive-fixture.h"

static GVariant *build_frozen_layout(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);

	/* The separator opens the single section of this menu; the importer drops it
	 * once the items behind it are placed. */
	invasive_add_separator(&layout, 1, NULL);

	invasive_add_label(&layout, 2, "First item");
	invasive_add_label(&layout, 3, "Second item");

	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

/* --- checking what reached the importer ------------------------------------ */

/* The layout must reach the importer as exactly one section holding the two labelled
 * items - no exporter and no model on the serving side. The labels and their non-empty
 * action attributes are checked by the shared helper; what is specific here is that each
 * bare action name resolves on the importer's own action group as a plain stateless
 * action (no parameter, no state). */
static void check_actions(InvasiveFixture *s, GString *errors)
{
	static const char *const expected_items[] = {"First item", "Second item"};

	GActionGroup *group = NULL;
	g_object_get(s->importer, "action-group", &group, NULL);
	if (group == NULL)
	{
		g_string_append_printf(errors, "no action group on the importer\n");
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		/* The attribute carries the "dbusmenu." namespace prefix while the importer's
		 * action group stores the bare name, which is what extract_bare_action strips off. */
		g_autofree char *bare = extract_bare_action(s->target, expected_items[i]);
		if (bare == NULL)
		{
			g_string_append_printf(errors, "'%s': no bare action name\n", expected_items[i]);
			continue;
		}
		if (!g_action_group_has_action(group, bare))
		{
			g_string_append_printf(errors, "'%s': action '%s' is missing\n", expected_items[i], bare);
			continue;
		}
		if (!g_action_group_get_action_enabled(group, bare))
			g_string_append_printf(errors, "'%s': action '%s' is not enabled\n", expected_items[i], bare);

		/* A plain item carries no state and takes no parameter. */
		if (g_action_group_get_action_parameter_type(group, bare) != NULL)
			g_string_append_printf(errors, "'%s': action '%s' takes a parameter\n", expected_items[i], bare);
		if (g_action_group_get_action_state(group, bare) != NULL)
			g_string_append_printf(errors, "'%s': action '%s' has state\n", expected_items[i], bare);
	}

	g_object_unref(group);
}

static void scenario_dummy_labels_only(GString *errors)
{
	static const char *const section_items[] = {"First item", "Second item"};

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, build_frozen_layout(), TEST_BUS_NAME);

	invasive_dump("dummy-labels-only", s.target);

	/* A separator with no label of its own opens the single section; because nothing
	 * titled it, that section stays untitled. */
	invasive_check_n_sections(s.target, errors, "dummy-labels-only", 1);
	invasive_check_section_label(s.target, 0, errors, "dummy section 0", NULL);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "dummy section 0", 2, section_items);

	check_actions(&s, errors);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_dummy_labels_only);
}
