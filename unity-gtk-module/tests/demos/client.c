#include <gtk/gtk.h>

#include "altgrabber.h"
#include "altmonitor.h"
#include "menusource.h"

#if 0
static void
items_changed (GMenuModel *menu,
               gint        position,
               gint        removed,
               gint        added,
               gpointer    user_data)
{
  g_print ("\n change");
  g_menu_markup_print_stderr (menu);
  g_print ("\n");
}
#endif

static void menu_changed(GObject *object, GParamSpec *pspec, gpointer user_data)
{
	static GActionGroup *actions;
	GtkMenuBar *bar = user_data;
	GMenuModel *menu;

	if (!actions)
		actions = G_ACTION_GROUP(g_simple_action_group_new());

	menu = menu_source_get_menu(MENU_SOURCE(object));

	g_print("\n");
	if (menu)
	{
		// g_signal_connect (menu, "items-changed", G_CALLBACK (items_changed), NULL);
		gtk_menu_shell_bind_model(GTK_MENU_SHELL(bar), G_MENU_MODEL(menu), 0, 0);
	}
	else
	{
		GMenu *empty;
		empty = g_menu_new();
		gtk_menu_shell_bind_model(GTK_MENU_SHELL(bar), G_MENU_MODEL(empty), 0, 0);
		g_print("[no menu]\n");
	}
	g_print("\n");
}

int main(int argc, char **argv)
{
	MenuSource *source;
	GMenuModel *m;
	GtkWidget *bar;
	GtkWidget *win;

	gtk_init(&argc, &argv);

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	bar = gtk_menu_bar_new();
	gtk_window_set_accept_focus(GTK_WINDOW(win), FALSE);
	gtk_container_add(GTK_CONTAINER(win), bar);
	gtk_window_set_default_size(GTK_WINDOW(win), 800, 200);
	gtk_widget_show_all(win);

	source = menu_source_get_for_screen(gdk_screen_get_default());
	g_signal_connect(source, "notify::menu", G_CALLBACK(menu_changed), bar);
	menu_changed(G_OBJECT(source), NULL, bar);

	gtk_main();

	return 0;
}
