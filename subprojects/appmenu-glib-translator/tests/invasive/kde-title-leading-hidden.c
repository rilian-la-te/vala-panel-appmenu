/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: a hidden titled marker leading the menu is fully ignored - it neither
 * opens a section nor labels the always-living first one, so that section stays
 * unlabeled and keeps its item. Hidden beats "leading". Shared primitives live in
 * invasive-fixture.h. */

#include "invasive-fixture.h"

static GVariant *layout_leading_hidden_title(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_hidden_kde_title(&layout, 10, "hidden title");
	invasive_add_label(&layout, 12, "only item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_leading_hidden_title(GString *errors)
{
	static const char *const only_item[] = { "only item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_leading_hidden_title(), "org.glib.MenuTest.KdeLeadingHidden");

	invasive_dump("kde-title leading hidden", s.target);

	/* The hidden leading marker is ignored entirely: the first section survives but is
	 * not labeled. */
	invasive_check_n_sections(s.target, errors, "leading-hidden", 1);
	invasive_check_section_label(s.target, 0, errors, "leading-hidden section 0", NULL);
	invasive_check_internal_label(s.model, 0, errors, "leading-hidden section 0", NULL);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "leading-hidden section 0", 1, only_item);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_leading_hidden_title);
}
