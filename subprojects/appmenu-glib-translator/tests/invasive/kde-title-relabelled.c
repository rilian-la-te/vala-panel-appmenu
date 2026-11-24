/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* x-kde-title: re-serving the marker under a different id and title keeps the section
 * item it already had while moving the new title onto it. Shared primitives live in
 * invasive-fixture.h. */

#include "invasive-fixture.h"

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

/* The layout above with its marker re-served under a different id and title. */
static GVariant *layout_kde_title_relabelled(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);
	invasive_add_label(&layout, 10, "first item");
	invasive_add_kde_title(&layout, 20, "Relabelled kde separator");
	invasive_add_label(&layout, 12, "second item");
	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_title_follows_relabelled(GString *errors)
{
	static const char *const one_item[]   = { "first item" };
	static const char *const other_item[] = { "second item" };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, layout_kde_title(), "org.glib.MenuTest.KdeRelabel");

	u_int32_t marker_id = invasive_section_item_id(s.model, 1);
	if (marker_id != 11)
		g_string_append_printf(errors, "relabel: section 1 is held by id %u, expected 11\n", marker_id);

	invasive_scenario_serve(&s, layout_kde_title_relabelled());
	invasive_dump("kde-title relabelled", s.target);

	u_int32_t new_marker_id = invasive_section_item_id(s.model, 1);
	if (new_marker_id != marker_id)
		g_string_append_printf(errors, "relabel: the section item moved from id %u to %u\n", marker_id, new_marker_id);

	invasive_check_n_sections(s.target, errors, "relabel", 2);
	invasive_check_section_label(s.target, 1, errors, "relabel section 1", "Relabelled kde separator");
	invasive_check_internal_label(s.model, 1, errors, "relabel section 1", "Relabelled kde separator");
	invasive_check_section_items(invasive_section_model(s.target, 0), errors, "relabel section 0", 1, one_item);
	invasive_check_section_items(invasive_section_model(s.target, 1), errors, "relabel section 1", 1, other_item);

	invasive_scenario_stop(&s);
}

G_GNUC_INTERNAL void test_func(void)
{
	invasive_scenario_run(scenario_title_follows_relabelled);
}
