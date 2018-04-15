/*
 * vala-panel-appmenu
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
 * Copyright (C) 2013-2018 Allison Lortie <desrt@desrt.ca>
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
#include <gio/gio.h>
#include <stdbool.h>
/* Markup printing {{{1 */

/* This used to be part of GLib, but it was removed before the stable
 * release because it wasn't generally useful.  We want it here, though.
 */

G_GNUC_INTERNAL void g_menu_markup_print_to_console(GMenuModel *menu)
{
	GString *str = g_string_new(NULL);
	g_menu_markup_print_string(str, menu, 4, 4);
	char *cstr = g_string_free(str, false);
	g_print("%s\n", cstr);
}

G_GNUC_INTERNAL void dbus_menu_print_variant(GVariant *var)
{
	g_autofree char *pr = g_variant_print(var, true);
	g_print("%s\n", pr);
}

static void indent_string(GString *string, gint indent)
{
	while (indent--)
		g_string_append_c(string, ' ');
}

G_GNUC_INTERNAL GString *g_menu_markup_print_string(GString *string, GMenuModel *model, gint indent,
                                                    gint tabstop)
{
	gboolean need_nl = false;
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
				char *str;
				str = g_markup_printf_escaped(" %s='%s'",
				                              name,
				                              g_variant_get_string(value, NULL));
				g_string_append(attrs, str);
				g_free(str);
			}

			else
			{
				char *printed;
				char *str;
				const char *type;

				printed = g_variant_print(value, true);
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
			const char *name = g_menu_link_iter_get_name(link_iter);
			GMenuModel *menu = g_menu_link_iter_get_value(link_iter);
			char *str;

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
			need_nl = true;
		}

		else
		{
			if (need_nl)
				g_string_append_c(string, '\n');

			indent_string(string, indent);
			g_string_append_printf(string, "<item%s/>\n", attrs->str);
			need_nl = false;
		}

		g_string_free(contents, true);
		g_string_free(attrs, true);
	}

	return string;
}
