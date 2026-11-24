/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: a titled section holding two items keeps its title across repeated identical
 * re-serves (a marker must not be second-guessed by later items). Shared primitives live in
 * invasive-fixture.h. */

#include "invasive-fixture.h"

/* A titled x-kde-title section holding two items. */
static GVariant *layout_two_items(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_label(&layout, 10, "first item");
	invasive_add_kde_title(&layout, 11, "kde title");
	invasive_add_label(&layout, 12, "second item");
	invasive_add_label(&layout, 13, "third item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_kde_title_two_items(GString *errors)
{
	static const char *const one_item[]    = { "first item" };
	static const char *const other_items[] = { "second item", "third item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_two_items(), "org.glib.MenuTest.KdeTwo");

	invasive_dump("kde-title two items", s.target);
	invasive_check_n_sections(s.target, errors, "kde-title two items", 2);
	invasive_check_section_label(s.target, 0, errors, "two section 0", NULL);
	invasive_check_internal_label(s.model, 0, errors, "two section 0", NULL);
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "two section 0", 1, one_item);

	invasive_check_section_label(s.target, 1, errors, "two section 1", "kde title");
	invasive_check_internal_label(s.model, 1, errors, "two section 1", "kde title");
	invasive_check_section_items(invasive_section_model(s.target, 1), errors, "two section 1", 2, other_items);

	/* Re-serve the identical layout. The mock server's setter overwrites its layout
	 * and emits LayoutUpdated; the importer should survive an unchanged re-parse. */
	invasive_scenario_serve(&s, layout_two_items());
	invasive_scenario_serve(&s, layout_two_items());
	invasive_dump("kde-title two items re-served", s.target);

	invasive_check_n_sections(s.target, errors, "two re-served", 2);
	invasive_check_section_label(s.target, 1, errors, "two re-served section 1", "kde title");
	invasive_check_internal_label(s.model, 1, errors, "two re-served section 1", "kde title");

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_kde_title_two_items);
}
