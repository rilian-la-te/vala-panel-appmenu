/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

#include <stdio.h>
#include <unistd.h>

#include "comparator.h"
#include "parser.h"

/* Build a representative menu, print it with the markup printer (captured from
 * stderr), parse that text back with dbus_menu_parse_one(), then compare
 * the original and the parsed model structurally. This exercises string
 * attributes (label/action), non-string <attribute> children, submenu links,
 * section links, empty submenus, and XML escaping in labels. */
static GMenuModel *build_source(void)
{
	GMenu *root = g_menu_new();

	/* A section holding plain items, including a label that needs escaping. */
	GMenu *section = g_menu_new();
	g_menu_append(section, "New", "app.new");
	g_menu_append(section, "Save & Close", "app.save-close"); /* '&' must survive escaping */
	g_menu_append_section(root, NULL, G_MENU_MODEL(section));
	g_object_unref(section);

	/* A submenu with a label and its own items. */
	GMenu *sub = g_menu_new();
	g_menu_append(sub, "Recent 1", "app.recent1");
	g_menu_append(sub, "Recent 2", "app.recent2");
	g_menu_append_submenu(root, "Open Recent", G_MENU_MODEL(sub));
	g_object_unref(sub);

	/* An item carrying non-string attributes: a boolean and an int32 target.
	 * These become <attribute type='b'>/<attribute type='i'> children. */
	GMenuItem *toggle = g_menu_item_new("Toggle", "app.toggle");
	g_menu_item_set_attribute_value(toggle, "visible", g_variant_new_boolean(FALSE));
	g_menu_item_set_attribute_value(toggle,
	                                G_MENU_ATTRIBUTE_TARGET,
	                                g_variant_new_int32(42));
	g_menu_append_item(root, toggle);
	g_object_unref(toggle); /* append does not sink the floating ref */

	/* An empty submenu (link with no items). */
	GMenu *empty = g_menu_new();
	g_menu_append_submenu(root, "Nothing Here", G_MENU_MODEL(empty));
	g_object_unref(empty);

	return G_MENU_MODEL(root);
}

/* dbus_menu_print_stdout writes with g_printerr(), i.e. to fd 2.
 * Capture that output through a pipe so we can feed it back to the parser. */
static char *capture_markup(GMenuModel *model)
{
	int fds[2];

	if (pipe(fds) != 0)
		return NULL;

	int saved = dup(2);
	dup2(fds[1], 2);

	dbus_menu_print_stdout(model);

	fflush(stderr);
	dup2(saved, 2);
	close(saved);
	close(fds[1]); /* writer closed -> read() below sees EOF */

	GString *out = g_string_new(NULL);
	char buf[4096];
	ssize_t n;

	while ((n = read(fds[0], buf, sizeof buf)) > 0)
		g_string_append_len(out, buf, n);

	close(fds[0]);
	return g_string_free(out, FALSE);
}

G_GNUC_INTERNAL void test_func(void)
{
	GMenuModel *source = build_source();

	char *markup = capture_markup(source);
	g_assert_nonnull(markup);
	g_assert_true(markup[0] != '\0');

	GError *error = NULL;
	GMenuModel *parsed = dbus_menu_parse_one(markup, NULL, &error);

	if (parsed == NULL)
	{
		g_printerr("parse failed: %s\nmarkup was:\n%s", error->message, markup);
		g_error_free(error);
		g_free(markup);
		g_object_unref(source);
		g_assert_not_reached();
	}

	GString *errors = g_string_new(NULL);
	gboolean ok = compare_models(source, parsed, errors, 0);

	if (!ok)
	{
		g_printerr("print/parse comparison mismatch:\n%s", errors->str);
		dump_models("source", source, parsed);
		g_printerr("--- markup ---\n%s--- end markup ---\n", markup);
	}

	g_assert_true(ok);
	g_assert_cmpint(g_menu_model_get_n_items(parsed), ==, g_menu_model_get_n_items(source));

	g_string_free(errors, TRUE);
	g_object_unref(parsed);
	g_object_unref(source);
	g_free(markup);
}
