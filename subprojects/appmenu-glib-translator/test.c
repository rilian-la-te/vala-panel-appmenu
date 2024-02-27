/*
 * vala-panel-appmenu
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
	//	DBusMenuImporter *importer = dbus_menu_importer_new("org.krusader", "/MenuBar/2");
	DBusMenuImporter *importer = dbus_menu_importer_new(":1.250", "/MenuBar/1");
	//	DBusMenuImporter *importer = dbus_menu_importer_new(":1.49", "/MenuBar/2");
	//	DBusMenuImporter *importer =
	//	    dbus_menu_importer_new(":1.227", "/com/canonical/menu/300003e");
	//	DBusMenuImporter *importer =
	//	    dbus_menu_importer_new(":1.458", "/com/canonical/menu/4600016");
	//	DBusMenuImporter *importer =
	//	    dbus_menu_importer_new(":1.542", "/com/canonical/menu/2600041");
	g_signal_connect(importer, "notify::model", G_CALLBACK(on_importer_model_changed), menubar);
	g_signal_connect(importer, "notify::model", G_CALLBACK(on_importer_model_changed), menu);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), menu, FALSE, FALSE, 0);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
