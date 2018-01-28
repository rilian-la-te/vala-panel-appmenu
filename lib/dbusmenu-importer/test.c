#include "debug.h"
#include "importer.h"
#include <gtk/gtk.h>
#include <stdbool.h>

void on_importer_model_changed(GObject *obj, GParamSpec *pspec, gpointer data)
{
	DBusMenuImporter *importer = (DBusMenuImporter *)obj;
	GMenuModel *model;
	GActionGroup *action_group;
	g_object_get(importer, "model", &model, "action-group", &action_group, NULL);
	if (GTK_IS_MENU_SHELL(data))
	{
		GtkMenuShell *menubar = GTK_MENU_SHELL(data);
		gtk_widget_insert_action_group(menubar, "dbusmenu", action_group);
		gtk_menu_shell_bind_model(menubar, model, NULL, true);
	}
	else if (GTK_IS_MENU_BUTTON(data))
	{
		GtkMenuButton *btn = GTK_MENU_BUTTON(data);
		GtkMenu *popover   = gtk_popover_new_from_model(btn, model);
		gtk_widget_insert_action_group(popover, "dbusmenu", action_group);
		gtk_menu_button_set_popover(btn, popover);
	}
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	GtkWindow *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 1000, 1000);
	gtk_window_set_title(GTK_WINDOW(window), "Submenu");

	GtkBox *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	GtkMenuBar *menubar = gtk_menu_bar_new();
	GtkMenuButton *menu = gtk_menu_button_new();
	//            DBusMenuImporter *importer = dbus_menu_importer_new("org.krusader",
	//            "/MenuBar/2");
	//	DBusMenuImporter *importer = dbus_menu_importer_new(":1.46", "/MenuBar/2");
	DBusMenuImporter *importer =
	    dbus_menu_importer_new(":1.227", "/com/canonical/menu/300003e");
	//    DBusMenuImporter *importer =
	//        dbus_menu_importer_new(":1.198", "/com/canonical/menu/5A00001");
	g_signal_connect(importer, "notify::model", G_CALLBACK(on_importer_model_changed), menubar);
	g_signal_connect(importer, "notify::model", G_CALLBACK(on_importer_model_changed), menu);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), menu, FALSE, FALSE, 0);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
