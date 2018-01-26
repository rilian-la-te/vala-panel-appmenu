#include "section.h"
#include "item.h"

struct _DBusMenuSectionModel
{
	GMenuModel parent_instance;

	DBusMenuModel *parent_model;
	GSequenceIter *begin_iter;
	GSequenceIter *end_iter;
	uint section_index;
};

enum
{
	PROP_NULL          = 0,
	PROP_PARENT_MODEL  = 1,
	PROP_SECTION_INDEX = 2,
	NUM_PROPS
};

static GParamSpec *properties[NUM_PROPS] = { NULL };
static void dbus_menu_section_model_update_iters(DBusMenuSectionModel *menu);

G_DEFINE_TYPE(DBusMenuSectionModel, dbus_menu_section_model, G_TYPE_MENU_MODEL)

static bool dbus_menu_section_model_is_mutable(GMenuModel *model)
{
	return true;
}

static gint dbus_menu_section_model_get_n_items(GMenuModel *model)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(model);
	dbus_menu_section_model_update_iters(menu);

	return g_sequence_iter_get_position(menu->end_iter) -
	       g_sequence_iter_get_position(menu->begin_iter);
}

static void dbus_menu_section_model_get_item_attributes(GMenuModel *model, gint position,
                                                        GHashTable **table)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(model);
	dbus_menu_section_model_update_iters(menu);
	GSequence *all_items = g_sequence_iter_get_sequence(menu->begin_iter);
	int index            = position + g_sequence_iter_get_position(menu->begin_iter);
	DBusMenuItem *item =
	    (DBusMenuItem *)g_sequence_get(g_sequence_get_iter_at_pos(all_items, index));
	*table = g_hash_table_ref(item->attributes);
}

static void dbus_menu_section_model_get_item_links(GMenuModel *model, gint position,
                                                   GHashTable **table)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(model);
	dbus_menu_section_model_update_iters(menu);
	GSequence *all_items = g_sequence_iter_get_sequence(menu->begin_iter);
	int index            = position + g_sequence_iter_get_position(menu->begin_iter);
	DBusMenuItem *item =
	    (DBusMenuItem *)g_sequence_get(g_sequence_get_iter_at_pos(all_items, index));
	*table = g_hash_table_ref(item->links);
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

static void dbus_menu_section_model_update_iters(DBusMenuSectionModel *menu)
{
	GSequenceIter *begin_iter =
	    dbus_menu_model_get_section_iter(menu->parent_model, menu->section_index);
	menu->begin_iter = g_sequence_iter_next(begin_iter);
	GSequenceIter *end_iter =
	    dbus_menu_model_get_section_iter(menu->parent_model, menu->section_index + 1);
	if (g_sequence_iter_is_end(end_iter))
		menu->end_iter = end_iter;
	else
		menu->end_iter = g_sequence_iter_prev(end_iter);
}

static void dbus_menu_section_model_set_property(GObject *object, guint property_id,
                                                 const GValue *value, GParamSpec *pspec)
{
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(object);

	switch (property_id)
	{
	case PROP_PARENT_MODEL:
		menu->parent_model = DBUS_MENU_MODEL(g_value_get_object(value));
		if (menu->begin_iter != NULL)
			dbus_menu_section_model_update_iters(menu);
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
	DBusMenuSectionModel *menu = DBUS_MENU_SECTION_MODEL(object);
	dbus_menu_section_model_update_iters(menu);
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
