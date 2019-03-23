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
	GSequence *sections;
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

static GParamSpec *properties[NUM_PROPS] = { NULL };

static DBusMenuItem *dbus_menu_model_find(DBusMenuModel *menu, uint item_id, int *section_num,
                                          int *position);

G_DEFINE_TYPE(DBusMenuModel, dbus_menu_model, G_TYPE_MENU_MODEL)

static gint dbus_menu_model_get_n_items(GMenuModel *model)
{
	DBusMenuModel *menu = (DBusMenuModel *)(model);
	return (int)g_sequence_get_length(menu->sections);
}

static void dbus_menu_model_get_item_attributes(GMenuModel *model, gint position,
                                                GHashTable **table)
{
	DBusMenuModel *menu = DBUS_MENU_MODEL(model);
	DBusMenuItem *item  = (DBusMenuItem *)g_sequence_get(
            (GSequenceIter *)g_sequence_get_iter_at_pos(menu->sections, position));

	*table = g_hash_table_ref(item->attributes);
}

static void dbus_menu_model_get_item_links(GMenuModel *model, gint position, GHashTable **table)
{
	DBusMenuModel *menu = DBUS_MENU_MODEL(model);
	DBusMenuItem *item  = (DBusMenuItem *)g_sequence_get(
            (GSequenceIter *)g_sequence_get_iter_at_pos(menu->sections, position));

	*table = g_hash_table_ref(item->links);
}

static bool dbus_menu_model_is_mutable(GMenuModel *model)
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

static void add_signal_to_queue(DBusMenuModel *model, GQueue *queue, int sect_num, int pos,
                                int removed, int added)
{
	struct layout_data *data = g_new0(struct layout_data, 1);
	if (sect_num >= 0)
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(
		    g_sequence_get_iter_at_pos(model->sections, sect_num));
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
	gpointer l        = g_queue_find_custom(queue, data, queue_compare_func);
	if (!l)
		g_queue_push_head(queue, data);
}

static bool queue_emit_all(GQueue *queue)
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
	return G_SOURCE_REMOVE;
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
	// avoid menu jumping bug. Now we need to use it for all menus - no menus are preloadded,
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
	g_autoptr(GQueue) signal_queue = g_queue_new();
	GVariantIter iter;
	GVariant *child;
	// Start parsing. We need to track section number, and also GSequenceIter to
	// current section. Also we track change position, number of added and removed
	// items for current section
	uint section_num                      = 0;
	uint old_sections                     = g_sequence_get_length(menu->sections);
	uint added                            = 0;
	int change_pos                        = -1;
	bool on_border                        = true;
	GSequenceIter *sections_iter          = g_sequence_get_begin_iter(menu->sections);
	DBusMenuSectionModel *current_section = DBUS_MENU_SECTION_MODEL(
	    g_hash_table_lookup(((DBusMenuItem *)g_sequence_get(sections_iter))->links,
	                        G_MENU_LINK_SECTION));
	GSequenceIter *current_iter = g_sequence_get_begin_iter(current_section->items);
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
			bool is_valid_section = !new_item->toggled && !on_border;
			// If old section is empty - new section is invalid
			if (g_menu_model_get_n_items(current_section) == 0)
				is_valid_section = false;
			// Section is valid, so, parse it
			if (is_valid_section)
			{
				// Do some common tasks: increment section_num and iter
				section_num++;
				sections_iter = g_sequence_iter_next(sections_iter);
				on_border     = true;
				// If there is a last section in old model, append new section and
				// set iter
				if (g_sequence_iter_is_end(sections_iter))
				{
					dbus_menu_item_generate_action(new_item, menu);
					g_hash_table_insert(
					    new_item->links,
					    g_strdup(G_MENU_LINK_SECTION),
					    dbus_menu_section_model_new(menu, section_num));
					sections_iter =
					    g_sequence_insert_before(g_sequence_get_end_iter(
					                                 menu->sections),
					                             new_item);
				}
				// Just remove uneeded section.
				else
				{
					dbus_menu_item_free(new_item);
				}
				// Cleanup all items in prev section (if there is more items than
				// current)
				g_sequence_remove_range(current_iter,
				                        g_sequence_get_end_iter(
				                            current_section->items));
				int removed = g_sequence_iter_get_position(
				                  g_sequence_get_end_iter(current_section->items)) -
				              g_sequence_iter_get_position(current_iter);
				// If we already have this section in old layout, and items to this
				// section was added and/or removed, we add a signal to signal_queue
				// about this change. Else do nothing, section signal will do it for
				// us
				if ((removed > 0 || added > 0) && section_num <= old_sections)
				{
					add_signal_to_queue(menu,
					                    signal_queue,
					                    section_num - 1,
					                    change_pos,
					                    removed,
					                    added);
				}
				// Update current_section and reset current_iter and added to new
				// section
				current_section = DBUS_MENU_SECTION_MODEL(
				    g_hash_table_lookup(((DBusMenuItem *)g_sequence_get(
				                             sections_iter))
				                            ->links,
				                        G_MENU_LINK_SECTION));
				current_iter = g_sequence_get_begin_iter(current_section->items);
				added        = 0;
			}
			// If section was invalid, just free received item.
			else
				dbus_menu_item_free(new_item);
		}
		else if (!dbus_menu_item_is_firefox_stub(new_item))
		{
			// There is no section, but had less items in current_section now. Append
			// item
			if (g_sequence_iter_is_end(current_iter) ||
			    g_sequence_is_empty(g_sequence_iter_get_sequence(current_iter)))
			{
				if (added == 0)
					change_pos =
					    change_pos < 0
					        ? g_sequence_iter_get_position(current_iter)
					        : change_pos;

				menu_item_copy_and_load(menu, NULL, new_item);
				// Insert new item
				current_iter = g_sequence_insert_before(current_iter, new_item);
				added++;
			}
			// If there is an old item exists, we need to check this properties
			else
			{
				// We should compare properties of old and new item
				old       = (DBusMenuItem *)g_sequence_get(current_iter);
				bool diff = !dbus_menu_item_compare_immutable(old, new_item);
				// Update properties of old item
				bool updated = dbus_menu_item_update_props(old, cprops);
				if (diff)
				{
					// Immutable properties was different, replace menu item
					menu_item_copy_and_load(menu, old, new_item);
					g_sequence_set(current_iter, new_item);
				}
				else
				{
					// Just free unneeded item
					dbus_menu_item_free(new_item);
				}
				// If item was updated - add a signal to queue about it, but only if
				// section was in old layout
				if ((diff || updated) && section_num < old_sections)
				{
					add_signal_to_queue(menu,
					                    signal_queue,
					                    section_num,
					                    g_sequence_iter_get_position(
					                        current_iter),
					                    1,
					                    1);
				}
			}
			current_iter = g_sequence_iter_next(current_iter);
			on_border    = false;
		}
		else
			// Just free unnedede item
			dbus_menu_item_free(new_item);
		g_variant_unref(cprops);
		g_variant_unref(value);
		g_variant_unref(child);
	}
	// We need to manage last section's changes. And check its validity
	bool is_valid_section = !on_border || section_num == 0;
	// If old section is empty - new section is invalid
	if (g_menu_model_get_n_items(current_section) == 0 && g_menu_model_get_n_items(menu) > 1)
		is_valid_section = false;
	current_iter = g_sequence_iter_next(current_iter);
	int removed =
	    g_sequence_iter_get_position(g_sequence_get_end_iter(current_section->items)) -
	    g_sequence_iter_get_position(current_iter);
	g_sequence_remove_range(current_iter, g_sequence_get_end_iter(current_section->items));
	// Now calculate a sections changed signal
	if (is_valid_section)
		section_num++;
	int secdiff = old_sections - section_num;
	g_sequence_remove_range(g_sequence_get_iter_at_pos(menu->sections, section_num),
	                        g_sequence_get_end_iter(menu->sections));
	// If section number is not changed, emit a signal about last section.
	// Because if we emit it and section will be a part of sections signal, this can
	// duplicate menu items
	if ((removed > 0 || added > 0) && secdiff == 0)
	{
		add_signal_to_queue(menu,
		                    signal_queue,
		                    section_num - 1,
		                    change_pos,
		                    removed,
		                    added);
	}
	// If sections was changed, add change signal to queue
	if (secdiff != 0)
		add_signal_to_queue(menu,
		                    signal_queue,
		                    -1,
		                    old_sections < section_num ? old_sections : section_num,
		                    (secdiff) > 0 ? (uint)secdiff - 1 : 0,
		                    (secdiff) < 0 ? (uint)-secdiff : 0);
	g_variant_unref(items);
	// Emit all signals from queus by LIFO order
	queue_emit_all(signal_queue);
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
	g_autoptr(GVariant) props      = NULL;
	g_autoptr(GVariant) items      = NULL;
	g_autoptr(GVariant) layout     = NULL;
	g_autoptr(GError) error        = NULL;
	g_autoptr(GQueue) signal_queue = g_queue_new();
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
	queue_emit_all(signal_queue);
}

G_GNUC_INTERNAL void dbus_menu_model_update_layout_sync(DBusMenuModel *menu)
{
	g_return_if_fail(DBUS_MENU_IS_MODEL(menu));
	g_autoptr(GVariant) layout = NULL;
	g_autoptr(GError) error    = NULL;
	guint revision;
	if (menu->layout_update_in_progress)
		menu->layout_update_required = true;
	else
		dbus_menu_xml_call_get_layout_sync(menu->xml,
		                                   menu->parent_id,
		                                   1,
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
	layout_parse(menu, layout);
	menu->layout_update_in_progress = false;
	if (menu->layout_update_required)
		dbus_menu_model_update_layout_sync(menu);
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
	if (!DBUS_MENU_IS_XML(proxy))
		return;
	if (((uint)parent == menu->parent_id) && menu->current_revision < revision)
	{
		g_debug("Remote attempt to update %u with rev %u\n", parent, revision);
		dbus_menu_model_update_layout(menu);
		menu->current_revision = revision;
		return;
	}
	int sect_n = 0, position = 0;
	DBusMenuItem *item = dbus_menu_model_find(menu, (uint)parent, &sect_n, &position);
	if (item != NULL)
		dbus_menu_update_item_properties_from_layout_sync(menu, item, sect_n, position);
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
		int sect_n = 0, position = 0;
		DBusMenuItem *item =
		    (DBusMenuItem *)dbus_menu_model_find(menu, id, &sect_n, &position);
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
					                    sect_n,
					                    position,
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
	queue_emit_all(signal_queue);
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

G_GNUC_INTERNAL void dbus_menu_model_set_layout_update_required(DBusMenuModel *model, bool required)
{
	model->layout_update_required = required;
}

static DBusMenuItem *dbus_menu_model_find(DBusMenuModel *menu, uint item_id, int *section_num,
                                          int *position)
{
	for (GSequenceIter *iter = g_sequence_get_begin_iter(menu->sections);
	     !g_sequence_iter_is_end(iter);
	     iter = g_sequence_iter_next(iter))
	{
		DBusMenuItem *current_section = (DBusMenuItem *)g_sequence_get(iter);
		int current_section_num       = g_sequence_iter_get_position(iter);
		if (current_section->id == item_id && current_section_num > 0)
		{
			*section_num = -1;
			*position    = current_section_num;
			return current_section;
		}
		DBusMenuSectionModel *smodel = DBUS_MENU_SECTION_MODEL(
		    g_hash_table_lookup(current_section->links, G_MENU_LINK_SECTION));
		if (g_menu_model_get_n_items(G_MENU_MODEL(smodel)) == 0)
			return NULL;
		for (GSequenceIter *siter = g_sequence_get_begin_iter(smodel->items);
		     !g_sequence_iter_is_end(siter);
		     siter = g_sequence_iter_next(siter))
		{
			DBusMenuItem *current_item = (DBusMenuItem *)g_sequence_get(siter);
			int current_num            = g_sequence_iter_get_position(siter);
			if (current_item->id == item_id)
			{
				*section_num = current_section_num;
				*position    = current_num;
				return current_item;
			}
		}
	}
	return NULL;
}

static void dbus_menu_model_init(DBusMenuModel *menu)
{
	menu->cancellable               = g_cancellable_new();
	menu->parent_id                 = UINT_MAX;
	menu->sections                  = g_sequence_new(dbus_menu_item_free);
	menu->layout_update_required    = true;
	menu->layout_update_in_progress = false;
	menu->current_revision          = 0;
}

static void dbus_menu_model_constructed(GObject *object)
{
	G_OBJECT_CLASS(dbus_menu_model_parent_class)->constructed(object);
	DBusMenuModel *menu = DBUS_MENU_MODEL(object);

	DBusMenuItem *first_section =
	    dbus_menu_item_new_first_section(menu->parent_id, menu->received_action_group);
	g_sequence_append(menu->sections, first_section);
	g_hash_table_insert(first_section->links,
	                    g_strdup(G_MENU_LINK_SECTION),
	                    dbus_menu_section_model_new(menu, 0));
}

static void dbus_menu_model_finalize(GObject *object)
{
	DBusMenuModel *menu = (DBusMenuModel *)(object);
	if (menu->xml)
		g_signal_handlers_disconnect_by_data(menu->xml, menu);
	g_cancellable_cancel(menu->cancellable);
	g_clear_object(&menu->cancellable);
	g_clear_pointer(&menu->sections, g_sequence_free);

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
