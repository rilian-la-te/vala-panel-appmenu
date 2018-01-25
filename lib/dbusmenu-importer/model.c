#include <inttypes.h>

#include "debug.h"
#include "definitions.h"
#include "item.h"
#include "model.h"

struct _DBusMenuModel
{
	GMenuModel parent_instance;

	uint parent_id;
	uint current_revision;
	bool is_section_model;
	bool layout_is_updating;
	GCancellable *cancellable;
	DBusMenuXml *xml;
	GActionGroup *received_action_group;
	GPtrArray *items;
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
	PROP_NULL             = 0,
	PROP_XML              = 1,
	PROP_ACTION_GROUP     = 2,
	PROP_PARENT_ID        = 3,
	PROP_IS_SECTION_MODEL = 4,
	NUM_PROPS
};

static DBusMenuModel *dbus_menu_model_new_section(uint parent_id, DBusMenuModel *parent,
                                                  DBusMenuXml *xml, GActionGroup *action_group);
static DBusMenuItem *dbus_menu_model_find(DBusMenuModel *menu, uint item_id, int *section_num,
                                          int *position);
static GParamSpec *properties[NUM_PROPS] = { NULL };

G_DEFINE_TYPE(DBusMenuModel, dbus_menu_model, G_TYPE_MENU_MODEL)

static gint dbus_menu_model_get_n_items(GMenuModel *model)
{
	DBusMenuModel *menu = (DBusMenuModel *)(model);

	return menu->items->len;
}

static void dbus_menu_model_get_item_attributes(GMenuModel *model, gint position,
                                                GHashTable **table)
{
	DBusMenuModel *menu = (DBusMenuModel *)(model);

	*table = g_hash_table_ref(
	    ((DBusMenuItem *)g_ptr_array_index(menu->items, position))->attributes);
}

static void dbus_menu_model_get_item_links(GMenuModel *model, gint position, GHashTable **table)
{
	DBusMenuModel *menu = (DBusMenuModel *)(model);

	*table =
	    g_hash_table_ref(((DBusMenuItem *)g_ptr_array_index(menu->items, position))->links);
}

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
	GMenuModel *current_section =
	    G_MENU_MODEL(dbus_menu_model_new_section(menu->parent_id,
	                                             menu,
	                                             menu->xml,
	                                             menu->received_action_group));
	DBusMenuItem *current_section_item =
	    dbus_menu_item_new_first_section(menu->parent_id,
	                                     menu->xml,
	                                     menu->received_action_group);
	g_ptr_array_add(menu->items, current_section_item);
	g_hash_table_insert(current_section_item->links,
	                    g_strdup(G_MENU_LINK_SECTION),
	                    current_section);
	g_variant_iter_init(&iter, items);
	while ((child = g_variant_iter_next_value(&iter)))
	{
		GVariant *value = g_variant_get_variant(child);
		guint cid;
		GVariant *cprops;
		GVariant *citems;
		g_variant_get(value, "(i@a{sv}@av)", &cid, &cprops, &citems);
		g_variant_unref(citems);

		DBusMenuItem *new_item =
		    dbus_menu_item_new(cid, current_section_item, menu, cprops);
		if (new_item->is_section)
		{
			current_section =
			    G_MENU_MODEL(dbus_menu_model_new_section(menu->parent_id,
			                                             menu,
			                                             menu->xml,
			                                             menu->received_action_group));
			current_section_item = new_item;
			g_ptr_array_add(menu->items, current_section_item);
			g_hash_table_insert(current_section_item->links,
			                    g_strdup(G_MENU_LINK_SECTION),
			                    current_section);
		}
		else
		{
			g_ptr_array_add(((DBusMenuModel *)current_section)->items, new_item);
		}
		g_variant_unref(cprops);
		g_variant_unref(citems);
		g_variant_unref(child);
	}
	g_variant_unref(items);
}

static void get_layout_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	g_autoptr(GVariant) layout = NULL;
	guint revision;
	DBusMenuModel *menu = (DBusMenuModel *)(user_data);
	;

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
	uint old_num = menu->items->len;
	if (old_num > 0)
		g_ptr_array_remove_range(menu->items, 0, old_num);
	layout_parse(menu, layout);
	uint new_num           = menu->items->len;
	menu->current_revision = revision;
	g_menu_model_items_changed(G_MENU_MODEL(menu), 0, old_num, new_num);
	menu->layout_is_updating = false;
	//	GString *str = g_string_new(NULL);
	//	g_menu_markup_print_string(str, menu, 4, 4);
	//	char *cstr = g_string_free(str, false);
	//	g_print("%s\n", cstr);
}

G_GNUC_INTERNAL void dbus_menu_model_update_layout(DBusMenuModel *menu)
{
	g_return_if_fail(DBUS_MENU_IS_MODEL(menu));
	menu->layout_is_updating = true;
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
	if (((uint)parent == menu->parent_id) && revision > menu->current_revision)
	{
		dbus_menu_model_update_layout(menu);
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
	DBusMenuModel *model;
	int position;
};

static bool items_properties_updated_idle(struct pos_data *data)
{
	if (DBUS_MENU_IS_MODEL(data->model) && !data->model->layout_is_updating)
		g_menu_model_items_changed(G_MENU_MODEL(data->model), data->position, 1, 1);
	return G_SOURCE_REMOVE;
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
			is_item_updated       = dbus_menu_item_update_props(item, props);
			struct pos_data *data = g_new0(struct pos_data, 1);
			if (sect_n >= 0)
			{
				DBusMenuItem *item = menu->items->pdata[sect_n];
				DBusMenuModel *chm =
				    g_hash_table_lookup(item->links, G_MENU_LINK_SECTION);
				data->model = chm;
			}
			else
				data->model = menu;
			data->position = position;
			if (!menu->layout_is_updating && is_item_updated)
				g_idle_add_full(500, items_properties_updated_idle, data, g_free);
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
			is_item_updated       = dbus_menu_item_remove_props(item, props);
			struct pos_data *data = g_new0(struct pos_data, 1);
			if (sect_n >= 0)
			{
				DBusMenuItem *item = menu->items->pdata[sect_n];
				DBusMenuModel *chm =
				    g_hash_table_lookup(item->links, G_MENU_LINK_SECTION);
				data->model = chm;
			}
			else
				data->model = menu;
			data->position = position;
			if (!menu->layout_is_updating && is_item_updated)
				g_idle_add_full(500, items_properties_updated_idle, data, g_free);
		}
	}
}

static void on_xml_property_changed(DBusMenuModel *model)
{
	if (!DBUS_MENU_IS_XML(model->xml))
		return;
	if (!model->is_section_model)
	{
		g_signal_connect(model->xml,
		                 "items-properties-updated",
		                 G_CALLBACK(items_properties_updated_cb),
		                 model);
		g_signal_connect(model->xml,
		                 "layout-updated",
		                 G_CALLBACK(layout_updated_cb),
		                 model);
		g_signal_connect(model->xml,
		                 "item-activation-requested",
		                 G_CALLBACK(item_activation_requested_cb),
		                 model);
		if (model->parent_id == 0)
			dbus_menu_model_update_layout(model);
	}
}

static DBusMenuModel *dbus_menu_model_new_section(uint parent_id, DBusMenuModel *parent,
                                                  DBusMenuXml *xml, GActionGroup *action_group)
{
	DBusMenuModel *ret = (DBusMenuModel *)g_object_new(dbus_menu_model_get_type(),
	                                                   "xml",
	                                                   xml,
	                                                   "action-group",
	                                                   action_group,
	                                                   "parent-id",
	                                                   parent_id,
	                                                   NULL);
	if (parent != NULL)
		g_object_bind_property(parent, "xml", ret, "xml", G_BINDING_SYNC_CREATE);
	return ret;
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
	                                                   "is-section-model",
	                                                   false,
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
	case PROP_IS_SECTION_MODEL:
		menu->is_section_model = g_value_get_boolean(value);
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
	case PROP_IS_SECTION_MODEL:
		g_value_set_boolean(value, menu->is_section_model);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static DBusMenuItem *dbus_menu_model_find(DBusMenuModel *menu, uint item_id, int *section_num,
                                          int *position)
{
	for (uint i = 0; i < menu->items->len; i++)
	{
		DBusMenuItem *item = menu->items->pdata[i];
		if (item->id == item_id)
		{
			*section_num = -1;
			*position    = i;
			return item;
		}
		DBusMenuModel *chm = g_hash_table_lookup(item->links, G_MENU_LINK_SECTION);
		if (chm != NULL)
		{
			int section_nump = -1, positionp = -1;
			DBusMenuItem *ch =
			    dbus_menu_model_find(chm, item_id, &section_nump, &positionp);
			if (ch != NULL)
			{
				*section_num = i;
				*position    = positionp;
				return ch;
			}
		}
	}
	return NULL;
}

static void dbus_menu_model_init(DBusMenuModel *menu)
{
	menu->cancellable        = g_cancellable_new();
	menu->parent_id          = UINT_MAX;
	menu->items              = g_ptr_array_new_with_free_func(dbus_menu_item_free);
	menu->current_revision   = 0;
	menu->layout_is_updating = false;
}

static void dbus_menu_model_finalize(GObject *object)
{
	DBusMenuModel *menu = (DBusMenuModel *)(object);
	if (DBUS_MENU_IS_XML(menu->xml))
		g_signal_handlers_disconnect_by_data(menu->xml, menu);
	g_cancellable_cancel(menu->cancellable);
	g_clear_object(&menu->cancellable);
	g_ptr_array_free(menu->items, true);

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
	properties[PROP_IS_SECTION_MODEL] =
	    g_param_spec_boolean("is-section-model",
	                         "is-section-model",
	                         "is-section-model",
	                         true,
	                         (GParamFlags)(G_PARAM_CONSTRUCT_ONLY | G_PARAM_READABLE |
	                                       G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_properties(object_class, NUM_PROPS, properties);
}

static void dbus_menu_model_class_init(DBusMenuModelClass *class)
{
	GMenuModelClass *model_class = G_MENU_MODEL_CLASS(class);
	GObjectClass *object_class   = G_OBJECT_CLASS(class);

	object_class->finalize     = dbus_menu_model_finalize;
	object_class->set_property = dbus_menu_model_set_property;
	object_class->get_property = dbus_menu_model_get_property;

	model_class->is_mutable          = dbus_menu_model_is_mutable;
	model_class->get_n_items         = dbus_menu_model_get_n_items;
	model_class->get_item_attributes = dbus_menu_model_get_item_attributes;
	model_class->get_item_links      = dbus_menu_model_get_item_links;
	install_properties(object_class);
}
