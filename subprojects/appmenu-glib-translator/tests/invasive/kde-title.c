/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: a titled marker's title lands on the section it opens (the second one),
 * while the always-living first section stays empty and unlabeled; an identical
 * re-serve leaves that unchanged. Shared primitives live in invasive-fixture.h. */

#include "invasive-fixture.h"

/* Two sections: an empty synthetic first one, then a titled x-kde-title section
 * holding one item. */
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

static void scenario_kde_title(GString *errors)
{
	static const char *const one_item[]   = { "first item" };
	static const char *const other_item[] = { "second item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_kde_title(), "org.glib.MenuTest.KdeTitle");

	invasive_dump("kde-title", s.target);
	invasive_check_n_sections(s.target, errors, "kde-title", 2);
	invasive_check_section_label(s.target, 0, errors, "kde-title section 0", NULL);
	invasive_check_internal_label(s.model, 0, errors, "kde-title section 0", NULL);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "kde-title section 0", 1, one_item);

	invasive_check_section_label(s.target, 1, errors, "kde-title section 1", "Kde titled separator");
	invasive_check_internal_label(s.model, 1, errors, "kde-title section 1", "Kde titled separator");
	invasive_check_section_items(invasive_section_model(s.target, 1), errors, "kde-title section 1", 1, other_item);

	/* Minimal re-serve probe: hand back the very same layout unchanged. */
	invasive_scenario_serve(&s, layout_kde_title());
	invasive_dump("kde-title re-served", s.target);

	invasive_check_n_sections(s.target, errors, "kde-title re-served", 2);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_kde_title);
}
