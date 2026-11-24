/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: two titled markers run back-to-back before one item. The first marker
 * opens no section (the current one is still empty) and the second opens it, so only the
 * LAST non-null title reaches the section - "run title two" wins over "run title one".
 * Shared primitives live in invasive-fixture.h. */

#include "invasive-fixture.h"

static GVariant *layout_kde_title_run(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_label(&layout, 10, "first item");
	invasive_add_kde_title(&layout, 11, "run title one");
	invasive_add_kde_title(&layout, 12, "run title two");
	invasive_add_label(&layout, 13, "second item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_kde_title_run(GString *errors)
{
	static const char *const one_item[]   = { "first item" };
	static const char *const other_item[] = { "second item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_kde_title_run(), "org.glib.MenuTest.KdeRun");

	invasive_dump("kde-title run", s.target);
	invasive_check_n_sections(s.target, errors, "kde-title run", 2);

	/* The always-living first section holds the first item and stays unlabeled. */
	invasive_check_section_label(s.target, 0, errors, "run section 0", NULL);
	invasive_check_internal_label(s.model, 0, errors, "run section 0", NULL);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "run section 0", 1, one_item);

	/* The second section is titled by the last non-null marker to precede it. */
	invasive_check_section_label(s.target, 1, errors, "run section 1", "run title two");
	invasive_check_internal_label(s.model, 1, errors, "run section 1", "run title two");
	invasive_check_section_items(invasive_section_model(s.target, 1), errors, "run section 1", 1, other_item);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_kde_title_run);
}
