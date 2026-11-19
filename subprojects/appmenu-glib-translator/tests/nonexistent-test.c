/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "comparator.h"

void test_func(void)
{
	Fixture f  = { 0 };
	f.importer = dbus_menu_importer_new(TEST_BUS_NAME, TEST_OBJECT_PATH);
	g_assert_nonnull(f.importer);
	test_wait();
	g_assert_nonnull(f.importer);
	g_clear_object(&f.importer);
}