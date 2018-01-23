#include "debug.h"
#include "importer.h"
#include <gtk/gtk.h>
#include <stdbool.h>

void on_importer_model_changed(GObject *obj, GParamSpec *pspec, gpointer data)
{
	DBusMenuImporter *importer = (DBusMenuImporter *)obj;
	GtkMenuBar *menubar        = GTK_MENU_BAR(data);
	GMenuModel *model;
	GActionGroup *action_group;
	g_object_get(importer, "model", &model, "action-group", &action_group, NULL);
	gtk_menu_shell_bind_model(menubar, model, "dbusmenu", true);
	gtk_widget_insert_action_group(menubar, "dbusmenu", action_group);
}

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	GtkWindow *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
	gtk_window_set_title(GTK_WINDOW(window), "Submenu");

	GtkBox *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	GtkMenuBar *menubar        = gtk_menu_bar_new();
	DBusMenuImporter *importer = dbus_menu_importer_new(":1.59", "/MenuBar/2");
	//	DBusMenuImporter *importer =
	//	    dbus_menu_importer_new(":1.120", "/com/canonical/menu/2a000ee");
	g_signal_connect(importer, "notify::model", G_CALLBACK(on_importer_model_changed), menubar);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
