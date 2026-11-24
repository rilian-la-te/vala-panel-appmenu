/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: a hidden titled marker between items is toggled off, so it opens no
 * section and its label goes nowhere - the single (always-living) first section stays
 * unlabeled and keeps both items. Shared primitives live in invasive-fixture.h. */

#include "invasive-fixture.h"

static GVariant *layout_hidden_kde_title(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_label(&layout, 10, "first item");
	invasive_add_hidden_kde_title(&layout, 11, "hidden title");
	invasive_add_label(&layout, 12, "second item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_hidden_kde_title(GString *errors)
{
	static const char *const both_items[] = { "first item", "second item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_hidden_kde_title(), "org.glib.MenuTest.KdeHidden");

	invasive_dump("kde-title hidden", s.target);

	/* The hidden marker opens no section: one section holds both items. */
	invasive_check_n_sections(s.target, errors, "hidden", 1);
	invasive_check_section_label(s.target, 0, errors, "hidden section 0", NULL);
	invasive_check_internal_label(s.model, 0, errors, "hidden section 0", NULL);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "hidden section 0", 2, both_items);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_hidden_kde_title);
}
