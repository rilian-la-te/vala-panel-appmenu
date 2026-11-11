/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"

G_GNUC_INTERNAL void test_func(void)
{
	GMenu *root     = g_menu_new();
	GMenu *section1 = g_menu_new();
	g_menu_append(section1, "New", "app.new");
	g_menu_append_section(root, NULL, G_MENU_MODEL(section1));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root), g_simple_action_group_new());

	GString *errors = g_string_new(NULL);

	gboolean ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("dynamic/first comparison", &f, errors);
	g_assert_true(ok);
	g_string_set_size(errors, 0);

	/* Change model - should not desync */
	g_menu_append(section1, "Open", "app.open");
	g_menu_append(section1, "Save", "app.save");

	GMenu *section2 = g_menu_new();
	g_menu_append(section2, "Quit", "app.quit");
	g_menu_append_section(root, NULL, G_MENU_MODEL(section2));
	g_object_unref(section2);

	test_wait();

	ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("dynamic/second comparison", &f, errors);
	g_assert_true(ok);

	g_string_free(errors, TRUE);
	fixture_teardown(&f);
	g_object_unref(section1);
}