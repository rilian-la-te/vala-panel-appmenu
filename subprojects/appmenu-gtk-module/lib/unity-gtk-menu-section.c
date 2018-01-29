/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Ryan Lortie <desrt@desrt.ca>
 *          William Hua <william.hua@canonical.com>
 */

#include "unity-gtk-menu-section-private.h"

#ifndef G_MENU_ATTRIBUTE_ACCEL
#define G_MENU_ATTRIBUTE_ACCEL "accel"
#endif

#ifndef G_MENU_ATTRIBUTE_ACCEL_TEXT
#define G_MENU_ATTRIBUTE_ACCEL_TEXT "x-canonical-accel"
#endif

#ifndef G_MENU_ATTRIBUTE_SUBMENU_ACTION
#define G_MENU_ATTRIBUTE_SUBMENU_ACTION "submenu-action"
#endif

G_DEFINE_TYPE(UnityGtkMenuSection, unity_gtk_menu_section, G_TYPE_MENU_MODEL);

static gint g_uintcmp(gconstpointer a, gconstpointer b, gpointer user_data)
{
	return GPOINTER_TO_INT(a) - GPOINTER_TO_INT(b);
}

static gboolean g_closure_equal(GtkAccelKey *key, GClosure *closure, gpointer data)
{
	return closure == data;
}

static void unity_gtk_menu_section_set_parent_shell(UnityGtkMenuSection *section,
                                                    UnityGtkMenuShell *parent_shell)
{
	g_return_if_fail(UNITY_GTK_IS_MENU_SECTION(section));

	section->parent_shell = parent_shell;
}

static void unity_gtk_menu_section_dispose(GObject *object)
{
	UnityGtkMenuSection *section;

	g_return_if_fail(UNITY_GTK_IS_MENU_SECTION(object));

	section = UNITY_GTK_MENU_SECTION(object);

	unity_gtk_menu_section_set_parent_shell(section, NULL);

	G_OBJECT_CLASS(unity_gtk_menu_section_parent_class)->dispose(object);
}

static gboolean unity_gtk_menu_section_is_mutable(GMenuModel *model)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_SECTION(model), TRUE);

	return TRUE;
}

static gint unity_gtk_menu_section_get_n_items(GMenuModel *model)
{
	UnityGtkMenuSection *section;
	GSequenceIter *begin;
	GSequenceIter *end;

	g_return_val_if_fail(UNITY_GTK_IS_MENU_SECTION(model), 0);

	section = UNITY_GTK_MENU_SECTION(model);
	begin   = unity_gtk_menu_section_get_begin_iter(section);
	end     = unity_gtk_menu_section_get_end_iter(section);

	g_return_val_if_fail(begin != NULL && end != NULL, 0);

	return g_sequence_iter_get_position(end) - g_sequence_iter_get_position(begin);
}

static void unity_gtk_menu_section_get_item_attributes(GMenuModel *model, gint item_index,
                                                       GHashTable **attributes)
{
	UnityGtkMenuSection *section;
	UnityGtkMenuShell *parent_shell;
	UnityGtkMenuItem *item;
	GSequenceIter *iter;
	guint index;
	const char *label;
	GIcon *icon;
	UnityGtkAction *action;

	g_return_if_fail(UNITY_GTK_IS_MENU_SECTION(model));
	g_return_if_fail(attributes != NULL);

	section      = UNITY_GTK_MENU_SECTION(model);
	parent_shell = section->parent_shell;

	g_return_if_fail(parent_shell != NULL);

	iter   = unity_gtk_menu_section_get_iter(section, item_index);
	index  = GPOINTER_TO_UINT(g_sequence_get(iter));
	item   = unity_gtk_menu_shell_get_item(parent_shell, index);
	label  = unity_gtk_menu_item_get_label(item);
	icon   = unity_gtk_menu_item_get_icon(item);
	action = item->action;

	*attributes =
	    g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_variant_unref);

	if (label != NULL)
		g_hash_table_insert(*attributes,
		                    G_MENU_ATTRIBUTE_LABEL,
		                    g_variant_ref_sink(g_variant_new_string(label)));

	if (icon != NULL)
	{
		g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_ICON, g_icon_serialize(icon));
		g_object_unref(icon);
	}

	if (action != NULL)
	{
		if (action->name != NULL)
		{
			char *name        = g_strdup_printf("unity.%s", action->name);
			GVariant *variant = g_variant_ref_sink(g_variant_new_string(name));

			g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_ACTION, variant);

			if (action->items_by_name != NULL)
			{
				GHashTableIter iter;
				gpointer key;
				gpointer value;
				const char *target = NULL;

				g_hash_table_iter_init(&iter, action->items_by_name);
				while (target == NULL &&
				       g_hash_table_iter_next(&iter, &key, &value))
					if (value == item)
						target = key;

				if (target != NULL)
					g_hash_table_insert(*attributes,
					                    G_MENU_ATTRIBUTE_TARGET,
					                    g_variant_ref_sink(
					                        g_variant_new_string(target)));
			}
			else if (unity_gtk_menu_item_get_draw_as_radio(item))
				g_hash_table_insert(*attributes,
				                    G_MENU_ATTRIBUTE_TARGET,
				                    g_variant_ref_sink(
				                        g_variant_new_string(action->name)));

			g_free(name);
		}

		if (action->subname != NULL)
		{
			char *subname     = g_strdup_printf("unity.%s", action->subname);
			GVariant *variant = g_variant_ref_sink(g_variant_new_string(subname));
			g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_SUBMENU_ACTION, variant);
			g_free(subname);
		}
	}

	if (item->menu_item != NULL)
	{
		char *accel_name       = NULL;
		const char *accel_path = gtk_menu_item_get_accel_path(item->menu_item);

		if (accel_path != NULL)
		{
			GtkAccelKey accel_key;

			if (gtk_accel_map_lookup_entry(accel_path, &accel_key))
				accel_name =
				    gtk_accelerator_name(accel_key.accel_key, accel_key.accel_mods);
		}

		if (accel_name == NULL)
		{
			GList *closures =
			    gtk_widget_list_accel_closures(GTK_WIDGET(item->menu_item));
			GList *iter;

			for (iter = closures; iter != NULL && accel_name == NULL;
			     iter = g_list_next(iter))
			{
				GClosure *closure = iter->data;
				GtkAccelGroup *accel_group =
				    gtk_accel_group_from_accel_closure(closure);

				if (accel_group != NULL)
				{
					GtkAccelKey *accel_key =
					    gtk_accel_group_find(accel_group,
					                         g_closure_equal,
					                         closure);

					if (accel_key != NULL)
						accel_name =
						    gtk_accelerator_name(accel_key->accel_key,
						                         accel_key->accel_mods);
				}
			}

			g_list_free(closures);
		}

		if (accel_name != NULL)
			g_hash_table_insert(*attributes,
			                    G_MENU_ATTRIBUTE_ACCEL,
			                    g_variant_ref_sink(g_variant_new_string(accel_name)));
		else
		{
#if GTK_MAJOR_VERSION == 2
			/* LP: #1208019 */
			GtkLabel *accel_label = gtk_menu_item_get_nth_label(item->menu_item, 0);

			if (GTK_IS_ACCEL_LABEL(accel_label))
			{
				/* Eclipse uses private API. */
				if (GTK_ACCEL_LABEL(accel_label)->accel_string != NULL)
					accel_name =
					    g_strdup(GTK_ACCEL_LABEL(accel_label)->accel_string);
			}
#endif

			if (accel_name == NULL)
				accel_name =
				    g_strdup(gtk_menu_item_get_nth_label_label(item->menu_item, 1));

			if (accel_name != NULL)
				g_hash_table_insert(*attributes,
				                    G_MENU_ATTRIBUTE_ACCEL_TEXT,
				                    g_variant_ref_sink(
				                        g_variant_new_string(accel_name)));
		}

		g_free(accel_name);
	}
}

static void unity_gtk_menu_section_get_item_links(GMenuModel *model, gint item_index,
                                                  GHashTable **links)
{
	UnityGtkMenuSection *section;
	UnityGtkMenuShell *parent_shell;
	UnityGtkMenuItem *item;
	GSequenceIter *iter;
	guint index;
	UnityGtkMenuShell *child_shell;

	g_return_if_fail(UNITY_GTK_IS_MENU_SECTION(model));
	g_return_if_fail(links != NULL);

	section      = UNITY_GTK_MENU_SECTION(model);
	parent_shell = section->parent_shell;

	g_return_if_fail(parent_shell != NULL);

	iter        = unity_gtk_menu_section_get_iter(section, item_index);
	index       = GPOINTER_TO_UINT(g_sequence_get(iter));
	item        = unity_gtk_menu_shell_get_item(parent_shell, index);
	child_shell = unity_gtk_menu_item_get_child_shell(item);

	*links = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);

	if (child_shell != NULL)
		g_hash_table_insert(*links, G_MENU_LINK_SUBMENU, g_object_ref(child_shell));
}

static void unity_gtk_menu_section_class_init(UnityGtkMenuSectionClass *klass)
{
	GObjectClass *object_class        = G_OBJECT_CLASS(klass);
	GMenuModelClass *menu_model_class = G_MENU_MODEL_CLASS(klass);

	object_class->dispose                 = unity_gtk_menu_section_dispose;
	menu_model_class->is_mutable          = unity_gtk_menu_section_is_mutable;
	menu_model_class->get_n_items         = unity_gtk_menu_section_get_n_items;
	menu_model_class->get_item_attributes = unity_gtk_menu_section_get_item_attributes;
	menu_model_class->get_item_links      = unity_gtk_menu_section_get_item_links;
}

static void unity_gtk_menu_section_init(UnityGtkMenuSection *self)
{
}

UnityGtkMenuSection *unity_gtk_menu_section_new(UnityGtkMenuShell *parent_shell,
                                                guint section_index)
{
	UnityGtkMenuSection *section = g_object_new(UNITY_GTK_TYPE_MENU_SECTION, NULL);

	unity_gtk_menu_section_set_parent_shell(section, parent_shell);
	section->section_index = section_index;

	return section;
}

GSequenceIter *unity_gtk_menu_section_get_begin_iter(UnityGtkMenuSection *section)
{
	UnityGtkMenuShell *parent_shell;
	GSequence *separator_indices;
	GSequence *visible_indices;
	GSequenceIter *separator_iter;
	GSequenceIter *visible_iter;
	guint section_index;

	g_return_val_if_fail(UNITY_GTK_IS_MENU_SECTION(section), NULL);

	parent_shell = section->parent_shell;

	g_return_val_if_fail(parent_shell != NULL, NULL);

	separator_indices = unity_gtk_menu_shell_get_separator_indices(parent_shell);
	visible_indices   = unity_gtk_menu_shell_get_visible_indices(parent_shell);
	section_index     = section->section_index;

	if (section_index > 0)
		separator_iter = g_sequence_get_iter_at_pos(separator_indices, section_index - 1);
	else
		separator_iter = NULL;

	if (separator_iter != NULL)
	{
		gpointer separator_index = g_sequence_get(separator_iter);
		visible_iter = g_sequence_lookup(visible_indices, separator_index, g_uintcmp, NULL);
		visible_iter = g_sequence_iter_next(visible_iter);
	}
	else
		visible_iter = g_sequence_get_begin_iter(visible_indices);

	return visible_iter;
}

GSequenceIter *unity_gtk_menu_section_get_end_iter(UnityGtkMenuSection *section)
{
	UnityGtkMenuShell *parent_shell;
	GSequence *separator_indices;
	GSequence *visible_indices;
	GSequenceIter *separator_iter;
	GSequenceIter *visible_iter;

	g_return_val_if_fail(UNITY_GTK_IS_MENU_SECTION(section), NULL);

	parent_shell = section->parent_shell;

	g_return_val_if_fail(parent_shell != NULL, NULL);

	separator_indices = unity_gtk_menu_shell_get_separator_indices(parent_shell);
	visible_indices   = unity_gtk_menu_shell_get_visible_indices(parent_shell);
	separator_iter    = g_sequence_get_iter_at_pos(separator_indices, section->section_index);

	if (g_sequence_iter_is_end(separator_iter))
		separator_iter = NULL;

	if (separator_iter != NULL)
		visible_iter = g_sequence_lookup(visible_indices,
		                                 g_sequence_get(separator_iter),
		                                 g_uintcmp,
		                                 NULL);
	else
		visible_iter = g_sequence_get_end_iter(visible_indices);

	return visible_iter;
}

GSequenceIter *unity_gtk_menu_section_get_iter(UnityGtkMenuSection *section, guint index)
{
	g_return_val_if_fail(UNITY_GTK_IS_MENU_SECTION(section), NULL);

	return g_sequence_iter_move(unity_gtk_menu_section_get_begin_iter(section), index);
}

void unity_gtk_menu_section_print(UnityGtkMenuSection *section, guint indent)
{
	char *space;

	g_return_if_fail(section == NULL || UNITY_GTK_IS_MENU_SECTION(section));

	space = g_strnfill(indent, ' ');

	if (section != NULL)
	{
		g_print("%s%u (%s *) %p\n",
		        space,
		        section->section_index,
		        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(section)),
		        section);

		if (section->parent_shell != NULL)
			g_print("%s  (%s *) %p\n",
			        space,
			        G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(section->parent_shell)),
			        section->parent_shell);
	}
	else
		g_print("%sNULL\n", space);

	g_free(space);
}
