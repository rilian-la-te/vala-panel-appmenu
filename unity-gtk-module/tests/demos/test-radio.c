#include <gtk/gtk.h>

static void handle_window_destroy(GtkWindow *window, gpointer user_data)
{
	gtk_main_quit();
}

static void handle_menu_item_activate(GtkMenuItem *menu_item, gpointer user_data)
{
	g_message("%s: %s", __func__, (const gchar *)user_data);
}

static void handle_check_menu_item_toggled(GtkCheckMenuItem *check_menu_item, gpointer user_data)
{
	g_message("%s: %s", __func__, (const gchar *)user_data);
}

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *menu_bar;
	GtkWidget *menu;
	GtkWidget *menu_item;
	GtkWidget *button;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", G_CALLBACK(handle_window_destroy), NULL);

	grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), grid);

	menu_bar = gtk_menu_bar_new();
	gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);

	menu_item = gtk_menu_item_new_with_label("File");
	gtk_container_add(GTK_CONTAINER(menu_bar), menu_item);

	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

	menu_item = gtk_radio_menu_item_new_with_label(NULL, "Radio A");
	g_signal_connect(menu_item, "activate", G_CALLBACK(handle_menu_item_activate), "Radio A");
	g_signal_connect(menu_item,
	                 "toggled",
	                 G_CALLBACK(handle_check_menu_item_toggled),
	                 "Radio A");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(menu_item),
	                                                           "Radio B");
	g_signal_connect(menu_item, "activate", G_CALLBACK(handle_menu_item_activate), "Radio B");
	g_signal_connect(menu_item,
	                 "toggled",
	                 G_CALLBACK(handle_check_menu_item_toggled),
	                 "Radio B");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(menu_item),
	                                                           "Radio C");
	g_signal_connect(menu_item, "activate", G_CALLBACK(handle_menu_item_activate), "Radio C");
	g_signal_connect(menu_item,
	                 "toggled",
	                 G_CALLBACK(handle_check_menu_item_toggled),
	                 "Radio C");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
