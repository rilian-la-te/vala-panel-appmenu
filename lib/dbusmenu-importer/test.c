#include "importer.h"
#include <gtk/gtk.h>
#include <stdbool.h>

/* Markup printing {{{1 */

/* This used to be part of GLib, but it was removed before the stable
 * release because it wasn't generally useful.  We want it here, though.
 */
static void indent_string(GString *string, gint indent)
{
	while (indent--)
		g_string_append_c(string, ' ');
}

static GString *g_menu_markup_print_string(GString *string, GMenuModel *model, gint indent,
                                           gint tabstop)
{
	gboolean need_nl = FALSE;
	gint i, n;

	if
		G_UNLIKELY(string == NULL)
	string = g_string_new(NULL);

	n = g_menu_model_get_n_items(model);

	for (i = 0; i < n; i++)
	{
		GMenuAttributeIter *attr_iter;
		GMenuLinkIter *link_iter;
		GString *contents;
		GString *attrs;

		attr_iter = g_menu_model_iterate_item_attributes(model, i);
		link_iter = g_menu_model_iterate_item_links(model, i);
		contents  = g_string_new(NULL);
		attrs     = g_string_new(NULL);

		while (g_menu_attribute_iter_next(attr_iter))
		{
			const char *name = g_menu_attribute_iter_get_name(attr_iter);
			GVariant *value  = g_menu_attribute_iter_get_value(attr_iter);

			if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING))
			{
				gchar *str;
				str = g_markup_printf_escaped(" %s='%s'",
				                              name,
				                              g_variant_get_string(value, NULL));
				g_string_append(attrs, str);
				g_free(str);
			}

			else
			{
				gchar *printed;
				gchar *str;
				const gchar *type;

				printed = g_variant_print(value, TRUE);
				type    = g_variant_type_peek_string(g_variant_get_type(value));
				str     = g_markup_printf_escaped(
                                    "<attribute name='%s' type='%s'>%s</attribute>\n",
                                    name,
                                    type,
                                    printed);
				indent_string(contents, indent + tabstop);
				g_string_append(contents, str);
				g_free(printed);
				g_free(str);
			}

			g_variant_unref(value);
		}
		g_object_unref(attr_iter);

		while (g_menu_link_iter_next(link_iter))
		{
			const gchar *name = g_menu_link_iter_get_name(link_iter);
			GMenuModel *menu  = g_menu_link_iter_get_value(link_iter);
			gchar *str;

			if (contents->str[0])
				g_string_append_c(contents, '\n');

			str = g_markup_printf_escaped("<link name='%s'>\n", name);
			indent_string(contents, indent + tabstop);
			g_string_append(contents, str);
			g_free(str);

			g_menu_markup_print_string(contents, menu, indent + 2 * tabstop, tabstop);

			indent_string(contents, indent + tabstop);
			g_string_append(contents, "</link>\n");
			g_object_unref(menu);
		}
		g_object_unref(link_iter);

		if (contents->str[0])
		{
			indent_string(string, indent);
			g_string_append_printf(string, "<item%s>\n", attrs->str);
			g_string_append(string, contents->str);
			indent_string(string, indent);
			g_string_append(string, "</item>\n");
			need_nl = TRUE;
		}

		else
		{
			if (need_nl)
				g_string_append_c(string, '\n');

			indent_string(string, indent);
			g_string_append_printf(string, "<item%s/>\n", attrs->str);
			need_nl = FALSE;
		}

		g_string_free(contents, TRUE);
		g_string_free(attrs, TRUE);
	}

	return string;
}

void on_importer_model_changed(GObject *obj, GParamSpec *pspec, gpointer data)
{
	DBusMenuImporter *importer = (DBusMenuImporter *)obj;
	GtkMenuBar *menubar        = GTK_MENU_BAR(data);
	GMenuModel *model;
	GActionGroup *action_group;
	g_object_get(importer, "model", &model, "action-group", &action_group, NULL);
	gtk_menu_shell_bind_model(menubar, model, "dbusmenu", true);
	gtk_widget_insert_action_group(menubar, "dbusmenu", action_group);
	g_autoptr(GString) str = g_string_new(NULL);
	g_menu_markup_print_string(str, model, 4, 4);
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
	DBusMenuImporter *importer = dbus_menu_importer_new(":1.48", "/MenuBar/2");
	g_signal_connect(importer, "notify::model", G_CALLBACK(on_importer_model_changed), menubar);
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
