#include <gtk/gtk.h>

static GtkWidget *menubar;
static GtkWidget *submenuitem;

static void destroy_window(GtkWindow *window, gpointer user_data)
{
	gtk_main_quit();
}

static void realize_menubar(GtkButton *button, gpointer user_data)
{
	if (gtk_widget_get_parent(menubar) == NULL)
	{
		GtkWidget *window;
		GtkWidget *grid;

		window = gtk_widget_get_toplevel(GTK_WIDGET(button));
		grid   = gtk_bin_get_child(GTK_BIN(window));

		gtk_grid_insert_row(GTK_GRID(grid), 0);
		gtk_grid_attach(GTK_GRID(grid), menubar, 0, 0, 1, 1);
	}
}

static void unrealize_menubar(GtkButton *button, gpointer user_data)
{
	if (gtk_widget_get_parent(menubar) != NULL)
	{
		GtkWidget *window;
		GtkWidget *grid;

		window = gtk_widget_get_toplevel(GTK_WIDGET(button));
		grid   = gtk_bin_get_child(GTK_BIN(window));

		gtk_container_remove(GTK_CONTAINER(grid), menubar);
	}
}

static void add_submenu(GtkButton *button, gpointer user_data)
{
	if (gtk_widget_get_parent(submenuitem) == NULL)
	{
		if (submenuitem == NULL)
		{
			GtkWidget *submenu;
			GtkWidget *item;

			submenuitem = gtk_menu_item_new_with_label("Extras");

			submenu = gtk_menu_new();

			item = gtk_menu_item_new_with_label("Extra 1");
			gtk_container_add(GTK_CONTAINER(submenu), item);

			item = gtk_menu_item_new_with_label("Extra 2");
			gtk_container_add(GTK_CONTAINER(submenu), item);

			gtk_menu_item_set_submenu(GTK_MENU_ITEM(submenuitem), submenu);
		}

		gtk_container_add(GTK_CONTAINER(menubar), submenuitem);
	}
}

static void remove_submenu(GtkButton *button, gpointer user_data)
{
	if (submenuitem != NULL && gtk_widget_get_parent(submenuitem) != NULL)
		gtk_container_remove(GTK_CONTAINER(menubar), submenuitem);
}

int main(int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *submenu;
	GtkWidget *item;
	GtkWidget *button;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy_window), NULL);

	grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), grid);

	menubar = gtk_menu_bar_new();
	gtk_grid_attach(GTK_GRID(grid), menubar, 0, 0, 1, 1);
	g_object_ref(menubar);

	item    = gtk_menu_item_new_with_label("File");
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
	gtk_container_add(GTK_CONTAINER(menubar), item);

	item = gtk_menu_item_new_with_label("New");
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_menu_item_new_with_label("Open");
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_menu_item_new_with_label("Save");
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_separator_menu_item_new();
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_radio_menu_item_new_with_label(NULL, "Item 1");
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(item), "Item 2");
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(item), "Item 3");
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_radio_menu_item_new_with_label_from_widget(GTK_RADIO_MENU_ITEM(item), "Item 4");
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_separator_menu_item_new();
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_menu_item_new_with_label("Quit");
	gtk_container_add(GTK_CONTAINER(submenu), item);

	item    = gtk_menu_item_new_with_label("Edit");
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
	gtk_container_add(GTK_CONTAINER(menubar), item);

	item = gtk_menu_item_new_with_label("Copy");
	gtk_container_add(GTK_CONTAINER(submenu), item);
	item = gtk_menu_item_new_with_label("Paste");
	gtk_container_add(GTK_CONTAINER(submenu), item);

	button = gtk_button_new_with_label("Realize menubar");
	g_signal_connect(button, "clicked", G_CALLBACK(realize_menubar), NULL);
	gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);

	button = gtk_button_new_with_label("Unrealize menubar");
	g_signal_connect(button, "clicked", G_CALLBACK(unrealize_menubar), NULL);
	gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);

	button = gtk_button_new_with_label("Add submenu");
	g_signal_connect(button, "clicked", G_CALLBACK(add_submenu), NULL);
	gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1);

	button = gtk_button_new_with_label("Remove submenu");
	g_signal_connect(button, "clicked", G_CALLBACK(remove_submenu), NULL);
	gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 1, 1);

	gtk_widget_show_all(window);

	gtk_main();

	g_object_unref(menubar);

	return 0;
}
