/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: a titled marker leading the menu (the first child, before any real item)
 * titles the always-living first section instead of opening a new one. Even though that
 * section can never be removed, it should still carry the title when a title is genuinely
 * the first thing served. Shared primitives live in invasive-fixture.h. */

#include "invasive-fixture.h"

static GVariant *layout_leading_title(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_kde_title(&layout, 10, "first section title");
	invasive_add_label(&layout, 12, "only item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_leading_title(GString *errors)
{
	static const char *const only_item[] = { "only item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_leading_title(), "org.glib.MenuTest.KdeLeading");

	invasive_dump("kde-title leading", s.target);

	/* The leading marker does not open a section; it labels the existing first one. */
	invasive_check_n_sections(s.target, errors, "leading", 1);
	invasive_check_section_label(s.target, 0, errors, "leading section 0", "first section title");
	invasive_check_internal_label(s.model, 0, errors, "leading section 0", "first section title");
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "leading section 0", 1, only_item);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_leading_title);
}
