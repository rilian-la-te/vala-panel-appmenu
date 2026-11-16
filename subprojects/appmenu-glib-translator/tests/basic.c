/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"
#include "exporter/exporter.h"
#include "importer/importer.h"

static GMenuModel *build_source_model(void)
{
	GMenu *menu = g_menu_new();

	/* Section 1: plain items. */
	{
		GMenu *s = g_menu_new();
		g_menu_append(s, "New", "app.new");
		g_menu_append(s, "Open", "app.open");
		g_menu_append(s, "Save", "app.save");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(s));
		g_object_unref(s);
	}

	/* Section 2: submenu + a plain item. */
	{
		GMenu *s      = g_menu_new();
		GMenu *recent = g_menu_new();
		g_menu_append(recent, "Recent 1", "app.recent1");
		g_menu_append(recent, "Recent 2", "app.recent2");
		g_menu_append_submenu(s, "Recent", G_MENU_MODEL(recent));
		g_object_unref(recent);
		g_menu_append(s, "Quit", "app.quit");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(s));
		g_object_unref(s);
	}

	/* Section 3: a single item. */
	{
		GMenu *s = g_menu_new();
		g_menu_append(s, "Help", "app.help");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(s));
		g_object_unref(s);
	}

	return G_MENU_MODEL(menu);
}

/* ------------------------------------------------------------------ */

G_GNUC_INTERNAL void test_func(void)
{
	GMenuModel *source               = build_source_model();
	GSimpleActionGroup *action_group = g_simple_action_group_new();
	DBusMenuExporter *exporter       = dbus_menu_exporter_new(source,
	                                                          "",
	                                                          G_ACTION_GROUP(action_group),
	                                                          TEST_BUS_NAME,
	                                                          TEST_OBJECT_PATH);
	test_wait();
	DBusMenuImporter *importer = dbus_menu_importer_new(TEST_BUS_NAME, TEST_OBJECT_PATH);
	test_wait();
	GMenuModel *target = NULL;
	g_object_get(importer, "model", &target, NULL);

	GString *errors = g_string_new(NULL);
	gboolean ok     = compare_models(source, target, errors, 0);
	if (!ok)
	{
		g_printerr("Models differ:\n%s\n", errors->str);
		g_printerr("--- source ---\n");
		g_menu_markup_print_to_console(source);
		g_printerr("--- parsed ---\n");
		g_menu_markup_print_to_console(target);
	}

	g_string_free(errors, TRUE);
	g_object_unref(action_group);
	g_object_unref(exporter);
	g_object_unref(importer);
	g_object_unref(target);
	g_object_unref(source);
	test_wait();

	g_assert_true(ok);
}