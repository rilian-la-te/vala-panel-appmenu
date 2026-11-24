/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: a leading titled marker first titles the always-living section; re-serving
 * the same menu with an untitled first item (no marker) must drop that stale title from
 * the first section. The section survives, but its label does not. Shared primitives live
 * in invasive-fixture.h. */

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

/* The same menu re-served with no marker: the first child is a plain untitled item. */
static GVariant *layout_leading_title_cleared(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_label(&layout, 12, "only item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_leading_title_cleared(GString *errors)
{
	static const char *const only_item[] = { "only item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_leading_title(), "org.glib.MenuTest.KdeLeadingClr");

	/* First: the leading marker titles the first section. */
	invasive_check_n_sections(s.target, errors, "leading-cleared before", 1);
	invasive_check_section_label(s.target, 0, errors, "leading-cleared before section 0", "first section title");

	invasive_scenario_serve(&s, layout_leading_title_cleared());
	invasive_dump("kde-title leading cleared", s.target);

	/* Now the first child is untitled, so the stale title on the surviving first
	 * section must be gone. */
	invasive_check_n_sections(s.target, errors, "leading-cleared after", 1);
	invasive_check_section_label(s.target, 0, errors, "leading-cleared after section 0", NULL);
	invasive_check_internal_label(s.model, 0, errors, "leading-cleared after section 0", NULL);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "leading-cleared after section 0", 1, only_item);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_leading_title_cleared);
}
