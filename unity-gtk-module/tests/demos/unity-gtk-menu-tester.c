#include <gtk/gtk.h>

enum
{
	ITEM_COLUMN,
	SHELL_COLUMN,
	LABEL_COLUMN,
	TYPE_COLUMN,
	ACCEL_COLUMN,
	N_COLUMNS
};

static GtkWidget *menu_bar;
static GtkWidget *tree_view;
static GtkTreeStore *tree_store;
static GtkWidget *entry;
static GSList *radio_group;

static void destroy_window(GtkWindow *window, gpointer user_data)
{
	gtk_main_quit();
}

static void update_model(GtkTreeStore *tree_store, GtkTreeIter *iter, GtkMenuShell *menu_shell)
{
	GtkTreeIter parent_iter;
	GtkTreeIter child_iter;
	GList *menu_items;

	if (iter == NULL)
	{
		if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tree_store), &parent_iter))
		{
			gtk_tree_store_append(tree_store, &parent_iter, NULL);
			gtk_tree_store_set(tree_store,
			                   &parent_iter,
			                   SHELL_COLUMN,
			                   menu_shell,
			                   LABEL_COLUMN,
			                   "Menu bar",
			                   -1);
		}
	}
	else
		parent_iter = *iter;

	menu_items = g_list_first(gtk_container_get_children(GTK_CONTAINER(menu_shell)));

	while (menu_items != NULL)
	{
		GtkMenuItem *menu_item;
		GtkWidget *submenu;

		menu_item = menu_items->data;
		submenu   = gtk_menu_item_get_submenu(menu_item);

		gtk_tree_store_append(tree_store, &child_iter, &parent_iter);
		gtk_tree_store_set(tree_store,
		                   &child_iter,
		                   ITEM_COLUMN,
		                   menu_item,
		                   SHELL_COLUMN,
		                   submenu,
		                   LABEL_COLUMN,
		                   gtk_menu_item_get_label(menu_item),
		                   -1);

		if (submenu != NULL)
			update_model(tree_store, &child_iter, GTK_MENU_SHELL(submenu));

		menu_items = g_list_next(menu_items);
	}
}

static void add_menu_bar(GtkToolButton *button, gpointer user_data)
{
	if (gtk_widget_get_parent(menu_bar) == NULL)
	{
		GtkWidget *window;

		window = gtk_widget_get_toplevel(GTK_WIDGET(button));

		if (gtk_widget_is_toplevel(window))
		{
			GtkWidget *grid;
			GtkTreeIter iter;

			grid = gtk_bin_get_child(GTK_BIN(window));
			gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);
			gtk_widget_show_all(menu_bar);

			gtk_tree_store_clear(tree_store);
			update_model(tree_store, NULL, GTK_MENU_SHELL(menu_bar));
			gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
		}
	}
}

static void remove_menu_bar(GtkToolButton *button, gpointer user_data)
{
	GtkWidget *parent = gtk_widget_get_parent(menu_bar);

	if (parent != NULL)
		gtk_container_remove(GTK_CONTAINER(parent), menu_bar);

	gtk_tree_store_clear(tree_store);
}

static GtkMenuItem *get_menu_item(GtkMenuShell *menu_shell, GtkTreeModel *tree_model,
                                  GtkTreeIter *child_iter);

static GtkMenuShell *get_menu_shell(GtkMenuShell *menu_shell, GtkTreeModel *tree_model,
                                    GtkTreeIter *child_iter)
{
	if (child_iter != NULL)
	{
		GtkTreeIter parent_iter;

		if (gtk_tree_model_iter_parent(tree_model, &parent_iter, child_iter))
			return GTK_MENU_SHELL(gtk_menu_item_get_submenu(
			    get_menu_item(menu_shell, tree_model, child_iter)));
		else
			return menu_shell;
	}

	return NULL;
}

static GtkMenuItem *get_menu_item(GtkMenuShell *menu_shell, GtkTreeModel *tree_model,
                                  GtkTreeIter *child_iter)
{
	GtkTreeIter parent_iter;

	if (child_iter != NULL && gtk_tree_model_iter_parent(tree_model, &parent_iter, child_iter))
	{
		GtkTreeIter item_iter;
		GList *children;

		menu_shell = get_menu_shell(menu_shell, tree_model, &parent_iter);
		item_iter  = *child_iter;

		children = gtk_container_get_children(GTK_CONTAINER(menu_shell));
		while (gtk_tree_model_iter_previous(tree_model, &item_iter))
			children = g_list_next(children);

		return children->data;
	}

	return NULL;
}

static GtkWidget *create_menu_item(void)
{
	GSList *toggle_buttons;
	GtkToggleButton *toggle_button;
	guint toggle_index;
	const gchar *mnemonic;
	GtkWidget *menu_item;

	toggle_buttons = radio_group;

	for (toggle_index = 0; toggle_buttons != NULL; toggle_index++)
	{
		toggle_button = toggle_buttons->data;

		if (gtk_toggle_button_get_active(toggle_button))
			break;

		toggle_buttons = g_slist_next(toggle_buttons);
	}

	if (toggle_buttons == NULL)
		return NULL;

	mnemonic = gtk_entry_get_text(GTK_ENTRY(entry));

	switch (toggle_index)
	{
	case 3:
		menu_item = gtk_check_menu_item_new_with_mnemonic(mnemonic);
		break;

	case 2:
		/* XXX */
		menu_item = gtk_radio_menu_item_new_with_mnemonic(NULL, mnemonic);
		break;

	case 1:
		menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
		gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menu_item), TRUE);
		break;

	case 0:
		menu_item = gtk_separator_menu_item_new();
		break;

	default:
		menu_item = gtk_menu_item_new_with_mnemonic(mnemonic);
		break;
	}

	return menu_item;
}

static void add_menu_item(GtkToolButton *button, gpointer user_data)
{
	GtkTreeSelection *selection;
	GtkTreeIter tree_iter;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));

	if (selection != NULL && gtk_tree_selection_get_selected(selection, NULL, &tree_iter))
	{
		GtkWidget *submenu;

		submenu = GTK_WIDGET(get_menu_shell(GTK_MENU_SHELL(menu_bar),
		                                    GTK_TREE_MODEL(tree_store),
		                                    &tree_iter));

		if (submenu == NULL)
		{
			GtkMenuItem *menu_item;

			menu_item = get_menu_item(GTK_MENU_SHELL(menu_bar),
			                          GTK_TREE_MODEL(tree_store),
			                          &tree_iter);

			if (menu_item != NULL)
			{
				submenu = gtk_menu_new();
				gtk_menu_item_set_submenu(menu_item, submenu);
			}
		}

		if (submenu != NULL)
		{
			GtkWidget *menu_item;

			menu_item = create_menu_item();
			gtk_container_add(GTK_CONTAINER(submenu), menu_item);
			gtk_widget_show(menu_item);

			gtk_tree_store_clear(tree_store);
			update_model(tree_store, NULL, GTK_MENU_SHELL(menu_bar));
			gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
		}
	}
}

static void insert_menu_item(GtkToolButton *button, gpointer user_data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *tree_model;
	GtkTreeIter parent_iter;
	GtkTreeIter child_iter;

	selection  = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	tree_model = GTK_TREE_MODEL(tree_store);

	if (selection != NULL && gtk_tree_selection_get_selected(selection, NULL, &child_iter) &&
	    gtk_tree_model_iter_parent(tree_model, &parent_iter, &child_iter))
	{
		GtkMenuShell *shell;
		GtkWidget *menu_item;
		guint shell_index;

		shell = get_menu_shell(GTK_MENU_SHELL(menu_bar), tree_model, &parent_iter);

		for (shell_index = 0; gtk_tree_model_iter_previous(tree_model, &child_iter);
		     shell_index++)
			;

		menu_item = create_menu_item();
		gtk_menu_shell_insert(shell, menu_item, shell_index);
		gtk_widget_show(menu_item);

		gtk_tree_store_clear(tree_store);
		update_model(tree_store, NULL, GTK_MENU_SHELL(menu_bar));
		gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
	}
}

static void remove_menu_item(GtkToolButton *button, gpointer user_data)
{
	GtkTreeSelection *selection;
	GtkTreeIter tree_iter;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));

	if (selection != NULL && gtk_tree_selection_get_selected(selection, NULL, &tree_iter))
	{
		GtkWidget *menu_item;

		menu_item = GTK_WIDGET(get_menu_item(GTK_MENU_SHELL(menu_bar),
		                                     GTK_TREE_MODEL(tree_store),
		                                     &tree_iter));

		if (menu_item != NULL)
		{
			GtkWidget *parent;

			parent = gtk_widget_get_parent(menu_item);

			if (parent != NULL)
			{
				gtk_container_remove(GTK_CONTAINER(parent), menu_item);

				gtk_tree_store_clear(tree_store);
				update_model(tree_store, NULL, GTK_MENU_SHELL(menu_bar));
				gtk_tree_view_expand_all(GTK_TREE_VIEW(tree_view));
			}
		}
	}
}

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *outer_grid;
	GtkWidget *menu;
	GtkWidget *menu_item;
	GtkWidget *paned;
	GtkWidget *inner_grid;
	GtkCellRenderer *cell_renderer;
	GtkTreeViewColumn *tree_view_column;
	GtkWidget *toolbar;
	GtkToolItem *tool_item;
	GtkWidget *label;
	GtkWidget *radio_button;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy_window), NULL);

	outer_grid = gtk_grid_new();

	menu_bar = g_object_ref(gtk_menu_bar_new());

	menu_item = gtk_menu_item_new_with_label("File");
	menu      = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_container_add(GTK_CONTAINER(menu_bar), menu_item);

	menu_item = gtk_menu_item_new_with_label("New");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Open");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Save");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_separator_menu_item_new();
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Recent File 1");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Recent File 2");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Recent File 3");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Recent File 4");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_separator_menu_item_new();
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Quit");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);

	menu_item = gtk_menu_item_new_with_label("Edit");
	menu      = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_container_add(GTK_CONTAINER(menu_bar), menu_item);

	menu_item = gtk_menu_item_new_with_label("Select All");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_separator_menu_item_new();
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Cut");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Copy");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);
	menu_item = gtk_menu_item_new_with_label("Paste");
	gtk_container_add(GTK_CONTAINER(menu), menu_item);

	paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

	inner_grid = gtk_grid_new();

	tree_store = gtk_tree_store_new(N_COLUMNS,
	                                GTK_TYPE_MENU_ITEM,
	                                GTK_TYPE_MENU_SHELL,
	                                G_TYPE_STRING,
	                                G_TYPE_STRING,
	                                G_TYPE_STRING);

	tree_view = g_object_ref(gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_store)));

	cell_renderer    = gtk_cell_renderer_text_new();
	tree_view_column = gtk_tree_view_column_new_with_attributes("Label",
	                                                            cell_renderer,
	                                                            "text",
	                                                            LABEL_COLUMN,
	                                                            NULL);
	gtk_tree_view_column_set_min_width(tree_view_column, 160);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_view_column);

	cell_renderer    = gtk_cell_renderer_text_new();
	tree_view_column = gtk_tree_view_column_new_with_attributes("Type",
	                                                            cell_renderer,
	                                                            "text",
	                                                            TYPE_COLUMN,
	                                                            NULL);
	gtk_tree_view_column_set_min_width(tree_view_column, 80);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_view_column);

	cell_renderer    = gtk_cell_renderer_text_new();
	tree_view_column = gtk_tree_view_column_new_with_attributes("Accel",
	                                                            cell_renderer,
	                                                            "text",
	                                                            ACCEL_COLUMN,
	                                                            NULL);
	gtk_tree_view_column_set_min_width(tree_view_column, 80);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_view_column);

	gtk_widget_set_vexpand(tree_view, TRUE);

	gtk_grid_attach(GTK_GRID(inner_grid), tree_view, 0, 0, 1, 1);

	toolbar = gtk_toolbar_new();
	gtk_widget_set_hexpand(toolbar, TRUE);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_YES);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(add_menu_bar), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, 0);

	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_NO);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(remove_menu_bar), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, 1);

	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(add_menu_item), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, 2);

	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(insert_menu_item), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, 3);

	tool_item = gtk_tool_button_new_from_stock(GTK_STOCK_REMOVE);
	g_signal_connect(tool_item, "clicked", G_CALLBACK(remove_menu_item), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, 4);

	gtk_grid_attach(GTK_GRID(inner_grid), toolbar, 0, 1, 1, 1);

	gtk_paned_add1(GTK_PANED(paned), inner_grid);

	inner_grid = gtk_grid_new();

	label = gtk_label_new_with_mnemonic("_Label:");
	entry = g_object_ref(gtk_entry_new());
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), entry);
	gtk_grid_attach(GTK_GRID(inner_grid), label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(inner_grid), entry, 1, 0, 1, 1);

	label = gtk_label_new("Type:");
	gtk_grid_attach(GTK_GRID(inner_grid), label, 0, 1, 1, 1);

	radio_button = gtk_radio_button_new_with_mnemonic(NULL, "_Normal");
	gtk_grid_attach(GTK_GRID(inner_grid), radio_button, 1, 1, 1, 1);

	radio_button =
	    gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(radio_button),
	                                                   "_Check");
	gtk_grid_attach(GTK_GRID(inner_grid), radio_button, 1, 2, 1, 1);

	radio_button =
	    gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(radio_button),
	                                                   "_Radio");
	gtk_grid_attach(GTK_GRID(inner_grid), radio_button, 1, 3, 1, 1);

	radio_button =
	    gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(radio_button),
	                                                   "_Image");
	gtk_grid_attach(GTK_GRID(inner_grid), radio_button, 1, 4, 1, 1);

	radio_button =
	    gtk_radio_button_new_with_mnemonic_from_widget(GTK_RADIO_BUTTON(radio_button),
	                                                   "_Separator");
	radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_button));
	gtk_grid_attach(GTK_GRID(inner_grid), radio_button, 1, 5, 1, 1);

	gtk_paned_add2(GTK_PANED(paned), inner_grid);

	gtk_grid_attach(GTK_GRID(outer_grid), paned, 0, 1, 1, 1);

	gtk_container_add(GTK_CONTAINER(window), outer_grid);

	gtk_widget_show_all(window);

	gtk_main();

	g_object_unref(entry);
	g_object_unref(tree_view);
	g_object_unref(menu_bar);

	return 0;
}
