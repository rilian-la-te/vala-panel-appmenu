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

#include <inttypes.h>

#include "debug.h"
#include "definitions.h"
#include "item.h"
#include "model.h"
#include "section.h"

struct _DBusMenuModel
{
	GMenuModel parent_instance;

	uint parent_id;
	uint current_revision;
	GCancellable *cancellable;
	DBusMenuXml *xml;
	GActionGroup *received_action_group;
	GSequence *items;
	uint full_update_signal;
	GQueue *full_update_queue;
	bool layout_update_required;
	bool layout_update_in_progress;
};

static const char *property_names[] = { "accessible-desc",
	                                "children-display",
	                                "disposition",
	                                "enabled",
	                                "icon-data",
	                                "icon-name",
	                                "label",
	                                "shortcut",
	                                "toggle-type",
	                                "toggle-state",
	                                "type",
	                                "visible",
	                                NULL };
enum
{
	PROP_NULL         = 0,
	PROP_XML          = 1,
	PROP_ACTION_GROUP = 2,
	PROP_PARENT_ID    = 3,
	NUM_PROPS
};

enum
{
	UPDATE_MODE_IMMEDIATE = 0,
	UPDATE_MODE_DEFERRED  = 1,
	UPDATE_MODE_FULL      = 2
};

static GParamSpec *properties[NUM_PROPS] = { NULL };

static DBusMenuItem *dbus_menu_model_find(DBusMenuModel *menu, uint item_id);
static DBusMenuItem *dbus_menu_model_find_section(DBusMenuModel *menu, uint section_num);
static GSequenceIter *dbus_menu_model_find_place(DBusMenuModel *menu, uint section_num, int place);

G_DEFINE_TYPE(DBusMenuModel, dbus_menu_model, G_TYPE_MENU_MODEL)

static gint dbus_menu_model_get_n_items(GMenuModel *model)
{
	DBusMenuModel *menu      = (DBusMenuModel *)(model);
	GSequenceIter *last_iter = g_sequence_iter_prev(g_sequence_get_end_iter(menu->items));
	DBusMenuItem *last       = g_sequence_get(last_iter);
	return last->section_num + 1;
}

static void dbus_menu_model_get_item_attributes(GMenuModel *model, gint position,
                                                GHashTable **table)
{
	DBusMenuModel *menu = DBUS_MENU_MODEL(model);
	for (GSequenceIter *iter = g_sequence_get_begin_iter(menu->items);
	     !g_sequence_iter_is_end(iter);
	     iter = g_sequence_iter_next(iter))
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(iter);
		if (item->section_num == position && item->place == -1)
		{
			*table = g_hash_table_ref(item->attrs);
			return;
		}
	}
}

static void dbus_menu_model_get_item_links(GMenuModel *model, gint position, GHashTable **table)
{
	DBusMenuModel *menu = DBUS_MENU_MODEL(model);
	for (GSequenceIter *iter = g_sequence_get_begin_iter(menu->items);
	     !g_sequence_iter_is_end(iter);
	     iter = g_sequence_iter_next(iter))
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(iter);
		if (item->section_num == position && item->place == -1)
		{
			*table = g_hash_table_ref(item->links);
			return;
		}
	}
}

static int dbus_menu_model_is_mutable(GMenuModel *model)
{
	return true;
}

struct layout_data
{
	GMenuModel *model;
	int section_num;
	uint pos;
	uint old_num;
	uint new_num;
};

GSequence *dbus_menu_model_items(DBusMenuModel *model)
{
	return model->items;
}

int queue_compare_func(const struct layout_data *a, const struct layout_data *b)
{
	if (a->model != b->model)
		return DBUS_MENU_IS_MODEL(a->model) ? -1 : 1;
	else if (a->old_num != b->old_num)
		return b->old_num - a->old_num;
	else if (a->new_num != b->new_num)
		return b->new_num - a->new_num;
	else if (a->pos != b->pos)
		return b->pos - a->pos;
	return 0;
}

static int dbus_menu_model_sort_func(gconstpointer a, gconstpointer b,
                                     G_GNUC_UNUSED void *user_data)
{
	DBusMenuItem *aitem = (DBusMenuItem *)a;
	DBusMenuItem *bitem = (DBusMenuItem *)b;

	if (bitem->section_num != aitem->section_num)
		return aitem->section_num - bitem->section_num;

	return aitem->place - bitem->place;
}

static void add_signal_to_queue(DBusMenuModel *model, GQueue *queue, int sect_num, int pos,
                                int removed, int added)
{
	struct layout_data *data = g_new0(struct layout_data, 1);
	if (sect_num >= 0)
	{
		DBusMenuItem *item = dbus_menu_model_find_section(model, sect_num);
		data->model = G_MENU_MODEL(g_hash_table_lookup(item->links, G_MENU_LINK_SECTION));
	}
	else
	{
		data->model = G_MENU_MODEL(model);
	}
	data->section_num = sect_num;
	data->pos         = pos;
	data->old_num     = removed;
	data->new_num     = added;
	gpointer l        = g_queue_find_custom(queue, data, (GCompareFunc)queue_compare_func);
	if (!l)
		g_queue_push_head(queue, data);
}

static void queue_emit_idle(GQueue *queue)
{
	struct layout_data *index = NULL;
	while ((index = (struct layout_data *)g_queue_pop_head(queue)))
	{
		g_menu_model_items_changed(index->model,
		                           index->pos,
		                           index->old_num,
		                           index->new_num);
		g_free(index);
	}
}

static bool queue_emit_full(DBusMenuModel *self)
{
	g_return_val_if_fail(DBUS_MENU_IS_MODEL(self), G_SOURCE_REMOVE);
	g_return_val_if_fail(self->full_update_queue, G_SOURCE_REMOVE);

	queue_emit_idle(self->full_update_queue);
	g_clear_pointer(&self->full_update_queue, g_queue_free);
	self->full_update_signal = 0;
	return G_SOURCE_REMOVE;
}

static void queue_emit_all(DBusMenuModel *self, GQueue *queue, int update_mode)
{
	if (update_mode == UPDATE_MODE_FULL)
		self->full_update_queue = queue;
	if (!self->full_update_signal)
	{
		if (update_mode == UPDATE_MODE_FULL)
			self->full_update_signal = g_timeout_add_full(G_PRIORITY_HIGH,
			                                              350,
			                                              (GSourceFunc)queue_emit_full,
			                                              self,
			                                              NULL);
		else if (update_mode == UPDATE_MODE_IMMEDIATE)
			queue_emit_idle(queue);
	}
}

static bool preload_idle(DBusMenuItem *item)
{
	dbus_menu_item_preload(item);
	return G_SOURCE_REMOVE;
}

static void menu_item_copy_and_load(DBusMenuModel *menu, DBusMenuItem *old, DBusMenuItem *new_item)
{
	dbus_menu_item_copy_submenu(old, new_item, menu);
	dbus_menu_item_generate_action(new_item, menu);
	// It is a preload hack. If this is a toplevel menu, we need to fetch menu under toplevel to
	// avoid menu jumping bug. Now we need to use it for all menus - no menus are preloaded,
	// AFAIK
	dbus_menu_item_update_enabled(new_item, true);
	new_item->toggled = true;
	g_timeout_add_full(100, 300, (GSourceFunc)preload_idle, new_item, NULL);
}

// We deal only with layouts with depth 1 (not all)
static void layout_parse(DBusMenuModel *menu, GVariant *layout)
{
	guint id;
	GVariant *props;
	GVariant *items;
	if (!g_variant_is_of_type(layout, G_VARIANT_TYPE("(ia{sv}av)")))
	{
		g_warning(
		    "Type of return value for 'layout' property in "
		    "'GetLayout' call should be '(ia{sv}av)' but got '%s'",
		    g_variant_get_type_string(layout));

		return;
	}
	if (menu->layout_update_in_progress)
		return;
	menu->layout_update_required    = false;
	menu->layout_update_in_progress = true;
	g_variant_get(layout, "(i@a{sv}@av)", &id, &props, &items);
	g_variant_unref(props);
	GQueue *signal_queue = g_queue_new();
	GVariantIter iter;
	GVariant *child;
	// Start parsing. We need to track section number, and also GSequenceIter to
	// current section. Also we track change position, number of added and removed
	// items for current section
	uint section_num            = 0;
	uint place                  = 0;
	uint old_sections           = g_menu_model_get_n_items(G_MENU_MODEL(menu));
	uint added                  = 0;
	int change_pos              = -1;
	GSequenceIter *current_iter = g_sequence_get_begin_iter(menu->items);
	g_variant_iter_init(&iter, items);
	while ((child = g_variant_iter_next_value(&iter)))
	{
		GVariant *value = g_variant_get_variant(child);
		guint cid;
		GVariant *cprops;
		GVariant *citems;
		g_variant_get(value, "(i@a{sv}@av)", &cid, &cprops, &citems);
		g_variant_unref(citems);

		DBusMenuItem *old      = NULL;
		DBusMenuItem *new_item = dbus_menu_item_new(cid, menu, cprops);
		// We receive a section (separator or x-kde-title)
		if (new_item->action_type == DBUS_MENU_ACTION_SECTION)
		{
			bool is_valid_section = !new_item->toggled && place > 0;
			// Section is valid, so, parse it
			if (is_valid_section)
			{
				// Do some common tasks: increment section_num and iter
				++section_num;
				place--;
				new_item->section_num = section_num;
				new_item->place       = -1;
				GSequenceIter *old_iter =
				    g_sequence_lookup(menu->items,
				                      new_item,
				                      dbus_menu_model_sort_func,
				                      NULL);
				if (!old_iter)
				{
					g_hash_table_insert(
					    new_item->links,
					    G_MENU_LINK_SECTION,
					    dbus_menu_section_model_new(menu, section_num));
					old_iter =
					    g_sequence_insert_sorted(menu->items,
					                             new_item,
					                             dbus_menu_model_sort_func,
					                             NULL);
				}
				else
					dbus_menu_item_free(new_item);
				old =
				    (DBusMenuItem *)g_sequence_get(g_sequence_iter_prev(old_iter));
				int delta                 = old->place - place;
				GSequenceIter *place_iter = g_sequence_iter_move(old_iter, -delta);
				// Cleanup all items in prev section (if there is more items than
				// current)
				if (delta > 0)
					g_sequence_remove_range(place_iter, old_iter);
				// Update current_section and reset current_iter and added to new
				// section
				current_iter = g_sequence_iter_next(old_iter);
				added        = 0;
				change_pos   = -1;
				place        = 0;
			}
			// If section was invalid, just free received item.
			else
				dbus_menu_item_free(new_item);
		}
		else if (!dbus_menu_item_is_firefox_stub(new_item))
		{
			new_item->section_num   = section_num;
			new_item->place         = place;
			GSequenceIter *old_iter = g_sequence_lookup(menu->items,
			                                            new_item,
			                                            dbus_menu_model_sort_func,
			                                            NULL);
			// There is no old item on this place
			if (!old_iter)
			{
				if (!added)
					change_pos = change_pos < 0 ? place : change_pos;
				menu_item_copy_and_load(menu, NULL, new_item);
				current_iter = g_sequence_insert_sorted(menu->items,
				                                        new_item,
				                                        dbus_menu_model_sort_func,
				                                        NULL);
				added++;
			}
			// If there is an old item exists, we need to check this properties
			else
			{
				old = (DBusMenuItem *)g_sequence_get(old_iter);
				// We should compare properties of old and new item
				bool diff = !dbus_menu_item_compare_immutable(old, new_item);
				dbus_menu_item_update_props(old, cprops);
				if (diff)
				{
					// Immutable properties was different, replace menu item
					menu_item_copy_and_load(menu, old, new_item);
					g_sequence_remove(old_iter);
					current_iter =
					    g_sequence_insert_sorted(menu->items,
					                             new_item,
					                             dbus_menu_model_sort_func,
					                             NULL);
				}
				else
				{
					// Just free unneeded item
					dbus_menu_item_free(new_item);
				}
			}
			current_iter = g_sequence_iter_next(current_iter);
			place++;
		}
		else
			// Just free unnedeed item
			dbus_menu_item_free(new_item);
		g_variant_unref(cprops);
		g_variant_unref(value);
		g_variant_unref(child);
	}
	section_num++;
	int secdiff = old_sections - section_num;
	if (secdiff > 0)
	{
		GSequenceIter *section_iter = dbus_menu_model_find_place(menu, section_num, -1);
		g_sequence_remove_range(section_iter, g_sequence_get_end_iter(menu->items));
	}
	// We need to manage last section's changes. And check its validity
	if (secdiff >= 0)
	{
		place--;
		DBusMenuItem *old = (DBusMenuItem *)g_sequence_get(
		    g_sequence_iter_prev(g_sequence_get_end_iter(menu->items)));
		int delta                 = old->place - place;
		GSequenceIter *last_iter  = g_sequence_get_end_iter(menu->items);
		GSequenceIter *place_iter = g_sequence_iter_move(last_iter, -delta);

		// Cleanup all items in prev section (if there is more items than
		// current)
		if (delta > 0)
			g_sequence_remove_range(place_iter, last_iter);
	}
	add_signal_to_queue(menu, signal_queue, -1, 0, old_sections, section_num);
	g_variant_unref(items);
	// Emit all signals from queue by LIFO order
	queue_emit_all(menu, signal_queue, UPDATE_MODE_FULL);
}

static void get_layout_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	g_autoptr(GVariant) layout = NULL;
	guint revision;
	if (!DBUS_MENU_IS_MODEL(user_data))
		return;
	DBusMenuModel *menu     = DBUS_MENU_MODEL(user_data);
	g_autoptr(GError) error = NULL;
	dbus_menu_xml_call_get_layout_finish((DBusMenuXml *)(source_object),
	                                     &revision,
	                                     &layout,
	                                     res,
	                                     &error);
	if (error != NULL)
	{
		if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning("%s", error->message);
		return;
	}
	layout_parse(menu, layout);
	menu->layout_update_in_progress = false;
	if (menu->layout_update_required)
		dbus_menu_model_update_layout(menu);
}

static void dbus_menu_update_item_properties_from_layout_sync(DBusMenuModel *menu,
                                                              DBusMenuItem *item, int sect_n,
                                                              int pos)
{
	g_return_if_fail(DBUS_MENU_IS_MODEL(menu));
	g_autoptr(GVariant) props  = NULL;
	g_autoptr(GVariant) items  = NULL;
	g_autoptr(GVariant) layout = NULL;
	g_autoptr(GError) error    = NULL;
	GQueue *signal_queue       = g_queue_new();
	guint id, revision;
	dbus_menu_xml_call_get_layout_sync(menu->xml,
	                                   item->id,
	                                   0,
	                                   property_names,
	                                   &revision,
	                                   &layout,
	                                   menu->cancellable,
	                                   &error);
	if (error != NULL)
	{
		if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning("%s", error->message);
		return;
	}
	g_variant_get(layout, "(i@a{sv}@av)", &id, &props, &items);
	bool is_item_updated = dbus_menu_item_update_props(item, props);
	if (is_item_updated)
		add_signal_to_queue(menu, signal_queue, sect_n, pos, 1, 1);
	queue_emit_all(menu, signal_queue, UPDATE_MODE_DEFERRED);
}

G_GNUC_INTERNAL void dbus_menu_model_update_layout(DBusMenuModel *menu)
{
	g_return_if_fail(DBUS_MENU_IS_MODEL(menu));
	if (menu->layout_update_in_progress)
		menu->layout_update_required = true;
	else
		dbus_menu_xml_call_get_layout(menu->xml,
		                              menu->parent_id,
		                              1,
		                              property_names,
		                              menu->cancellable,
		                              get_layout_cb,
		                              menu);
}

static void layout_updated_cb(DBusMenuXml *proxy, guint revision, gint parent, DBusMenuModel *menu)
{
	g_return_if_fail(DBUS_MENU_IS_XML(proxy));
	g_return_if_fail(DBUS_MENU_IS_MODEL(menu));

	if (((uint)parent == menu->parent_id) && menu->current_revision < revision)
	{
		g_debug("Remote attempt to update %u with rev %u\n", parent, revision);
		dbus_menu_model_update_layout(menu);
		menu->current_revision = revision;
		return;
	}
	DBusMenuItem *item = dbus_menu_model_find(menu, (uint)parent);
	if (item != NULL)
		dbus_menu_update_item_properties_from_layout_sync(menu,
		                                                  item,
		                                                  item->section_num,
		                                                  item->place);
}

static void item_activation_requested_cb(DBusMenuXml *proxy, gint id, guint timestamp,
                                         DBusMenuModel *menu)
{
	if (!DBUS_MENU_IS_XML(proxy))
		return;
	g_autofree char *ordinary_name = g_strdup_printf(ACTION_PREFIX "%u", id);
	g_action_group_activate_action(menu->received_action_group, ordinary_name, NULL);
	g_debug("activation requested: id - %d, timestamp - %d", id, timestamp);
}

static void items_properties_loop(DBusMenuModel *menu, GVariant *up_props, GQueue *signal_queue,
                                  bool is_removal)
{
	GVariantIter iter;
	guint id;
	GVariant *props;
	g_variant_iter_init(&iter, up_props);
	while (g_variant_iter_loop(&iter, !is_removal ? "(i@a{sv})" : "(i@as)", &id, &props))
	{
		DBusMenuItem *item   = (DBusMenuItem *)dbus_menu_model_find(menu, id);
		bool is_item_updated = false;
		if (item != NULL)
		{
			// It is the best what we can do to update a section
			if (item->action_type == DBUS_MENU_ACTION_SECTION)
			{
				//                            dbus_menu_model_update_layout(menu);
			}
			else
			{
				is_item_updated = !is_removal
				                      ? dbus_menu_item_update_props(item, props)
				                      : dbus_menu_item_remove_props(item, props);
				if (is_item_updated)
					add_signal_to_queue(menu,
					                    signal_queue,
					                    item->section_num,
					                    item->place,
					                    1,
					                    1);
			}
		}
	}
}

static void items_properties_updated_cb(DBusMenuXml *proxy, GVariant *updated_props,
                                        GVariant *removed_props, DBusMenuModel *menu)
{
	if (!DBUS_MENU_IS_XML(proxy))
		return;
	if (menu->layout_update_in_progress == true)
		return;
	g_autoptr(GQueue) signal_queue = g_queue_new();
	items_properties_loop(menu, updated_props, signal_queue, false);
	items_properties_loop(menu, removed_props, signal_queue, true);
	queue_emit_all(menu, signal_queue, UPDATE_MODE_DEFERRED);
}

static void on_xml_property_changed(DBusMenuModel *model)
{
	if (!DBUS_MENU_IS_XML(model->xml))
		return;
	g_signal_connect(model->xml,
	                 "items-properties-updated",
	                 G_CALLBACK(items_properties_updated_cb),
	                 model);
	g_signal_connect(model->xml, "layout-updated", G_CALLBACK(layout_updated_cb), model);
	g_signal_connect(model->xml,
	                 "item-activation-requested",
	                 G_CALLBACK(item_activation_requested_cb),
	                 model);
	if (model->parent_id == 0)
		dbus_menu_model_update_layout(model);
}

G_GNUC_INTERNAL DBusMenuModel *dbus_menu_model_new(uint parent_id, DBusMenuModel *parent,
                                                   DBusMenuXml *xml, GActionGroup *action_group)
{
	DBusMenuModel *ret = (DBusMenuModel *)g_object_new(dbus_menu_model_get_type(),
	                                                   "parent-id",
	                                                   parent_id,
	                                                   "xml",
	                                                   xml,
	                                                   "action-group",
	                                                   action_group,
	                                                   NULL);
	if (parent != NULL)
		g_object_bind_property(parent, "xml", ret, "xml", G_BINDING_SYNC_CREATE);
	return ret;
}

static void dbus_menu_model_set_property(GObject *object, guint property_id, const GValue *value,
                                         GParamSpec *pspec)
{
	DBusMenuModel *menu = (DBusMenuModel *)(object);
	void *old_xml       = menu->xml;

	switch (property_id)
	{
	case PROP_XML:
		menu->xml = DBUS_MENU_XML(g_value_get_object(value));
		if (menu->xml != NULL && old_xml != menu->xml)
		{
			if (old_xml != NULL)
				g_signal_handlers_disconnect_by_data(old_xml, menu);
			on_xml_property_changed(menu);
		}
		break;
	case PROP_ACTION_GROUP:
		menu->received_action_group = G_ACTION_GROUP(g_value_get_object(value));
		break;
	case PROP_PARENT_ID:
		menu->layout_update_required = true;
		menu->parent_id              = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void dbus_menu_model_get_property(GObject *object, guint property_id, GValue *value,
                                         GParamSpec *pspec)
{
	DBusMenuModel *menu;

	menu = (DBusMenuModel *)(object);

	switch (property_id)
	{
	case PROP_XML:
		g_value_set_object(value, menu->xml);
		break;
	case PROP_PARENT_ID:
		g_value_set_uint(value, menu->parent_id);
		break;
	case PROP_ACTION_GROUP:
		g_value_set_object(value, menu->received_action_group);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

G_GNUC_INTERNAL bool dbus_menu_model_is_layout_update_required(DBusMenuModel *model)
{
	return model->layout_update_required;
}

static DBusMenuItem *dbus_menu_model_find(DBusMenuModel *menu, uint item_id)
{
	for (GSequenceIter *iter = g_sequence_get_begin_iter(menu->items);
	     !g_sequence_iter_is_end(iter);
	     iter = g_sequence_iter_next(iter))
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(iter);
		if (item->id == item_id)
			return item;
	}
	return NULL;
}

static GSequenceIter *dbus_menu_model_find_place(DBusMenuModel *menu, uint section_num, int place)
{
	for (GSequenceIter *iter = g_sequence_get_begin_iter(menu->items);
	     !g_sequence_iter_is_end(iter);
	     iter = g_sequence_iter_next(iter))
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(iter);
		if (item->section_num == section_num && item->place == place)
			return iter;
	}
	return NULL;
}

static DBusMenuItem *dbus_menu_model_find_section(DBusMenuModel *menu, uint section_num)
{
	return (DBusMenuItem *)g_sequence_get(dbus_menu_model_find_place(menu, section_num, -1));
}

static void dbus_menu_model_init(DBusMenuModel *menu)
{
	menu->cancellable               = g_cancellable_new();
	menu->parent_id                 = UINT_MAX;
	menu->items                     = g_sequence_new(dbus_menu_item_free);
	menu->layout_update_required    = true;
	menu->layout_update_in_progress = false;
	menu->full_update_queue         = NULL;
	menu->full_update_signal        = 0;
	menu->current_revision          = 0;
}

static void dbus_menu_model_constructed(GObject *object)
{
	G_OBJECT_CLASS(dbus_menu_model_parent_class)->constructed(object);
	DBusMenuModel *menu = DBUS_MENU_MODEL(object);

	DBusMenuItem *first_section =
	    dbus_menu_item_new_first_section(menu->parent_id, menu->received_action_group);
	first_section->section_num = 0;
	first_section->place       = -1;
	g_hash_table_insert(first_section->links,
	                    G_MENU_LINK_SECTION,
	                    dbus_menu_section_model_new(menu, 0));
	g_sequence_insert_sorted(menu->items, first_section, dbus_menu_model_sort_func, NULL);
}

static void dbus_menu_model_finalize(GObject *object)
{
	DBusMenuModel *menu = (DBusMenuModel *)(object);
	if (menu->xml)
		g_signal_handlers_disconnect_by_data(menu->xml, menu);
	if (menu->full_update_signal)
		g_source_remove(menu->full_update_signal);
	if (menu->full_update_queue)
		g_queue_free_full(menu->full_update_queue, g_free);
	g_cancellable_cancel(menu->cancellable);
	g_clear_object(&menu->cancellable);
	g_clear_pointer(&menu->items, g_sequence_free);

	G_OBJECT_CLASS(dbus_menu_model_parent_class)->finalize(object);
}

static void install_properties(GObjectClass *object_class)
{
	properties[PROP_XML] =
	    g_param_spec_object("xml",
	                        "xml",
	                        "xml",
	                        dbus_menu_xml_get_type(),
	                        (GParamFlags)(G_PARAM_CONSTRUCT | G_PARAM_READABLE |
	                                      G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

	properties[PROP_ACTION_GROUP] =
	    g_param_spec_object("action-group",
	                        "action-group",
	                        "action-group",
	                        g_action_group_get_type(),
	                        (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
	                                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
	properties[PROP_PARENT_ID] =
	    g_param_spec_uint("parent-id",
	                      "parent-id",
	                      "parent-id",
	                      0,
	                      UINT_MAX,
	                      0,
	                      (GParamFlags)(G_PARAM_CONSTRUCT | G_PARAM_WRITABLE |
	                                    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_properties(object_class, NUM_PROPS, properties);
}

static void dbus_menu_model_class_init(DBusMenuModelClass *klass)
{
	GMenuModelClass *model_class = G_MENU_MODEL_CLASS(klass);
	GObjectClass *object_class   = G_OBJECT_CLASS(klass);

	object_class->finalize     = dbus_menu_model_finalize;
	object_class->set_property = dbus_menu_model_set_property;
	object_class->get_property = dbus_menu_model_get_property;
	object_class->constructed  = dbus_menu_model_constructed;

	model_class->is_mutable          = dbus_menu_model_is_mutable;
	model_class->get_n_items         = dbus_menu_model_get_n_items;
	model_class->get_item_attributes = dbus_menu_model_get_item_attributes;
	model_class->get_item_links      = dbus_menu_model_get_item_links;
	install_properties(object_class);
}
