/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"
#include "invasive-fixture.h"

static GVariant *build_frozen_layout(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);

	for (u_int32_t id = 10; id <= 20; id++)
		invasive_add_separator(&layout, id, NULL);

	/* A labelled x-kde-title marker: the only marker that can carry a label through to
	 * the model, so it titles the section it opens. */
	invasive_add_kde_title(&layout, 1, "Titled separator");

	for (u_int32_t id = 5; id <= 8; id++)
		invasive_add_separator(&layout, id, NULL);

	invasive_add_label(&layout, 3, "First item");
	invasive_add_label(&layout, 4, "Second item");

	invasive_add_kde_title(&layout, 2, "Second titled separator");

	invasive_add_label(&layout, 7, "Third item");

	for (u_int32_t id = 21; id <= 25; id++)
		invasive_add_separator(&layout, id, NULL);

	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_separator_run(GString *errors)
{
	static const char *const first_section_items[]  = { "First item", "Second item" };
	static const char *const second_section_items[] = { "Third item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, build_frozen_layout(), TEST_BUS_NAME);

	invasive_dump("separator-run", s.target);
	invasive_check_n_sections(s.target, errors, "separator-run", 2);

	invasive_check_section_label(s.target, 0, errors, "section 0", "Titled separator");
	invasive_check_section_label(s.target, 1, errors, "section 1", "Second titled separator");

	invasive_check_section_items(invasive_section_model(s.target, 0),
	                             errors,
	                             "section 0",
	                             2,
	                             first_section_items);
	invasive_check_section_items(invasive_section_model(s.target, 1),
	                             errors,
	                             "section 1",
	                             1,
	                             second_section_items);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_separator_run);
}
