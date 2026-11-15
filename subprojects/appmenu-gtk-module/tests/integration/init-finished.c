/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

static gboolean state_ready           = FALSE;
static gboolean saw_show_before_ready = FALSE;
static gboolean saw_show_after_ready  = FALSE;
static guint    show_calls            = 0;

static void on_submenu_show(GtkWidget *submenu, gpointer data)
{
	(void)submenu;
	(void)data;
	show_calls++;
	if (state_ready)
		saw_show_after_ready = TRUE;
	else
		saw_show_before_ready = TRUE;
}

G_GNUC_INTERNAL void test_func(void)
{
	GtkWidget *menubar    = gtk_menu_bar_new();
	GtkWidget *go_item    = gtk_menu_item_new_with_label("Go");
	GtkWidget *go_submenu = gtk_menu_new();
	GtkWidget *home_item  = gtk_menu_item_new_with_label("Home");

	gtk_menu_shell_append(GTK_MENU_SHELL(go_submenu), home_item);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(go_item), go_submenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), go_item);

	state_ready           = FALSE;
	saw_show_before_ready = FALSE;
	saw_show_after_ready  = FALSE;
	show_calls            = 0;
	g_signal_connect(go_submenu, "show",
	                 G_CALLBACK(on_submenu_show), NULL);

	GMenuModel *shell =
	    G_MENU_MODEL(unity_gtk_menu_shell_new(GTK_MENU_SHELL(menubar)));
	(void)g_menu_model_get_n_items(shell);
	state_ready = TRUE;
	g_assert_false(saw_show_before_ready);
	gtk_widget_show(go_submenu);
	g_assert_cmpuint(show_calls, >, 0);
	g_assert_true(saw_show_after_ready);
	g_assert_false(saw_show_before_ready);

	g_object_unref(shell);
	gtk_widget_destroy(menubar);
}
