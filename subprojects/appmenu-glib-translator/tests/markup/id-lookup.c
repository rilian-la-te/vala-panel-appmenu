/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include "comparator.h"
#include "parser.h"

#define DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID "x-translator-id"

/* A GtkBuilder-style UI file where several elements declare id=. The handles must
 * end up on the items as DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID, for ordinary items
 * (inline id=) and for section/submenu wrapper items (<section id>/<submenu id>)
 * alike, so that one walker can find any of them without a side table. */
static const char *UI_FILE =
    "<section id=\"file-section\">\n"
    "    <item id=\"new-item\" label=\"_New\" action=\"app.new\"/>\n"
    "    <submenu id=\"recent-sub\">\n"
    "        <item id=\"recent-1\" label=\"Recent 1\" action=\"app.recent1\"/>\n"
    "    </submenu>\n"
    "</section>\n"
    "<section id=\"quit-section\">\n"
    "    <item>\n"
    "        <attribute name=\"label\" translatable=\"yes\">_Quit</attribute>\n"
    "        <attribute name=\"action\">app.quit</attribute>\n"
    "    </item>\n"
    "</section>\n";

static void expect_id(GMenuModel *model, int i, const char *want)
{
	g_autofree char *got = NULL;
	g_assert_true(g_menu_model_get_item_attribute(model,
	                                              i,
	                                              DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID,
	                                              "s",
	                                              &got));
	g_assert_cmpstr(got, ==, want);
}

/* Looks an id up and checks the copy it returns carries the right payload: the
 * label for an item, the link (and its contents) for a group. */
static void check_lookup(GMenuModel *model, const char *id, const char *want_label)
{
	GMenuItem *item = dbus_menu_parsed_find_item_by_id(model, id);
	g_assert_nonnull(item);

	if (want_label != NULL)
	{
		g_autofree char *label = NULL;
		g_assert_true(g_menu_item_get_attribute(item, "label", "s", &label));
		g_assert_cmpstr(label, ==, want_label);
	}

	g_object_unref(item);
}

G_GNUC_INTERNAL void test_func(void)
{
	GError *error = NULL;
	GMenuModel *parsed = dbus_menu_parse_one(UI_FILE, NULL, &error);

	if (parsed == NULL)
	{
		g_printerr("parse failed: %s\nmarkup was:\n%s", error->message, UI_FILE);
		g_error_free(error);
		g_assert_not_reached();
	}

	/* The two sections are the root's items, in document order. */
	g_assert_cmpuint(g_menu_model_get_n_items(parsed), ==, 2);
	expect_id(parsed, 0, "file-section");
	expect_id(parsed, 1, "quit-section");

	/* Inside the first section: a plain item and a submenu, both named. */
	GMenuModel *file_sec = g_menu_model_get_item_link(parsed, 0, G_MENU_LINK_SECTION);
	g_assert_nonnull(file_sec);
	g_assert_cmpuint(g_menu_model_get_n_items(file_sec), ==, 2);
	expect_id(file_sec, 0, "new-item");
	expect_id(file_sec, 1, "recent-sub");

	GMenuModel *recent = g_menu_model_get_item_link(file_sec, 1, G_MENU_LINK_SUBMENU);
	g_assert_nonnull(recent);
	g_assert_cmpuint(g_menu_model_get_n_items(recent), ==, 1);
	expect_id(recent, 0, "recent-1");

	/* Every handle is reachable from the root by walking alone. */
	check_lookup(parsed, "new-item", "_New");
	check_lookup(parsed, "recent-1", "Recent 1");
	check_lookup(parsed, "file-section", NULL);
	check_lookup(parsed, "recent-sub", NULL);

	/* The returned item's link is the SAME live sub-model, not a copy: modifying
	 * it through that borrowed pointer changes the real tree. */
	GMenuModel *tree_recent = g_menu_model_get_item_link(file_sec, 1, G_MENU_LINK_SUBMENU);
	g_assert_nonnull(tree_recent);

	GMenuItem *sub = dbus_menu_parsed_find_item_by_id(parsed, "recent-sub");
	g_assert_nonnull(sub);
	GMenuModel *sub_model = g_menu_item_get_link(sub, G_MENU_LINK_SUBMENU);
	g_assert_nonnull(sub_model);
	g_assert_true(sub_model == tree_recent); /* identical pointer, not a deep copy */
	g_assert_cmpuint(g_menu_model_get_n_items(tree_recent), ==, 1);

	/* In-place edit through the borrowed link reaches the group the tree sees. */
	g_menu_insert((GMenu *) sub_model, 0, "Injected", "app.injected");
	g_assert_cmpuint(g_menu_model_get_n_items(tree_recent), ==, 2);
	g_menu_remove((GMenu *) sub_model, 0);
	g_assert_cmpuint(g_menu_model_get_n_items(tree_recent), ==, 1);

	g_object_unref(sub_model);
	g_object_unref(sub);
	g_object_unref(tree_recent);

	/* An unknown id is not found. */
	g_assert_null(dbus_menu_parsed_find_item_by_id(parsed, "nope"));

	/* The section that holds only an <attribute>-built item still carries its own
	 * handle, and the id= must not leak onto that inner item. */
	GMenuModel *quit_sec = g_menu_model_get_item_link(parsed, 1, G_MENU_LINK_SECTION);
	g_assert_nonnull(quit_sec);
	g_assert_cmpuint(g_menu_model_get_n_items(quit_sec), ==, 1);
	g_autofree char *inner_id = NULL;
	g_assert_false(g_menu_model_get_item_attribute(quit_sec,
	                                               0,
	                                               DBUS_MENU_ATTRIBUTE_TRANSLATOR_ID,
	                                               "s",
	                                               &inner_id));

	g_object_unref(quit_sec);
	g_object_unref(recent);
	g_object_unref(file_sec);
	g_object_unref(parsed);
}
