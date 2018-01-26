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
	GSequence *all_items;
	GPtrArray *sections;
	bool layout_update_required;
};

static const gchar *property_names[] = { "accessible-desc",
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
static void emit_item_update_signal(DBusMenuModel *model, int sect_num, int pos);
static GPtrArray *dbus_menu_model_generate_section_array(GSequence *seq);

G_DEFINE_TYPE(DBusMenuModel, dbus_menu_model, G_TYPE_MENU_MODEL)

static gint dbus_menu_model_get_n_items(GMenuModel *model)
{
	DBusMenuModel *menu = (DBusMenuModel *)(model);

	return (int)menu->sections->len;
}

static void dbus_menu_model_get_item_attributes(GMenuModel *model, gint position,
                                                GHashTable **table)
{
	DBusMenuModel *menu = DBUS_MENU_MODEL(model);
	DBusMenuItem *item  = (DBusMenuItem *)g_sequence_get(
            (GSequenceIter *)g_ptr_array_index(menu->sections, position));

	*table = g_hash_table_ref(item->attributes);
}

static void dbus_menu_model_get_item_links(GMenuModel *model, gint position, GHashTable **table)
{
	DBusMenuModel *menu = DBUS_MENU_MODEL(model);
	DBusMenuItem *item  = (DBusMenuItem *)g_sequence_get(
            (GSequenceIter *)g_ptr_array_index(menu->sections, position));

	*table = g_hash_table_ref(item->links);
}

static GPtrArray *dbus_menu_model_generate_section_array(GSequence *seq)
{
	GPtrArray *ret = g_ptr_array_new();
	GSequenceIter *iter;
	for (iter = g_sequence_get_begin_iter(seq); !g_sequence_iter_is_end(iter);
	     iter = g_sequence_iter_next(iter))
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(iter);
		if (item->is_section)
			g_ptr_array_add(ret, iter);
	}
	return ret;
}

struct layout_data
{
	GMenuModel *model;
	uint pos;
	uint old_num;
	uint new_num;
};

static bool get_layout_idle(struct layout_data *data)
{
	if (G_IS_MENU_MODEL(data->model))
		g_menu_model_items_changed(G_MENU_MODEL(data->model),
		                           data->pos,
		                           data->old_num,
		                           data->new_num);
	return G_SOURCE_REMOVE;
}

static void emit_layout_update_signal(DBusMenuModel *model, int sect_num, uint pos, uint removed,
                                      uint added)
{
	struct layout_data *data = g_new0(struct layout_data, 1);
	if (sect_num >= 0)
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(
		    dbus_menu_model_get_section_iter(model, sect_num));
		data->model = G_MENU_MODEL(g_hash_table_lookup(item->links, G_MENU_LINK_SECTION));
	}
	else
		data->model = G_MENU_MODEL(model);
	data->pos     = pos;
	data->old_num = removed;
	data->new_num = added;
	g_idle_add_full(600, get_layout_idle, data, g_free);
}

static void update_section(DBusMenuModel *model, GSequenceIter *new_section_begin,
                           GSequenceIter *new_section_end, int section_num)
{
	GSequenceIter *old_section_begin = dbus_menu_model_get_section_iter(model, section_num);
	GSequenceIter *old_section_end   = dbus_menu_model_get_section_iter(model, section_num + 1);
	if (!g_sequence_iter_is_end(old_section_end))
		old_section_end = g_sequence_iter_prev(old_section_end);
	int old_n = g_sequence_iter_get_position(old_section_end) -
	            g_sequence_iter_get_position(old_section_begin) - 1;
	int new_n = g_sequence_iter_get_position(new_section_end) -
	            g_sequence_iter_get_position(new_section_begin);
	if (old_n != new_n)
	{
		g_sequence_remove_range(g_sequence_iter_next(old_section_begin), old_section_end);
		g_sequence_move_range(g_sequence_iter_next(old_section_begin),
		                      new_section_begin,
		                      new_section_end);
		emit_layout_update_signal(model, section_num, 0, old_n, new_n);
	}
	else if (old_n != 0)
	{
		GSequenceIter *old_section_iter = old_section_begin;
		old_section_iter                = g_sequence_iter_next(old_section_iter);
		GSequenceIter *new_section_iter = new_section_begin;
		for (int i = 0; g_sequence_iter_compare(old_section_iter, old_section_end) < 0; i++)
		{
			DBusMenuItem *old      = (DBusMenuItem *)g_sequence_get(old_section_iter);
			DBusMenuItem *new_item = (DBusMenuItem *)g_sequence_get(new_section_iter);
			bool updated           = !dbus_menu_item_compare_immutable(old, new_item);
			if (updated)
			{
				g_sequence_swap(old_section_iter, new_section_iter);
				GSequenceIter *tmp = new_section_iter;
				new_section_iter   = old_section_iter;
				old_section_iter   = tmp;
				emit_item_update_signal(model, section_num, i);
			}
			else
			{
				updated = dbus_menu_item_copy_attributes(new_item, old);
				if (updated)
					emit_item_update_signal(model, section_num, i);
			}
			old_section_iter = g_sequence_iter_next(old_section_iter);
			new_section_iter = g_sequence_iter_next(new_section_iter);
		}
	}
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

	g_variant_get(layout, "(i@a{sv}@av)", &id, &props, &items);
	g_variant_unref(props);
	GVariantIter iter;
	GVariant *child;
	g_autoptr(GSequence) new_sequence = g_sequence_new(dbus_menu_item_free);
	g_variant_iter_init(&iter, items);
	while ((child = g_variant_iter_next_value(&iter)))
	{
		GVariant *value = g_variant_get_variant(child);
		guint cid;
		GVariant *cprops;
		GVariant *citems;
		g_variant_get(value, "(i@a{sv}@av)", &cid, &cprops, &citems);
		g_variant_unref(citems);

		DBusMenuItem *new_item = dbus_menu_item_new(cid, menu, cprops);
		g_sequence_append(new_sequence, new_item);
		g_variant_unref(cprops);
		g_variant_unref(child);
	}
	g_variant_unref(items);
	GSequenceIter *old_iter = g_sequence_iter_next(g_sequence_get_begin_iter(menu->all_items));
	GSequenceIter *new_iter = g_sequence_get_begin_iter(new_sequence);
	g_autoptr(GPtrArray) new_sections = dbus_menu_model_generate_section_array(new_sequence);
	int diff                          = (int)new_sections->len - (int)menu->sections->len + 1;
	if (!diff)
		for (uint i = 0; i < MIN(new_sections->len, menu->sections->len); i++)
			if (!dbus_menu_item_compare_immutable((DBusMenuItem *)
			                                          new_sections->pdata[i],
			                                      (DBusMenuItem *)
			                                          menu->sections->pdata[i + 1]))
			{
				diff = 1;
				break;
			}
	if (diff)
	{
		g_sequence_remove_range(old_iter, g_sequence_get_end_iter(menu->all_items));
		g_sequence_move_range(g_sequence_get_end_iter(menu->all_items),
		                      new_iter,
		                      g_sequence_get_end_iter(new_sequence));
		uint old_len = menu->sections->len;
		g_ptr_array_unref(menu->sections);
		menu->sections = dbus_menu_model_generate_section_array(menu->all_items);
		for (uint i = 1; i < menu->sections->len; i++)
		{
			DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(
			    (GSequenceIter *)g_ptr_array_index(menu->sections, i));
			DBusMenuSectionModel *current_section =
			    dbus_menu_section_model_new(menu, i);
			g_hash_table_insert(item->links,
			                    g_strdup(G_MENU_LINK_SECTION),
			                    current_section);
		}
		emit_layout_update_signal(menu, -1, 0, old_len, menu->sections->len);
		return;
	}
	for (uint i = 0; i < menu->sections->len; i++)
	{
		GSequenceIter *begin;
		if (i == 0)
			begin = g_sequence_get_begin_iter(new_sequence);
		else
			begin = g_sequence_iter_next((GSequenceIter *)new_sections->pdata[i]);
		GSequenceIter *end;
		if (i + 1 > new_sections->len)
			end = g_sequence_get_end_iter(new_sequence);
		else
			end = g_sequence_iter_prev((GSequenceIter *)new_sections->pdata[i + 1]);
		update_section(menu, begin, end, i);
	}
}

static void get_layout_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	g_autoptr(GVariant) layout = NULL;
	guint revision;
	if (!DBUS_MENU_IS_MODEL(user_data))
		return;
	DBusMenuModel *menu = DBUS_MENU_MODEL(user_data);

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
	menu->layout_update_required = false;
	//	GString *str                 = g_string_new(NULL);
	//	g_menu_markup_print_string(str, menu, 4, 4);
	//	char *cstr = g_string_free(str, false);
	//	g_print("%s\n", cstr);
}

G_GNUC_INTERNAL void dbus_menu_model_update_layout(DBusMenuModel *menu)
{
	g_return_if_fail(DBUS_MENU_IS_MODEL(menu));
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
	if (((uint)parent == menu->parent_id) && revision > menu->current_revision && parent == 0)
	{
		g_warning("Remote attempt to update root\n");
		dbus_menu_model_update_layout(menu);
	}
	else if (((uint)parent == menu->parent_id) && revision > menu->current_revision)
	{
		menu->layout_update_required = true;
	}
}

static void item_activation_requested_cb(DBusMenuXml *proxy, gint id, guint timestamp,
                                         DBusMenuModel *menu)
{
	char *ordinary_name = g_strdup_printf(ACTION_PREFIX "%u", id);
	g_action_group_activate_action(menu->received_action_group, ordinary_name, NULL);
	g_debug("activation requested: id - %d, timestamp - %d", id, timestamp);
}

struct pos_data
{
	GMenuModel *model;
	int position;
};

static bool items_properties_updated_idle(struct pos_data *data)
{
	if (G_IS_MENU_MODEL(data->model))
		g_menu_model_items_changed(data->model, data->position, 1, 1);
	return G_SOURCE_REMOVE;
}

static void emit_item_update_signal(DBusMenuModel *model, int sect_num, int pos)
{
	struct pos_data *data = g_new0(struct pos_data, 1);
	if (sect_num >= 0)
	{
		DBusMenuItem *item = (DBusMenuItem *)g_sequence_get(
		    dbus_menu_model_get_section_iter(model, sect_num));
		GMenuModel *chm =
		    G_MENU_MODEL(g_hash_table_lookup(item->links, G_MENU_LINK_SECTION));
		data->model = chm;
	}
	else
		data->model = G_MENU_MODEL(model);
	data->position = pos;
	g_idle_add_full(500, items_properties_updated_idle, data, g_free);
}

static void items_properties_updated_cb(DBusMenuXml *proxy, GVariant *updated_props,
                                        GVariant *removed_props, DBusMenuModel *menu)
{
	GVariantIter iter;
	guint id;
	GVariant *props;
	DBusMenuItem *item;
	g_variant_iter_init(&iter, updated_props);
	while (g_variant_iter_loop(&iter, "(i@a{sv})", &id, &props))
	{
		int sect_n = 0, position = 0;
		item = (DBusMenuItem *)dbus_menu_model_find(menu, id, &sect_n, &position);
		bool is_item_updated = false;
		if (item != NULL)
		{
			is_item_updated = dbus_menu_item_update_props(item, props);
			if (is_item_updated)
				emit_item_update_signal(menu, sect_n, position);
		}
	}
	g_variant_iter_init(&iter, removed_props);
	while (g_variant_iter_loop(&iter, "(i@as)", &id, &props))
	{
		int sect_n = 0, position = 0;
		item = (DBusMenuItem *)dbus_menu_model_find(menu, id, &sect_n, &position);
		bool is_item_updated = false;
		if (item != NULL)
		{
			is_item_updated = dbus_menu_item_remove_props(item, props);
			if (is_item_updated)
				emit_item_update_signal(menu, sect_n, position);
		}
	}
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

DBusMenuModel *dbus_menu_model_new(uint parent_id, DBusMenuModel *parent, DBusMenuXml *xml,
                                   GActionGroup *action_group)
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

static bool dbus_menu_model_is_mutable(GMenuModel *model)
{
	return true;
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
			on_xml_property_changed(menu);
		break;
	case PROP_ACTION_GROUP:
		menu->received_action_group = G_ACTION_GROUP(g_value_get_object(value));
		break;
	case PROP_PARENT_ID:
		menu->parent_id = g_value_get_uint(value);
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

G_GNUC_INTERNAL GSequenceIter *dbus_menu_model_get_section_iter(DBusMenuModel *model,
                                                                uint section_index)
{
	if (section_index >= model->sections->len)
		return g_sequence_get_end_iter(model->all_items);
	return (GSequenceIter *)model->sections->pdata[section_index];
}

G_GNUC_INTERNAL bool dbus_menu_model_is_layout_update_required(DBusMenuModel *model)
{
	return model->layout_update_required;
}

static DBusMenuItem *dbus_menu_model_find(DBusMenuModel *menu, uint item_id, int *section_num,
                                          int *position)
{
	GSequenceIter *iter = g_sequence_lookup(menu->all_items,
	                                        GUINT_TO_POINTER(item_id),
	                                        (GCompareDataFunc)dbus_menu_item_id_compare_func,
	                                        NULL);
	if (!iter)
		return NULL;
	for (uint i = 0; i < menu->sections->len - 1; i++)
	{
		int cmp_begin =
		    g_sequence_iter_compare(iter, dbus_menu_model_get_section_iter(menu, i));
		int cmp_end =
		    g_sequence_iter_compare(iter, dbus_menu_model_get_section_iter(menu, i + 1));
		if (cmp_begin > 0 &&
		    (cmp_end < 0 || (cmp_end == 0 && i + 1 == menu->sections->len)))
		{
			*section_num = i;
			*position =
			    g_sequence_iter_get_position(iter) -
			    g_sequence_iter_get_position((GSequenceIter *)menu->sections->pdata[i]);
		}
		else if (cmp_begin == 0)
		{
			*section_num = -1;
			*position    = i;
		}
		return (DBusMenuItem *)g_sequence_get(iter);
	}
	return NULL;
}

static void dbus_menu_model_init(DBusMenuModel *menu)
{
	menu->cancellable            = g_cancellable_new();
	menu->parent_id              = UINT_MAX;
	menu->sections               = g_ptr_array_new();
	menu->all_items              = g_sequence_new(dbus_menu_item_free);
	menu->layout_update_required = false;
	menu->current_revision       = 0;
}

static void dbus_menu_model_constructed(GObject *object)
{
	G_OBJECT_CLASS(dbus_menu_model_parent_class)->constructed(object);
	DBusMenuModel *menu = DBUS_MENU_MODEL(object);

	DBusMenuItem *first_section =
	    dbus_menu_item_new_first_section(menu->parent_id, menu->received_action_group);
	g_sequence_append(menu->all_items, first_section);
	GSequenceIter *iter = g_sequence_get_iter_at_pos(menu->all_items, 0);
	g_ptr_array_add(menu->sections, iter);
	g_hash_table_insert(first_section->links,
	                    g_strdup(G_MENU_LINK_SECTION),
	                    dbus_menu_section_model_new(menu, 0));
}

static void dbus_menu_model_finalize(GObject *object)
{
	DBusMenuModel *menu = (DBusMenuModel *)(object);
	if (DBUS_MENU_IS_XML(menu->xml))
		g_signal_handlers_disconnect_by_data(menu->xml, menu);
	g_cancellable_cancel(menu->cancellable);
	g_clear_object(&menu->cancellable);
	g_clear_pointer(&menu->sections, g_ptr_array_unref);
	g_clear_pointer(&menu->all_items, g_sequence_free);

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
	                      (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
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
