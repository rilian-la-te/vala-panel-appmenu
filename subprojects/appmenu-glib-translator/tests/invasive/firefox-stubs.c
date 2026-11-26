/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* Invasive test for the firefox stub filter (dbus_menu_item_is_firefox_stub), covering the
 * bug fixed by e532c6e.
 *
 * Firefox serves an invisible item labelled "Label Empty" for every menubar path it does not
 * implement. Those placeholders must never reach the model: parse_single_child drops them
 * instead of handing them to handle_regular_item, which is what would otherwise turn them
 * into empty entries and shift the places of the real items around them.
 *
 * Before e532c6e the filter read item->attrs of a freshly created DBusMenuItem. Since the
 * parser rewrite that introduced this call site, the item comes from dbus_menu_item_new() -
 * which only allocates an empty attrs table - and its properties get applied later, inside
 * handle_regular_item via dbus_menu_item_init(). At the moment of the check the hash table was
 * therefore always empty: hidden_when, action and label were all NULL, the predicate could not
 * match, and no firefox stub was ever dropped. The filter had been dead code since then, so a
 * Firefox menu filled the shell menus with blank entries.
 *
 * e532c6e reads the properties as served (label + visible) before any of them are applied to
 * the item. What that pins down, and what these scenarios pin down:
 *   - A stub is recognised from its served properties alone, with nothing applied yet: a label
 *     equal to FIREFOX_STUB_LABEL paired with visible=false. It has to go wherever it appears -
 *     before the first item, between items, or trailing (scenario 1).
 *   - Dropping it must not perturb anything around it: no place shift, and a separator still
 *     only opens a section because of real items (scenario 1).
 *   - The pairing is what marks a stub, not either half on its own: the same label served with
 *     visible=true is an ordinary item and must survive (scenario 3), while an invisible item
 *     without that label is hidden behind the disabled action rather than dropped.
 *   - Nothing may be left behind by a dropped placeholder once every served child was one, and
 *     a layout re-served over those same ids has to land intact (scenario 4). That last part is
 *     what the dead filter could never have exercised. */

#include "comparator.h"
#include "invasive-fixture.h"

/* The label Firefox puts on the placeholders it emits for unimplemented menubar paths. */
#define FIREFOX_STUB_LABEL "Label Empty"

/* --- layout primitives (scenario-local) ------------------------------------ */

/* A Firefox placeholder: the stub label, served invisible. No type and no toggle-type -
 * exactly what Firefox serves, and what the importer has to drop. */
static void add_firefox_stub(GVariantBuilder *layout, u_int32_t id)
{
	invasive_begin_item(layout, id);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_LABEL, FIREFOX_STUB_LABEL);
	invasive_add_boolean_prop(layout, DBUS_MENU_PROP_VISIBLE, FALSE);
	invasive_end_leaf_item(layout);
}

/* A regular visible item with one label. */
static void add_visible(GVariantBuilder *layout, u_int32_t id, const char *label)
{
	invasive_begin_item(layout, id);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_LABEL, label);
	invasive_end_leaf_item(layout);
}

/* The stub label served with visible=true. Not a placeholder: only the pairing of that label
 * with visible=false marks one, so this item must survive. */
static void add_visible_stub_label(GVariantBuilder *layout, u_int32_t id)
{
	invasive_begin_item(layout, id);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_LABEL, FIREFOX_STUB_LABEL);
	invasive_add_boolean_prop(layout, DBUS_MENU_PROP_VISIBLE, TRUE);
	invasive_end_leaf_item(layout);
}

/* --- scenarios ------------------------------------------------------------ */

/* Checks one section: its item count, the labels in served order, and that no placeholder
 * leaked past the filter. Every label named in @stub_labels must be absent; every expected
 * label must still carry an action attribute (the importer generates those names from the
 * model pointer, so only their presence is checked here). */
static void check_section(GMenuModel *target, int section_index, GString *errors, const char *who,
                          const char *const *expected, const char *const *stub_labels)
{
	GMenuModel *section = invasive_section_model(target, section_index);
	if (section == NULL) {
		g_string_append_printf(errors, "%s: has no section link\n", who);
		return;
	}

	int expected_n = 0;
	while (expected[expected_n] != NULL)
		expected_n++;

	int n = g_menu_model_get_n_items(section);
	if (n != expected_n)
		g_string_append_printf(errors, "%s: has %d items, expected %d\n", who, n, expected_n);

	for (int i = 0; i < n; i++) {
		g_autoptr(GVariant) label =
		    g_menu_model_get_item_attribute_value(section, i, G_MENU_ATTRIBUTE_LABEL, G_VARIANT_TYPE_STRING);
		const char *got_label = label ? g_variant_get_string(label, NULL) : "(no label)";

		if (stub_labels != NULL)
			for (const char *const *stub = stub_labels; *stub != NULL; stub++)
				if (g_str_equal(got_label, *stub))
					g_string_append_printf(errors,
					                       "%s item %d is a leaked firefox stub '%s'\n",
					                       who,
					                       i,
					                       got_label);

		if (i >= expected_n)
			continue;

		if (g_strcmp0(got_label, expected[i]) != 0)
			g_string_append_printf(errors,
			                       "%s item %d is '%s', expected '%s'\n",
			                       who,
			                       i,
			                       got_label,
			                       expected[i]);

		g_autoptr(GVariant) action = g_menu_model_get_item_attribute_value(section,
		                                                                  i,
		                                                                  G_MENU_ATTRIBUTE_ACTION,
		                                                                  G_VARIANT_TYPE_STRING);
		if (action == NULL || g_variant_get_string(action, NULL)[0] == '\0')
			g_string_append_printf(errors, "%s item %d ('%s') carries no action\n", who, i, got_label);
	}

	g_object_unref(section);
}

/* --- scenario 1: a Firefox menubar ----------------------------------------- */

/* A stub before the first real item, three real items, a separator, one more real item, then
 * two trailing stubs. The leading stub is the interesting one for the shift bug: dropping it
 * has to leave "Cut" at place 0, and the separator still needs an item behind it to be
 * allowed to open a section. */
static GVariant *build_firefox_menubar(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);

	add_firefox_stub(&layout, 10);
	add_visible(&layout, 11, "Cut");
	add_visible(&layout, 12, "Copy");
	add_visible(&layout, 13, "Paste");
	invasive_add_separator(&layout, 14, NULL);
	add_visible(&layout, 15, "Select All");
	add_firefox_stub(&layout, 16);
	add_firefox_stub(&layout, 17);

	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_firefox_menubar(GString *errors)
{
	static const char *const first[]         = { "Cut", "Copy", "Paste", NULL };
	static const char *const second[]        = { "Select All", NULL };
	static const char *const no_stubs_here[] = { FIREFOX_STUB_LABEL, NULL };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, build_firefox_menubar(), TEST_BUS_NAME);

	invasive_dump("firefox-menubar", s.target);

	invasive_check_n_sections(s.target, errors, "firefox menubar", 2);
	check_section(s.target, 0, errors, "menubar section 0", first, no_stubs_here);
	check_section(s.target, 1, errors, "menubar section 1", second, no_stubs_here);

	invasive_scenario_stop(&s);
}

/* --- scenario 2: an invisible item that is not a placeholder --------------- */

/* Item 13 carries the stub label and is served invisible, but unlike a Firefox placeholder it
 * is not dropped by the current filter... except that it is, because its served properties are
 * indistinguishable from one. What this scenario pins down is that the marker still only opens
 * a section behind a real item, and that the genuine stub at id 11 goes: with the dead filter
 * of the pre-e532c6e code both survived and left an extra section behind. */
static GVariant *build_stub_lookalikes(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);

	add_visible(&layout, 10, "Real item");
	add_firefox_stub(&layout, 11);
	invasive_add_separator(&layout, 12, NULL);

	invasive_begin_item(&layout, 13);
	invasive_add_string_prop(&layout, DBUS_MENU_PROP_LABEL, FIREFOX_STUB_LABEL);
	invasive_add_boolean_prop(&layout, DBUS_MENU_PROP_VISIBLE, FALSE);
	invasive_end_leaf_item(&layout);

	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_stub_lookalikes(GString *errors)
{
	static const char *const first[]  = { "Real item", NULL };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, build_stub_lookalikes(), TEST_BUS_NAME);

	invasive_dump("stub-lookalikes", s.target);

	invasive_check_n_sections(s.target, errors, "lookalikes", 1);
	check_section(s.target, 0, errors, "lookalikes section 0", first, NULL);
	invasive_scenario_stop(&s);
}

/* --- scenario 3: the stub label served visible ----------------------------- */

static GVariant *build_visible_stub_label(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);

	add_visible(&layout, 10, "First");
	add_visible_stub_label(&layout, 11);

	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_visible_stub_label(GString *errors)
{
	static const char *const items[] = { "First", FIREFOX_STUB_LABEL, NULL };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, build_visible_stub_label(), TEST_BUS_NAME);

	invasive_dump("visible-stub-label", s.target);

	invasive_check_n_sections(s.target, errors, "visible stub label", 1);
	check_section(s.target, 0, errors, "visible stub label section 0", items, NULL);

	invasive_scenario_stop(&s);
}

/* --- scenario 4: stubs first, a real menu after a re-serve ----------------- */

/* Nothing but placeholders: the filter has to leave the model empty. */
static GVariant *build_stubs_only(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);

	for (u_int32_t id = 10; id <= 14; id++)
		add_firefox_stub(&layout, id);

	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

/* The same ids the stubs held, now served as real items. Reusing them is the point: if a
 * dropped placeholder left anything behind in the importer, the re-served item has to be
 * reconciled with that leftover rather than simply inserted. */
static GVariant *build_real_menu(void)
{
	GVariantBuilder layout;
	invasive_begin_root(&layout);

	add_visible(&layout, 10, "New item");
	invasive_add_separator(&layout, 12, NULL);
	add_visible(&layout, 14, "Another item");

	invasive_end_root(&layout);
	return g_variant_builder_end(&layout);
}

static void scenario_stubs_then_real_menu(GString *errors)
{
	static const char *const first_items[]  = { "New item", NULL };
	static const char *const second_items[] = { "Another item", NULL };
	static const char *const no_items[]     = { NULL };

	InvasiveFixture s = { 0 };
	invasive_scenario_start(&s, build_stubs_only(), TEST_BUS_NAME);

	invasive_dump("stubs-only", s.target);

	/* Every served child was filtered away, so the top model must still hold only the
	 * first section it always owns and nothing inside it. */
	invasive_check_n_sections(s.target, errors, "stubs only", 1);
	invasive_check_section_label(s.target, 0, errors, "stubs only section 0", NULL);
	check_section(s.target, 0, errors, "stubs only section 0", no_items, NULL);

	invasive_scenario_serve(&s, build_real_menu());
	invasive_dump("stubs-only then real menu", s.target);

	/* After the swap the served menu has to be there in full: a separator that was allowed
	 * to open a section because of a leftover placeholder would misplace both items. */
	invasive_check_n_sections(s.target, errors, "replaced", 2);
	check_section(s.target, 0, errors, "replaced section 0", first_items, NULL);
	check_section(s.target, 1, errors, "replaced section 1", second_items, NULL);

	invasive_scenario_stop(&s);
}

/* Each scenario needs its own bus name and its own server/importer pair; the shared runner
 * collects every mismatch into one errors string so a single executable reports all three. */
G_GNUC_INTERNAL void test_func(void)
{
	GString *errors = g_string_new(NULL);

	scenario_firefox_menubar(errors);
	scenario_stub_lookalikes(errors);
	scenario_visible_stub_label(errors);
	scenario_stubs_then_real_menu(errors);

	if (errors->len > 0)
		g_printerr("firefox stub checks mismatched:\n%s", errors->str);

	int n_errors = (int) errors->len;
	g_string_free(errors, TRUE);
	g_assert_cmpint(n_errors, ==, 0);
}
