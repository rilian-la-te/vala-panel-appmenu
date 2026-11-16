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
	GVariant *current_layout;
	bool layout_update_required;
	uint parse_pending;
};

static const char *property_names[] = { DBUS_MENU_PROP_ACCESSIBLE_DESC,
	                                    DBUS_MENU_PROP_CHILDREN_DISPLAY,
	                                    DBUS_MENU_PROP_DISPOSITION,
    	                                DBUS_MENU_PROP_ENABLED,
    	                                DBUS_MENU_PROP_ICON_DATA,
    	                                DBUS_MENU_PROP_ICON_NAME,
    	                                DBUS_MENU_PROP_LABEL,
    	                                DBUS_MENU_PROP_SHORTCUT,
    	                                DBUS_MENU_PROP_TOGGLE_TYPE,
    	                                DBUS_MENU_PROP_TOGGLE_STATE,
    	                                DBUS_MENU_PROP_TYPE,
    	                                DBUS_MENU_PROP_VISIBLE,
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

static DBusMenuItem *dbus_menu_model_find(DBusMenuModel *menu, uint item_id);
static DBusMenuItem *dbus_menu_model_find_section(DBusMenuModel *menu, uint section_num);
static GSequenceIter *dbus_menu_model_find_place(DBusMenuModel *menu, uint section_num, int place);
static void dbus_menu_model_update_layout(DBusMenuModel *menu, u_int32_t child_id);

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

static int queue_compare_func(const struct layout_data *a, const struct layout_data *b)
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
	else
    	g_free(data);
}

static void queue_emit_all(GQueue *queue)
{
	struct layout_data *raw_index = NULL;
	while ((raw_index = (struct layout_data *)g_queue_pop_head(queue))) {
		g_autofree struct layout_data* index = raw_index;
		g_menu_model_items_changed(index->model, index->pos, index->old_num, index->new_num);
	}
}

static bool preload_idle(DBusMenuItem *item)
{
	dbus_menu_item_preload(item);
	return G_SOURCE_REMOVE;
}

static void menu_item_schedule_preload(DBusMenuItem* item) {
	g_timeout_add_full(100, 100, (GSourceFunc)preload_idle, item, NULL);
}

static void menu_item_copy_and_load(DBusMenuModel *menu, DBusMenuItem *old, DBusMenuItem *new_item)
{
	bool new_submenu = dbus_menu_item_copy_submenu(old, new_item, menu);
	dbus_menu_item_generate_action(new_item, menu);
	/* It is a preload hack. If this is a toplevel menu, we need to fetch menu under toplevel to
	 * avoid menu jumping bug. Now we need to use it for all menus - no menus are preloaded,
	 * AFAIK. But always enabled make sense only for submenus.
	 */
	dbus_menu_item_update_enabled(new_item, new_submenu || new_item->enabled);
	new_item->toggled = true;
	menu_item_schedule_preload(new_item);
}

// We deal only with layouts with depth 1 (not all)
typedef struct
{
	DBusMenuModel *menu;
	GQueue *queue;
	uint section_num;
	uint place;
	uint old_sections;
	int first_changed;
	int last_changed;
	int old_sec_items;
} LayoutParseContext;

static int get_section_item_count(DBusMenuModel *menu, uint section_num)
{
	DBusMenuItem *sec = dbus_menu_model_find_section(menu, section_num);
	GMenuModel *m     = G_MENU_MODEL(g_hash_table_lookup(sec->links, G_MENU_LINK_SECTION));
	return g_menu_model_get_n_items(m);
}

static void mark_item_changed(LayoutParseContext *ctx)
{
	if (ctx->section_num >= ctx->old_sections)
		return;

	if (ctx->first_changed == -1)
		ctx->first_changed = (int)ctx->place;
	ctx->last_changed = (int)ctx->place;
}

static GSequenceIter *insert_menu_item(DBusMenuModel *menu, DBusMenuItem *item)
{
	return g_sequence_insert_sorted(menu->items, item, dbus_menu_model_sort_func, NULL);
}

static GSequenceIter *lookup_menu_item(DBusMenuModel *menu, DBusMenuItem *item)
{
	return g_sequence_lookup(menu->items, item, dbus_menu_model_sort_func, NULL);
}

static void finalize_section(LayoutParseContext *ctx, GSequenceIter *end_iter)
{
	DBusMenuItem *old = (DBusMenuItem *)g_sequence_get(g_sequence_iter_prev(end_iter));
	int delta         = old->place - (int)(ctx->place - 1);

	// Cleanup all items in prev section (if there is more items than
	// current)
	if (delta > 0)
	{
		GSequenceIter *place_iter = g_sequence_iter_move(end_iter, -delta);
		g_sequence_remove_range(place_iter, end_iter);
	}

	// Update current_section and reset current_iter and added to new
	// section
	// If it is changed section - schedule signal for it
	if (ctx->section_num >= ctx->old_sections)
		return;

	if (delta > 0)
	{
		if (ctx->first_changed == -1)
			ctx->first_changed = (int)ctx->place;
		ctx->last_changed = ctx->old_sec_items - 1;
	}
	if (ctx->first_changed != -1)
	{
		int new_sec_items  = (int)ctx->place;
		int untouched_tail = (delta > 0) ? 0 : (ctx->old_sec_items - 1 - ctx->last_changed);
		int removed        = ctx->old_sec_items - ctx->first_changed - untouched_tail;
		int added_cnt      = new_sec_items - ctx->first_changed - untouched_tail;
		add_signal_to_queue(ctx->menu,
		                    ctx->queue,
		                    ctx->section_num,
		                    ctx->first_changed,
		                    removed,
		                    added_cnt);
	}
}

static void handle_section_item(LayoutParseContext *ctx, DBusMenuItem *new_item)
{
	bool is_valid_section = !new_item->toggled && ctx->place > 0;

	// If section was invalid, just free received item.
	if (!is_valid_section)
	{
		dbus_menu_item_free(new_item);
		return;
	}

	// Section is valid, so, parse it
	// Do some common tasks: increment section_num
	uint next_section     = ctx->section_num + 1;
	new_item->section_num = next_section;
	new_item->place       = -1;

	GSequenceIter *old_iter = lookup_menu_item(ctx->menu, new_item);
	if (!old_iter)
	{
		g_hash_table_insert(new_item->links,
		                    G_MENU_LINK_SECTION,
		                    dbus_menu_section_model_new(ctx->menu, next_section));
		old_iter = insert_menu_item(ctx->menu, new_item);
	}
	else
		dbus_menu_item_free(new_item);

	finalize_section(ctx, old_iter);

	// Ensure next section
	ctx->section_num   = next_section;
	ctx->place         = 0;
	ctx->first_changed = -1;
	ctx->last_changed  = -1;
	ctx->old_sec_items = (ctx->section_num < ctx->old_sections)
	                         ? get_section_item_count(ctx->menu, ctx->section_num)
	                         : 0;
}

static void handle_regular_item(LayoutParseContext *ctx, DBusMenuItem *new_item, GVariant *cprops)
{
	new_item->section_num   = ctx->section_num;
	new_item->place         = ctx->place;
	GSequenceIter *old_iter = lookup_menu_item(ctx->menu, new_item);
	// There is no old item on this place
	if (!old_iter)
	{
		mark_item_changed(ctx);
		menu_item_copy_and_load(ctx->menu, NULL, new_item);
		insert_menu_item(ctx->menu, new_item);
		ctx->place++;
		return;
	}

	DBusMenuItem *old = (DBusMenuItem *)g_sequence_get(old_iter);
	// We should compare properties of old and new item
	if (dbus_menu_item_update_props(old, cprops))
	{
		mark_item_changed(ctx);
		menu_item_copy_and_load(ctx->menu, old, new_item);
		g_sequence_remove(old_iter);
		insert_menu_item(ctx->menu, new_item);
	}
	else
	{
		// Schedule reloading on child
		DBusMenuModel *submenu = dbus_menu_item_get_submenu(old);
		if (submenu)
		{
			submenu->layout_update_required = true;
			menu_item_schedule_preload(old);
		}
		// Free unneeded item
		dbus_menu_item_free(new_item);
	}
	ctx->place++;
}

static void parse_single_child(LayoutParseContext *ctx, GVariant *child)
{
	g_autoptr(GVariant) value  = g_variant_get_variant(child);
	g_autoptr(GVariant) cprops = NULL;
	g_autoptr(GVariant) citems = NULL;
	guint cid;
	g_variant_get(value, "(i@a{sv}@av)", &cid, &cprops, &citems);

	DBusMenuItem *new_item = dbus_menu_item_new(cid, ctx->menu, cprops);

	// We receive a section (separator or x-kde-title)
	if (new_item->action_type == DBUS_MENU_ACTION_SECTION)
		handle_section_item(ctx, new_item);
	else if (!dbus_menu_item_is_firefox_stub(new_item))
		handle_regular_item(ctx, new_item, cprops);
	else
		// Just free unnedeed item
		dbus_menu_item_free(new_item);
}

static void layout_parse_items(DBusMenuModel *menu, GVariant *items)
{
	GVariantIter iter;
	GVariant *child;
	g_autoptr(GQueue) queue = g_queue_new();

	// Start parsing. We need to track section number, and also GSequenceIter to
	// current section. Also we track change position, number of added and removed
	// items for current section
	// Current section changes, take first section data for references
	LayoutParseContext ctx = {
		.menu          = menu,
		.queue         = queue,
		.section_num   = 0,
		.place         = 0,
		.old_sections  = g_menu_model_get_n_items(G_MENU_MODEL(menu)),
		.first_changed = -1,
		.last_changed  = -1,
		.old_sec_items = get_section_item_count(menu, 0),
	};

	g_variant_iter_init(&iter, items);
	while ((child = g_variant_iter_next_value(&iter)))
	{
		parse_single_child(&ctx, child);
		g_variant_unref(child);
	}
	uint new_sections = ctx.section_num + 1;
	int secdiff       = (int)ctx.old_sections - (int)new_sections;

	// Remove last sections from the end
	if (secdiff > 0)
	{
		GSequenceIter *section_iter = dbus_menu_model_find_place(menu, new_sections, -1);
		g_sequence_remove_range(section_iter, g_sequence_get_end_iter(menu->items));
	}
	// We need to manage last section's changes. And check its validity
	if (secdiff >= 0)
	{
		// Do last section housework
		finalize_section(&ctx, g_sequence_get_end_iter(menu->items));
	}

	// Update all layout
	if (new_sections > ctx.old_sections)
	{
		add_signal_to_queue(menu,
		                    queue,
		                    -1,
		                    ctx.old_sections,
		                    0,
		                    new_sections - ctx.old_sections);
	}
	else if (new_sections < ctx.old_sections)
	{
		add_signal_to_queue(menu,
		                    queue,
		                    -1,
		                    new_sections,
		                    ctx.old_sections - new_sections,
		                    0);
	}

	queue_emit_all(queue);
}

static void layout_parse(DBusMenuModel *menu, GVariant* layout) {
	guint id;
	g_autoptr(GVariant) props = NULL;
	g_autoptr(GVariant) items = NULL;
	if (!g_variant_is_of_type(layout, G_VARIANT_TYPE("(ia{sv}av)")))
	{
		g_warning(
		    "Type of return value for 'layout' property in "
		    "'GetLayout' call should be '(ia{sv}av)' but got '%s'",
		    g_variant_get_type_string(layout));

		return;
	}
	//We really should not run if we are not a menu
	if(!DBUS_MENU_IS_MODEL(menu))
		return;
	g_variant_get(layout, "(i@a{sv}@av)", &id, &props, &items);
	if (id == menu->parent_id) {
		layout_parse_items(menu, items);
		return;
	}

	DBusMenuItem *item = dbus_menu_model_find(menu, (uint)id);
	if(!item)
		return;
	g_autoptr(UpdaterQueue) signal_queue = g_queue_new();
	int sect_n = item->section_num, pos = item->place;
	DBusMenuUpdaterFlags is_item_updated = dbus_menu_item_update_props(item, props);
	if (is_item_updated & DBUS_MENU_UPDATER_IMMUTABLE)
		dbus_menu_model_update_layout(menu, 0);
	else if (is_item_updated)
		add_signal_to_queue(menu, signal_queue, sect_n, pos, 1, 1);
	queue_emit_all(signal_queue);
}

static bool get_layout_idle(DBusMenuModel *self)
{
	g_return_val_if_fail(DBUS_MENU_IS_MODEL(self), G_SOURCE_REMOVE);
	layout_parse(self, self->current_layout);
	self->parse_pending = 0;
	return G_SOURCE_REMOVE;
}

static void get_layout_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	guint revision;
	if (!DBUS_MENU_IS_MODEL(user_data))
		return;
	DBusMenuModel *menu            = DBUS_MENU_MODEL(user_data);
	g_autoptr(GError) error        = NULL;
	g_autoptr(GVariant) old_layout = menu->current_layout;
	GVariant* new_layout = NULL;
	menu->current_layout = NULL;
	dbus_menu_xml_call_get_layout_finish((DBusMenuXml *)(source_object),
	                                     &revision,
	                                     &new_layout,
	                                     res,
	                                     &error);
	if (error != NULL)
	{
		if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_warning("%s", error->message);
		g_object_unref(menu);
		return;
	}
	menu->current_layout = new_layout;
	menu->layout_update_required = false;
	if (!menu->parse_pending)
		menu->parse_pending = g_timeout_add_full(G_PRIORITY_HIGH,
		                                         100,
		                                         (GSourceFunc)get_layout_idle,
		                                         g_object_ref(menu),
		                                         g_object_unref);
	g_object_unref(menu);
}

struct AboutToShowCtx {
    DBusMenuModel* model;
    GSimpleAction* submenu_action;
};

static void dbus_menu_model_about_to_show_cb(GObject *source, GAsyncResult *res, gpointer user_data)
{
	struct AboutToShowCtx *ctx = user_data;
	g_autoptr(GError) error = NULL;
	int need_update = 0;
	dbus_menu_xml_call_about_to_show_finish(DBUS_MENU_XML(source),
	                                        (gboolean *)&need_update,
	                                        res, &error);
	if (error != NULL)
	{
		if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
			g_debug("%s", error->message);
		goto fail;
	}
	if (g_menu_model_get_n_items(G_MENU_MODEL(ctx->model)) == 0)
		need_update = true;
	need_update = need_update ||
	              dbus_menu_model_is_layout_update_required(ctx->model);
	if (need_update)
		dbus_menu_model_update_layout(ctx->model, 0);
	if (ctx->submenu_action && G_IS_SIMPLE_ACTION(ctx->submenu_action))
		g_simple_action_set_state(ctx->submenu_action, g_variant_new_boolean(true));

fail:
	if (ctx->submenu_action && G_IS_SIMPLE_ACTION(ctx->submenu_action))
		g_object_unref(ctx->submenu_action);
	if (ctx->model && G_IS_OBJECT(ctx->model))
		g_object_unref(ctx->model);
	g_free(ctx);
}

G_GNUC_INTERNAL void dbus_menu_model_load(DBusMenuModel* self, GSimpleAction* action) {
	/* Async preload now. Just fire-and-forget */
	dbus_menu_xml_call_event(self->xml, self->parent_id,
							DBUS_MENU_EVENT_OPENED,
							g_variant_new("v", g_variant_new_int32(0)),
							CURRENT_TIME,
							NULL, NULL, NULL);
	struct AboutToShowCtx* ctx = g_malloc0(sizeof(*ctx));
	ctx->model = g_object_ref(self);
	ctx->submenu_action = action ? g_object_ref(action) : NULL;
	/* Unfortunately, this can hold a ref to a aged model */
	dbus_menu_xml_call_about_to_show(self->xml, self->parent_id, self->cancellable,
									dbus_menu_model_about_to_show_cb,
									ctx);
}

static void dbus_menu_model_update_layout(DBusMenuModel *menu, u_int32_t child_id)
{
	g_return_if_fail(DBUS_MENU_IS_MODEL(menu));
	dbus_menu_xml_call_get_layout(menu->xml,
	                              child_id ? child_id : menu->parent_id,
	                              child_id ? 0 : 1,
	                              property_names,
	                              menu->cancellable,
	                              get_layout_cb,
	                              g_object_ref(menu));
}

static void layout_updated_cb(DBusMenuXml *proxy, guint revision, gint parent, DBusMenuModel *menu)
{
	if (!DBUS_MENU_IS_XML(proxy))
		return;
	if (((uint)parent == menu->parent_id) && menu->current_revision < revision)
	{
		g_debug("Remote attempt to update %u with rev %u", parent, revision);
		dbus_menu_model_update_layout(menu, 0);
		menu->current_revision = revision;
		return;
	}
	DBusMenuItem *item = dbus_menu_model_find(menu, (uint)parent);
	if (item != NULL && menu->current_revision < revision) {
		dbus_menu_model_update_layout(menu, menu->parse_pending ? 0 : item->id);
		menu->current_revision = revision;
	}
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

static bool items_properties_loop(DBusMenuModel *menu, GVariant *up_props, GQueue *signal_queue,
                                  bool is_removal)
{
	GVariantIter iter;
	guint id;
	GVariant *props;
	g_variant_iter_init(&iter, up_props);
	while (g_variant_iter_next(&iter, !is_removal ? "(i@a{sv})" : "(i@as)", &id, &props))
	{
		g_autoptr(GVariant) owned_props = props;
		DBusMenuItem *item   = (DBusMenuItem *)dbus_menu_model_find(menu, id);
		DBusMenuUpdaterFlags is_item_updated = DBUS_MENU_UPDATER_NONE;
		if (item != NULL)
		{
			// It is the best what we can do to update a section
			if (item->action_type == DBUS_MENU_ACTION_SECTION)
				return true;
			else
			{
				is_item_updated = !is_removal
				                      ? dbus_menu_item_update_props(item, owned_props)
				                      : dbus_menu_item_remove_props(item, owned_props);
				if (is_item_updated & DBUS_MENU_UPDATER_IMMUTABLE)
					return true;
				else if (is_item_updated)
					add_signal_to_queue(menu,
					                    signal_queue,
					                    item->section_num,
					                    item->place,
					                    1,
					                    1);
			}
		}
	}
	return false;
}

static void items_properties_updated_cb(DBusMenuXml *proxy, GVariant *updated_props,
                                        GVariant *removed_props, DBusMenuModel *menu)
{
	if (!DBUS_MENU_IS_XML(proxy))
		return;
	if (menu->parse_pending)
		return;
	g_autoptr(UpdaterQueue) signal_queue = g_queue_new();
	bool immutable = false;
	immutable |= items_properties_loop(menu, updated_props, signal_queue, false);
	immutable |= items_properties_loop(menu, removed_props, signal_queue, true);
	if (immutable)
	{
		dbus_menu_model_update_layout(menu, 0);
		return;
	}
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
		dbus_menu_model_update_layout(model, 0);
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
	void *new_xml       = NULL;

	switch (property_id)
	{
	case PROP_XML:
		new_xml = DBUS_MENU_XML(g_value_get_object(value));
		if (menu->xml != NULL)
		{
			g_signal_handlers_disconnect_by_data(menu->xml, menu);
			g_clear_object(&menu->xml);
		}
		if(new_xml != NULL)
		{
			menu->xml= g_object_ref(new_xml);
			on_xml_property_changed(menu);
		}
		break;
	case PROP_ACTION_GROUP:
		g_clear_object(&menu->received_action_group);
		menu->received_action_group = g_object_ref(G_ACTION_GROUP(g_value_get_object(value)));
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
    GSequenceIter *iter = dbus_menu_model_find_place(menu, section_num, -1);
    if (iter == NULL)
        return NULL;
    return (DBusMenuItem *)g_sequence_get(iter);
}

static void dbus_menu_model_init(DBusMenuModel *menu)
{
	menu->cancellable            = g_cancellable_new();
	menu->parent_id              = UINT_MAX;
	menu->items                  = g_sequence_new(dbus_menu_item_free);
	menu->layout_update_required = true;
	menu->parse_pending          = 0;
	menu->current_revision       = 0;
	menu->current_layout		 = NULL;
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
	DBusMenuModel *menu = DBUS_MENU_MODEL(object);
	if (G_IS_OBJECT(menu->xml)) {
		g_signal_handlers_disconnect_by_data(menu->xml, menu);
		g_clear_object(&menu->xml);
	}
	g_source_remove_by_user_data(menu);
	g_cancellable_cancel(menu->cancellable);
	g_clear_object(&menu->cancellable);
	g_clear_object(&menu->received_action_group);
	g_clear_pointer(&menu->items, g_sequence_free);
	g_clear_pointer(&menu->current_layout, g_variant_unref);

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
