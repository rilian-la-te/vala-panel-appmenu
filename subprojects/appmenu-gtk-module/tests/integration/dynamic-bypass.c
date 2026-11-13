/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

const char *test_path = "/e2e/dynamic/add-and-remove";

void test_func(void)
{
	E2E e;
	e2e_setup(&e);
	assert_models(&e, "dyn/initial");

	g_menu_append(e.section2, "Save", "app.save");
	settle(400);
	load_imported(&e);
	assert_models(&e, "dyn/after-add");

	g_menu_remove(e.section2, 3);
	settle(400);
	load_imported(&e);
	assert_models(&e, "dyn/after-remove");

	e2e_teardown(&e);
}