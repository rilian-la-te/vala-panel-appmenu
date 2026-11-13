/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <gtk/gtk.h>

extern void test_func(void);

int main(int argc, char *argv[])
{
	g_setenv("NO_AT_BRIDGE", "1", TRUE);
	g_setenv("GSETTINGS_BACKEND", "memory", TRUE);
	g_setenv("GTK_A11Y", "none", TRUE);
	g_setenv("GTK_MODULES", "", TRUE);
	g_setenv("GIO_USE_VFS", "local", TRUE);
	g_setenv("GIO_USE_VOLUME_MONITOR", "unix", TRUE);
	g_test_init(&argc, &argv, NULL);

	if (!gtk_init_check(&argc, &argv))
	{
		g_printerr("Skipping: no display available\n");
		return 77;
	}
	GTestDBus *bus = g_test_dbus_new(G_TEST_DBUS_NONE);
	g_test_dbus_up(bus);

	g_test_add_func("/test", test_func);

	int ret = g_test_run();

	GdkDisplay *display = gdk_display_get_default();
	if (display != NULL)
		gdk_display_close(display);
	while (g_main_context_iteration(NULL, FALSE))
		;

	g_test_dbus_down(bus);
	g_object_unref(bus);
	return ret;
}