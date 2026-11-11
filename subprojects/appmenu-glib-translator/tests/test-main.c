/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gio/gio.h>

extern void test_func(void);

int main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	GTestDBus *bus = g_test_dbus_new(G_TEST_DBUS_NONE);
	g_test_dbus_up(bus);

	g_test_add_func("/dbusmenu/test", test_func);

	int ret = g_test_run();

	g_test_dbus_down(bus);
	g_object_unref(bus);

	return ret;
}