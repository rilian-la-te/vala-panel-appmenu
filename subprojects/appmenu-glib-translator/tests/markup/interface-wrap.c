/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include "comparator.h"
#include "parser.h"

/* A GtkBuilder UI file keeps its menu content inside <interface>, and names that
 * content with <menu id="NAME">. The lib now hands one whole menu file per parse
 * to the parser, so both containers must be stepped over: this checks that a
 * correct UI file yields exactly the model the bare content yields. */
static const char *UNWRAPPED =
    "<section>\n"
    "    <item>\n"
    "        <attribute name=\"label\" translatable=\"yes\">_New</attribute>\n"
    "        <attribute name=\"action\">conf.new</attribute>\n"
    "        <attribute name=\"hidden-when\">action-missing</attribute>\n"
    "    </item>\n"
    "</section>\n";

/* A correct single-menu GtkBuilder UI file: the interface names its translation
 * domain, the menu its name, and neither of them is model content. */
static const char *UI_FILE =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<interface domain=\"vala-panel-appmenu\">\n"
    "  <menu id=\"appmenu-stub\">\n"
    "      <section>\n"
    "          <item>\n"
    "              <attribute name=\"label\" translatable=\"yes\">_New</attribute>\n"
    "              <attribute name=\"action\">conf.new</attribute>\n"
    "              <attribute name=\"hidden-when\">action-missing</attribute>\n"
    "          </item>\n"
    "      </section>\n"
    "  </menu>\n"
    "</interface>\n";

static void check_ids(void);
static void check_lazy_submenu(void);

static GMenuModel *parse_or_fail(const char *markup, const char *what)
{
	GError *error = NULL;
	GMenuModel *parsed = dbus_menu_parse_one(markup, NULL, &error);

	if (parsed == NULL)
	{
		g_printerr("parsing %s failed: %s\nmarkup was:\n%s", what, error->message, markup);
		g_error_free(error);
		g_assert_not_reached();
	}

	return parsed;
}

G_GNUC_INTERNAL void test_func(void)
{
	/* The bare content and the same content inside a correct UI file must agree
	 * element for element: the containers may only be stepped over, never turned
	 * into items of their own. */
	GMenuModel *plain   = parse_or_fail(UNWRAPPED, "unwrapped markup");
	GMenuModel *wrapped = parse_or_fail(UI_FILE, "UI-file markup");

	GString *errors = g_string_new(NULL);
	gboolean ok     = compare_models(plain, wrapped, errors, 0);

	if (!ok)
	{
		g_printerr("UI file containers changed the model:\n%s", errors->str);
		dump_models("content vs UI file", plain, wrapped);
	}

	g_string_free(errors, TRUE);
	g_assert_true(ok);
	/* The section is the menu's only item; <interface> and <menu> added none. */
	g_assert_cmpuint(g_menu_model_get_n_items(wrapped), ==, 1);

	g_object_unref(plain);
	g_object_unref(wrapped);

	/* A content element's id= must survive the containers, and so must a lazy
	 * submenu's attributes: that is how the desktop helper reaches its groups and
	 * their populate actions without GTK. */
	check_ids();
	check_lazy_submenu();
}

static const char *ID_UI_FILE =
    "<interface>\n"
    "  <menu id=\"appmenu-stub\">\n"
    "      <section id=\"desktop-actions\"></section>\n"
    "      <submenu id=\"recent-sub\">\n"
    "          <item id=\"recent-1\" label=\"Recent 1\" action=\"app.recent1\"/>\n"
    "      </submenu>\n"
    "  </menu>\n"
    "</interface>\n";

static void expect_id(GMenuModel *model, int i, const char *want)
{
	g_autofree char *got = NULL;
	g_assert_true(g_menu_model_get_item_attribute(model,
	                                              i,
	                                              "x-translator-id",
	                                              "s",
	                                              &got));
	g_assert_cmpstr(got, ==, want);
}

static void check_ids(void)
{
	GMenuModel *model = parse_or_fail(ID_UI_FILE, "UI file with ids");

	g_assert_cmpuint(g_menu_model_get_n_items(model), ==, 2);
	expect_id(model, 0, "desktop-actions");
	expect_id(model, 1, "recent-sub");

	/* Both groups are reachable by id, and the inner item of the submenu too. */
	GMenuItem *sub = dbus_menu_parsed_find_item_by_id(model, "recent-sub");
	g_assert_nonnull(sub);
	GMenuModel *sub_model = g_menu_item_get_link(sub, G_MENU_LINK_SUBMENU);
	g_assert_nonnull(sub_model);
	g_assert_cmpuint(g_menu_model_get_n_items(sub_model), ==, 1);
	expect_id(sub_model, 0, "recent-1");

	/* The empty section is preserved through the containers, as GtkBuilder keeps it. */
	GMenuItem *sec = dbus_menu_parsed_find_item_by_id(model, "desktop-actions");
	g_assert_nonnull(sec);
	GMenuModel *sec_model = g_menu_item_get_link(sec, G_MENU_LINK_SECTION);
	g_assert_nonnull(sec_model);
	g_assert_cmpuint(g_menu_model_get_n_items(sec_model), ==, 0);

	g_object_unref(sec_model);
	g_object_unref(sec);
	g_object_unref(sub_model);
	g_object_unref(sub);
	g_object_unref(model);
}

/* GtkBuilder's lazy submenu: a <submenu id=...> whose label and submenu-action
 * are given as <attribute> children instead of inline on the tag. Those annotate
 * the wrapper item that links the group, while its contents stay in the submodel,
 * which is empty until the action populates it. */
static const char *LAZY_SUBMENU_FILE =
    "<interface domain=\"vala-panel-appmenu\">\n"
    "  <menu id=\"menubar\">\n"
    "      <submenu id=\"files\">\n"
    "          <attribute name=\"label\" translatable=\"yes\">_Files</attribute>\n"
    "          <attribute name=\"submenu-action\">menu.populate-files</attribute>\n"
    "      </submenu>\n"
    "  </menu>\n"
    "</interface>\n";

static void check_lazy_submenu(void)
{
	GMenuModel *model = parse_or_fail(LAZY_SUBMENU_FILE, "lazy submenu UI file");

	g_assert_cmpuint(g_menu_model_get_n_items(model), ==, 1);

	/* The attributes landed on the wrapper item... */
	g_autofree char *label = NULL;
	g_autofree char *action = NULL;
	g_assert_true(g_menu_model_get_item_attribute(model, 0, G_MENU_ATTRIBUTE_LABEL, "s", &label));
	g_assert_cmpstr(label, ==, "_Files");
	g_assert_true(g_menu_model_get_item_attribute(model,
	                                              0,
	                                              G_MENU_ATTRIBUTE_SUBMENU_ACTION,
	                                              "s",
	                                              &action));
	g_assert_cmpstr(action, ==, "menu.populate-files");

	/* ...and the group itself is an empty submodel held by the submenu link. */
	GMenuModel *sub = g_menu_model_get_item_link(model, 0, G_MENU_LINK_SUBMENU);
	g_assert_nonnull(sub);
	g_assert_cmpuint(g_menu_model_get_n_items(sub), ==, 0);
	g_object_unref(sub);

	/* It is still findable by its id, which is how the desktop helper reaches it. */
	GMenuItem *item = dbus_menu_parsed_find_item_by_id(model, "files");
	g_assert_nonnull(item);
	g_autofree char *found_label = NULL;
	g_assert_true(g_menu_item_get_attribute(item, G_MENU_ATTRIBUTE_LABEL, "s", &found_label));
	g_assert_cmpstr(found_label, ==, "_Files");
	g_object_unref(item);

	g_object_unref(model);
}
