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

#include "section.h"
#include "item.h"
enum
{
	PROP_NULL          = 0,
	PROP_PARENT_MODEL  = 1,
	PROP_SECTION_INDEX = 2,
	NUM_PROPS
};

static GParamSpec *properties[NUM_PROPS] = { NULL };

G_DEFINE_TYPE(DBusMenuSectionModel, dbus_menu_section_model, G_TYPE_MENU_MODEL)

static int dbus_menu_section_model_is_mutable(GMenuModel *model)
{
	return true;
}

static gint dbus_menu_section_model_get_n_items(GMenuModel *model)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(model);
	GSequence *items           = dbus_menu_model_items(menu->parent_model);
	int begin = 0, end = -1;
	for (GSequenceIter *iter = g_sequence_get_begin_iter(items); !g_sequence_iter_is_end(iter);
	     iter                = g_sequence_iter_next(iter))
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(iter);
		if (item->section_num == menu->section_index && item->place == -1)
			begin = g_sequence_iter_get_position(iter);
		end = g_sequence_iter_get_position(iter);
		if (item->section_num == menu->section_index + 1 && item->place == -1)
		{
			end--;
			break;
		}
	}
	return end - begin;
}

static void dbus_menu_section_model_get_item_attributes(GMenuModel *model, gint position,
                                                        GHashTable **table)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(model);
	GSequence *items           = dbus_menu_model_items(menu->parent_model);
	for (GSequenceIter *iter = g_sequence_get_begin_iter(items); !g_sequence_iter_is_end(iter);
	     iter                = g_sequence_iter_next(iter))
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(iter);
		if (item->section_num == menu->section_index && item->place == position)
		{
			*table = g_hash_table_ref(item->attrs);
			return;
		}
	}
}

static void dbus_menu_section_model_get_item_links(GMenuModel *model, gint position,
                                                   GHashTable **table)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(model);
	GSequence *items           = dbus_menu_model_items(menu->parent_model);
	for (GSequenceIter *iter = g_sequence_get_begin_iter(items); !g_sequence_iter_is_end(iter);
	     iter                = g_sequence_iter_next(iter))
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(iter);
		if (item->section_num == menu->section_index && item->place == position)
		{
			if (g_hash_table_contains(item->links, G_MENU_LINK_SECTION))
				g_warning("Item has section, but should not\n");
			*table = g_hash_table_ref(item->links);
			return;
		}
	}
}
static void dbus_menu_section_model_init(DBusMenuSectionModel *menu)
{
	menu->parent_model = NULL;
}

static void dbus_menu_section_model_finalize(GObject *object)
{
	G_OBJECT_CLASS(dbus_menu_section_model_parent_class)->finalize(object);
}

static void install_properties(GObjectClass *object_class)
{
	properties[PROP_PARENT_MODEL] =
	    g_param_spec_object("parent-model",
	                        "parent-model",
	                        "parent-model",
	                        dbus_menu_model_get_type(),
	                        (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READABLE |
	                                      G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

	properties[PROP_SECTION_INDEX] =
	    g_param_spec_uint("section-index",
	                      "section-index",
	                      "section-index",
	                      0,
	                      UINT_MAX,
	                      0,
	                      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
	                                    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_properties(object_class, NUM_PROPS, properties);
}

static void dbus_menu_section_model_set_property(GObject *object, guint property_id,
                                                 const GValue *value, GParamSpec *pspec)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(object);

	switch (property_id)
	{
	case PROP_PARENT_MODEL:
		menu->parent_model = DBUS_MENU_MODEL(g_value_get_object(value));
		break;
	case PROP_SECTION_INDEX:
		menu->section_index = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void dbus_menu_section_model_get_property(GObject *object, guint property_id, GValue *value,
                                                 GParamSpec *pspec)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(object);

	switch (property_id)
	{
	case PROP_PARENT_MODEL:
		g_value_set_object(value, menu->parent_model);
		break;
	case PROP_SECTION_INDEX:
		g_value_set_uint(value, menu->section_index);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void dbus_menu_section_model_constructed(GObject *object)
{
	G_OBJECT_CLASS(dbus_menu_section_model_parent_class)->constructed(object);
}

static void dbus_menu_section_model_class_init(DBusMenuSectionModelClass *klass)
{
	GMenuModelClass *model_class = G_MENU_MODEL_CLASS(klass);
	GObjectClass *object_class   = G_OBJECT_CLASS(klass);

	object_class->finalize     = dbus_menu_section_model_finalize;
	object_class->set_property = dbus_menu_section_model_set_property;
	object_class->get_property = dbus_menu_section_model_get_property;
	object_class->constructed  = dbus_menu_section_model_constructed;

	model_class->is_mutable          = dbus_menu_section_model_is_mutable;
	model_class->get_n_items         = dbus_menu_section_model_get_n_items;
	model_class->get_item_attributes = dbus_menu_section_model_get_item_attributes;
	model_class->get_item_links      = dbus_menu_section_model_get_item_links;
	install_properties(object_class);
}

DBusMenuSectionModel *dbus_menu_section_model_new(DBusMenuModel *parent, int section_index)
{
	return DBUS_MENU_SECTION_MODEL(g_object_new(dbus_menu_section_model_get_type(),
	                                            "parent-model",
	                                            parent,
	                                            "section-index",
	                                            section_index,
	                                            NULL));
}
