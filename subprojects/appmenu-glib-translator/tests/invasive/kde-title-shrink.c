/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: dropping the marker from a two-section layout shrinks it back to one
 * (always-living) section holding all remaining items, with no title left on it. The
 * first section can never be killed - it belongs to the synthetic item the importer
 * builds on construction - so removing the only marker must fold its items back into
 * that first section rather than emptying or deleting it. Shared primitives live in
 * invasive-fixture.h. */

#include "invasive-fixture.h"

/* Two sections: an empty synthetic first one, then a titled x-kde-title section. */
static GVariant *layout_kde_title(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_label(&layout, 10, "first item");
	invasive_add_kde_title(&layout, 11, "Kde titled separator");
	invasive_add_label(&layout, 12, "second item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

/* The same items with no marker at all: one plain section. */
static GVariant *layout_one_section(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_label(&layout, 10, "first item");
	invasive_add_label(&layout, 12, "second item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_shrink_sections(GString *errors)
{
	static const char *const one_item[]   = { "first item" };
	static const char *const both_items[] = { "first item", "second item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_kde_title(), "org.glib.MenuTest.KdeShrink");

	invasive_check_n_sections(s.target, errors, "shrink before", 2);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "shrink before section 0", 1, one_item);

	invasive_scenario_serve(&s, layout_one_section());
	invasive_dump("kde-title shrink", s.target);

	/* The second section is gone; the surviving first one now holds both items and
	 * carries none of the title the dropped marker used to hold. */
	invasive_check_n_sections(s.target, errors, "shrink after", 1);
	invasive_check_section_label(s.target, 0, errors, "shrink after section 0", NULL);
	invasive_check_internal_label(s.model, 0, errors, "shrink after section 0", NULL);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "shrink after section 0", 2, both_items);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_shrink_sections);
}
