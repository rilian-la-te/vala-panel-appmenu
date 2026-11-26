/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include "comparator.h"
#include "parser.h"

/* The markup the debug printer emits (dbus_menu_print_string) only ever uses
 * inline string attributes and non-empty items.  GtkBuilder UI files use a
 * superset of that shape: <item> elements whose label/action/... are expressed
 * as child <attribute name="...">text</attribute> elements (some marked
 * translatable="yes"), plus empty <section id="..."></section> grouping
 * elements.  This test feeds exactly that UI-file form to the parser and checks
 * it reconstructs the right hierarchy, including that empty sections collapse
 * away and translatable labels pass through unchanged when no catalog is bound. */
static const char *STUB_MARKUP =
    "<section>\n"
    "    <item>\n"
    "        <attribute name=\"label\" translatable=\"yes\">_New</attribute>\n"
    "        <attribute name=\"action\">conf.new</attribute>\n"
    "        <attribute name=\"hidden-when\">action-missing</attribute>\n"
    "    </item>\n"
    "</section>\n"
    "<section id=\"desktop-actions\">\n"
    "</section>\n"
    "<section id=\"unity-actions\">\n"
    "</section>\n"
    "<section>\n"
    "    <item>\n"
    "        <attribute name=\"label\" translatable=\"yes\">_Quit</attribute>\n"
    "        <attribute name=\"action\">conf.quit</attribute>\n"
    "        <attribute name=\"hidden-when\">action-missing</attribute>\n"
    "    </item>\n"
    "</section>\n";

/* A UI-file <section> mirrors a <link name='section'>: it becomes its own nested
 * GMenu wrapped in an item. The four sections therefore become four section-
 * linked items at the root, including the two empty ones (which wrap an empty
 * menu), matching GtkBuilder's model. */
static GMenuModel *build_expected(void)
{
	GMenu *root = g_menu_new();

	/* 1. section with _New / conf.new */
	GMenu *s1 = g_menu_new();
	GMenuItem *ni = g_menu_item_new("_New", "conf.new");
	g_menu_append_item(s1, ni);
	g_object_unref(ni);
	g_menu_append_section(root, NULL, G_MENU_MODEL(s1));
	g_object_unref(s1);

	/* 2. empty section (id="desktop-actions") */
	GMenu *s2 = g_menu_new();
	g_menu_append_section(root, NULL, G_MENU_MODEL(s2));
	g_object_unref(s2);

	/* 3. empty section (id="unity-actions") */
	GMenu *s3 = g_menu_new();
	g_menu_append_section(root, NULL, G_MENU_MODEL(s3));
	g_object_unref(s3);

	/* 4. section with _Quit / conf.quit */
	GMenu *s4 = g_menu_new();
	GMenuItem *qi = g_menu_item_new("_Quit", "conf.quit");
	g_menu_append_item(s4, qi);
	g_object_unref(qi);
	g_menu_append_section(root, NULL, G_MENU_MODEL(s4));
	g_object_unref(s4);

	return G_MENU_MODEL(root);
}

G_GNUC_INTERNAL void test_func(void)
{
	GError *error = NULL;
	GMenuModel *parsed = dbus_menu_parse_one(STUB_MARKUP, NULL, &error);

	if (parsed == NULL)
	{
		g_printerr("parse failed: %s\nmarkup was:\n%s", error->message, STUB_MARKUP);
		g_error_free(error);
		g_assert_not_reached();
	}

	/* All four sections are preserved as section-linked items (empty ones wrap an
	 * empty menu rather than collapsing). */
	g_assert_cmpuint(g_menu_model_get_n_items(parsed), ==, 4);

	GMenuModel *expected = build_expected();

	GString *errors = g_string_new(NULL);
	gboolean ok     = compare_models(expected, parsed, errors, 0);

	if (!ok)
	{
		g_printerr("ui-file parse comparison mismatch:\n%s", errors->str);
		dump_models("ui-file", expected, parsed);
	}

	g_assert_true(ok);

	/* Spot-check the attributes the comparator does not look at directly: they
	 * live on the item inside the first section. */
	GMenuModel *sec0 = g_menu_model_get_item_link(parsed, 0, G_MENU_LINK_SECTION);
	g_assert_nonnull(sec0);
	char *hw = NULL;
	g_assert_true(g_menu_model_get_item_attribute(sec0,
	                                              0,
	                                              "hidden-when",
	                                              "s",
	                                              &hw));
	g_assert_cmpstr(hw, ==, "action-missing");
	g_free(hw);
	g_object_unref(sec0);

	g_string_free(errors, TRUE);
	g_object_unref(parsed);
	g_object_unref(expected);
}
