#include <inttypes.h>
#include <stdbool.h>

#include "definitions.h"
#include "item.h"
#include "model.h"
#include "debug.h"

struct _DBusMenuModel
{
	GMenuModel parent_instance;

	uint parent_id;
	bool is_section_model;
	GCancellable *cancellable;
	GHashTable *dbusmenu_section_items;
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
	g_hash_table_insert(menu->dbusmenu_section_items,
	                    GUINT_TO_POINTER(menu->parent_id),
	                    current_section_item);
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
			g_hash_table_insert(menu->dbusmenu_section_items,
			                    GUINT_TO_POINTER(menu->parent_id),
			                    current_section_item);
		}
		else
		{
			g_ptr_array_add(((DBusMenuModel *)current_section)->items, new_item);
			g_hash_table_insert(menu->dbusmenu_section_items,
			                    GUINT_TO_POINTER(menu->parent_id),
			                    new_item);
		}
		g_variant_unref(cprops);
        g_variant_unref(citems);
		g_variant_unref(child);
	}
	g_variant_unref(items);
}

static void get_layout_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	GVariant *layout;
	guint revision;
	DBusMenuModel *menu;

	g_autoptr(GError) error = NULL;
	dbus_menu_xml_call_get_layout_finish((DBusMenuXml *)(source_object),
	                                     &revision,
	                                     &layout,
	                                     res,
	                                     &error);

	if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
	{
		return;
	}

	menu = (DBusMenuModel *)(user_data);

	if (error != NULL)
	{
		g_warning("%s", error->message);
		return;
	}
	uint old_num = menu->items->len;
	g_hash_table_remove_all(menu->dbusmenu_section_items);
	if (old_num > 0)
		g_ptr_array_remove_range(menu->items, 0, old_num);
	layout_parse(menu, layout);
	uint new_num = menu->items->len;
    g_menu_model_items_changed(G_MENU_MODEL(menu), 0, old_num, new_num);
    GString* str = g_string_new(NULL);
    g_menu_markup_print_string(str, menu, 4, 4);
    char* cstr = g_string_free(str,false);
    g_print("%s\n",cstr);
    g_variant_unref(layout);
}
G_GNUC_INTERNAL void dbus_menu_model_update_layout(DBusMenuModel *menu)
{
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
	if ((uint)parent == menu->parent_id)
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

static void items_properties_updated_cb(DBusMenuXml *proxy, GVariant *updated_props,
                                        GVariant *removed_props, DBusMenuModel *menu)
{
	GVariantIter iter;
	guint id;
	GVariant *props;
	DBusMenuItem *item;

	g_variant_iter_init(&iter, updated_props);
	while (g_variant_iter_next(&iter, "(i@a{sv})", &id, &props))
	{
		item = (DBusMenuItem *)g_hash_table_lookup(menu->dbusmenu_section_items,
		                                           GUINT_TO_POINTER(id));

		if (item != NULL)
			dbus_menu_item_update_props(item, props);

		g_variant_unref(props);
	}

	g_variant_iter_init(&iter, removed_props);
	while (g_variant_iter_next(&iter, "(i@as)", &id, &props))
	{
		item = (DBusMenuItem *)g_hash_table_lookup(menu->dbusmenu_section_items,
		                                           GUINT_TO_POINTER(id));

		if (item != NULL)
			dbus_menu_item_remove_props(item, props);

		g_variant_unref(props);
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
	                                                   "is-section-model",
	                                                   true,
	                                                   NULL);
	if (parent != NULL)
		g_object_bind_property(parent, "xml", ret, "xml", G_BINDING_SYNC_CREATE);
	return ret;
}

DBusMenuModel *dbus_menu_model_new(uint parent_id, DBusMenuModel *parent, DBusMenuXml *xml,
                                   GActionGroup *action_group)
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
	dbus_menu_model_update_layout(ret);
	return ret;
}

static void dbus_menu_model_finalize(GObject *object)
{
	DBusMenuModel *menu = (DBusMenuModel *)(object);
	g_cancellable_cancel(menu->cancellable);
	g_clear_object(&menu->cancellable);
	g_ptr_array_free(menu->items, true);

	G_OBJECT_CLASS(dbus_menu_model_parent_class)->finalize(object);
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
		//        if (menu->xml != NULL && old_xml != menu->xml)
		//			on_xml_property_changed(menu);
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

static void dbus_menu_model_init(DBusMenuModel *menu)
{
	menu->cancellable            = g_cancellable_new();
	menu->items                  = g_ptr_array_new_with_free_func(dbus_menu_item_free);
	menu->dbusmenu_section_items = g_hash_table_new(g_direct_hash, g_direct_equal);
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
	                                    G_PARAM_STATIC_STRINGS));
	properties[PROP_IS_SECTION_MODEL] =
	    g_param_spec_boolean("is-section-model",
	                         "is-section-model",
	                         "is-section-model",
	                         false,
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
