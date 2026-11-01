// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exporter.h"
#include "dbusmenu-interface.h"
#include "exporter-item.h"
#include "glib-object.h"
#include "gtkactionmuxer.h"
#include "gtkmenutracker.h"
#include "gtkmenutrackeritem.h"

struct _DBusMenuExporter
{
	GCancellable *cancellable;
	DBusMenuXml *xml;
	gchar *bus_name;
	ulong name_id;
	gchar *object_path;
	GMenuModel *source_menu;
	GtkActionMuxer *source_actions;
	GPtrArray *items;
	GSList *free_ids;
	guint revision;

	GHashTable *pending_properties; // Ключ: guint (ID элемента), Значение: GVariantDict (Имя
	                                // свойства -> GVariant)
	guint flush_timeout_id;
};

enum
{
	PROP_NULL,
	PROP_BUS_NAME,
	PROP_OBJECT_PATH,
	PROP_MODEL,
	PROP_ACTION_GROUP,
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };
G_DEFINE_TYPE(DBusMenuExporter, dbus_menu_exporter, G_TYPE_OBJECT)

static void dbus_menu_exporter_tracker_add_func(GtkMenuTrackerItem *item, gint position,
                                                gpointer user_data);
static void dbus_menu_exporter_tracker_remove_func(gint position, gpointer user_data);

G_GNUC_INTERNAL DBusMenuExporterItem *dbus_menu_exporter_get_by_id(DBusMenuExporter *self,
                                                                   u_int32_t id)
{
	if (id >= self->items->len)
		return NULL;
	return (DBusMenuExporterItem *)g_ptr_array_index(self->items, id);
}

u_int32_t dbus_menu_exporter_get_free_id(DBusMenuExporter *self)
{
	if (self->free_ids)
	{
		u_int32_t id;
		id             = GPOINTER_TO_UINT(self->free_ids->data);
		self->free_ids = g_slist_remove(self->free_ids, self->free_ids->data);
		return id;
	}
	return self->items->len;
}

static gboolean dbus_menu_exporter_flush_properties_cb(gpointer user_data)
{
	DBusMenuExporter *self = DBUS_MENU_EXPORTER(user_data);

	if (g_hash_table_size(self->pending_properties) == 0)
	{
		self->flush_timeout_id = 0;
		return G_SOURCE_REMOVE;
	}

	GVariantBuilder updated_builder;
	GVariantBuilder removed_builder;

	// a(ia{sv}) - (id, dict), so, just use id + GVariantDict
	g_variant_builder_init(&updated_builder, G_VARIANT_TYPE("a(ia{sv})"));
	// removed properties always empty - engine limitation
	g_variant_builder_init(&removed_builder, G_VARIANT_TYPE("a(ias)"));

	GHashTableIter iter;
	gpointer key_id, value_table;
	g_hash_table_iter_init(&iter, self->pending_properties);

	while (g_hash_table_iter_next(&iter, &key_id, &value_table))
	{
		guint item_id       = GPOINTER_TO_UINT(key_id);
		GVariantDict *props = (GVariantDict *)value_table;

		g_variant_builder_open(&updated_builder, G_VARIANT_TYPE("(ia{sv})"));
		g_variant_builder_add(&updated_builder, "i", (gint)item_id);
		g_variant_builder_add(&updated_builder, "a{sv}", g_variant_dict_end(props));
		g_variant_builder_close(&updated_builder);
	}
	g_hash_table_remove_all(self->pending_properties);

	GVariant *updated_variant = g_variant_builder_end(&updated_builder);
	GVariant *removed_variant = g_variant_builder_end(&removed_builder);
	dbus_menu_xml_emit_items_properties_updated(self->xml, updated_variant, removed_variant);

	self->flush_timeout_id = 0;
	return G_SOURCE_REMOVE;
}

G_GNUC_INTERNAL void dbus_menu_exporter_queue_property_change(DBusMenuExporter *self,
                                                              DBusMenuExporterItem *item,
                                                              const gchar *dbus_prop)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(item != NULL);

	GVariant *value = dbus_menu_exporter_item_get_variant_for_prop(item, dbus_prop);
	if (!value)
		return;

	gpointer id_ptr          = GUINT_TO_POINTER(item->id);
	GVariantDict *item_props = g_hash_table_lookup(self->pending_properties, id_ptr);
	if (!item_props)
	{
		item_props = g_variant_dict_new(NULL);
		g_hash_table_insert(self->pending_properties, id_ptr, item_props);
	}

	g_variant_dict_insert_value(item_props, dbus_prop, value);
	if (self->flush_timeout_id == 0)
		self->flush_timeout_id =
		    g_timeout_add(200, dbus_menu_exporter_flush_properties_cb, self);
}

static gboolean check_invocation(DBusMenuExporter *self, GDBusMethodInvocation *invocation, gint id)
{
	DBusMenuExporterItem *item;
	if (id < 0 || (item = dbus_menu_exporter_get_by_id(self, id)) == NULL)
	{
		g_dbus_method_invocation_return_error(invocation,
		                                      G_DBUS_ERROR,
		                                      G_DBUS_ERROR_INVALID_ARGS,
		                                      "Unknown menu item id %d",
		                                      id);
		return FALSE;
	}
	return TRUE;
}

static gboolean dbus_menu_exporter_handle_about_to_show(DBusMenuXml *object,
                                                        GDBusMethodInvocation *invocation, gint id,
                                                        gpointer user_data)
{
	DBusMenuExporter *self     = DBUS_MENU_EXPORTER(user_data);
	DBusMenuExporterItem *item = dbus_menu_exporter_get_by_id(self, id);
	if (!check_invocation(self, invocation, id))
		return TRUE;
	dbus_menu_exporter_item_about_to_show(item);
	dbus_menu_xml_complete_about_to_show(object, invocation, FALSE);
	return TRUE;
}

static gboolean dbus_menu_exporter_handle_event(DBusMenuXml *object,
                                                GDBusMethodInvocation *invocation, gint id,
                                                const gchar *event_id, GVariant *data,
                                                guint timestamp, gpointer user_data)
{
	DBusMenuExporter *self     = DBUS_MENU_EXPORTER(user_data);
	DBusMenuExporterItem *item = dbus_menu_exporter_get_by_id(self, id);
	if (!check_invocation(self, invocation, id))
		return TRUE;
	dbus_menu_exporter_item_event(item, event_id);
	dbus_menu_xml_complete_event(object, invocation);
	return TRUE;
}

static gboolean dbus_menu_exporter_handle_get_property(DBusMenuXml *object,
                                                       GDBusMethodInvocation *invocation, gint id,
                                                       const gchar *property, gpointer user_data)
{
	DBusMenuExporter *self     = DBUS_MENU_EXPORTER(user_data);
	DBusMenuExporterItem *item = dbus_menu_exporter_get_by_id(self, id);
	GVariant *value            = NULL;
	if (!check_invocation(self, invocation, id))
		return TRUE;
	value = dbus_menu_exporter_item_get_variant_for_prop(item, property);
	if (!value)
	{
		g_dbus_method_invocation_return_error(invocation,
		                                      G_DBUS_ERROR,
		                                      G_DBUS_ERROR_INVALID_ARGS,
		                                      "Unknown property '%s'",
		                                      property);
		return TRUE;
	}
	dbus_menu_xml_complete_get_property(object, invocation, value);
	return TRUE;
}

static void dbus_menu_exporter_tracker_add_func(GtkMenuTrackerItem *item, gint position,
                                                gpointer user_data)
{
	DBusMenuExporterItem *parent = (DBusMenuExporterItem *)user_data;
	DBusMenuExporter *self       = dbus_menu_exporter_item_get_exporter(parent);
	u_int32_t id                 = dbus_menu_exporter_get_free_id(self);
	DBusMenuExporterItem *child  = dbus_menu_exporter_item_new(self, id, item, parent);

	g_ptr_array_insert(self->items, id, child);
	dbus_menu_exporter_item_add_child(parent, child, position);

	if (gtk_menu_tracker_item_get_has_link(item, G_MENU_LINK_SUBMENU))
	{
		child->child_tracker =
		    gtk_menu_tracker_new_for_item_link(item,
		                                       G_MENU_LINK_SUBMENU,
		                                       TRUE,
		                                       FALSE,
		                                       dbus_menu_exporter_tracker_add_func,
		                                       dbus_menu_exporter_tracker_remove_func,
		                                       child);
		child->is_submenu = TRUE;
	}
	dbus_menu_xml_emit_layout_updated(self->xml, self->revision++, parent->id);
}

static void dbus_menu_exporter_tracker_remove_func(gint position, gpointer user_data)
{
	DBusMenuExporterItem *parent = (DBusMenuExporterItem *)user_data;
	DBusMenuExporter *self       = dbus_menu_exporter_item_get_exporter(parent);

	if (position >= parent->children->len)
	{
		g_warning("Child not found with pos %d\n", position);
		return;
	}
	u_int32_t id = g_array_index(parent->children, u_int32_t, position);
	if (id >= self->items->len || id == 0)
	{
		g_warning("Child not found with id %d\n", id);
		return;
	}
	DBusMenuExporterItem *child = dbus_menu_exporter_get_by_id(self, id);
	if (!child)
	{
		g_warning("double remove id %u", id);
		return;
	}
	dbus_menu_exporter_item_remove_child(parent, position);
	dbus_menu_exporter_item_free(child);
	g_ptr_array_index(self->items, id) = NULL;
	self->free_ids                     = g_slist_prepend(self->free_ids, GUINT_TO_POINTER(id));
	dbus_menu_xml_emit_layout_updated(self->xml, self->revision++, parent->id);
}

static void name_acquired_cb(GDBusConnection *conn, const gchar *name, void *user_data)
{
	DBusMenuExporter *self  = DBUS_MENU_EXPORTER(user_data);
	g_autoptr(GError) error = NULL;

	// Default properties
	g_object_set(self->xml, "version", 3, "status", "normal", "text-direction", "none", NULL);
	g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(self->xml),
	                                 conn,
	                                 self->object_path,
	                                 &error);
	if (error)
	{
		g_warning("%s", error->message);
		return;
	}
}

static void name_lost_cb(GDBusConnection *connection, const char *name, gpointer user_data)
{
	DBusMenuExporter *menu = DBUS_MENU_EXPORTER(user_data);

	g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(menu->xml),
	                                                   connection);
}

static void dbus_menu_exporter_set_property(GObject *object, guint property_id, const GValue *value,
                                            GParamSpec *pspec)
{
	DBusMenuExporter *menu = DBUS_MENU_EXPORTER(object);

	switch (property_id)
	{
	case PROP_BUS_NAME:
		menu->bus_name = g_value_dup_string(value);
		break;

	case PROP_OBJECT_PATH:
		menu->object_path = g_value_dup_string(value);
		break;

	case PROP_MODEL:
		g_clear_object(&menu->source_menu);
		menu->source_menu = g_value_dup_object(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void dbus_menu_exporter_get_property(GObject *object, guint property_id, GValue *value,
                                            GParamSpec *pspec)
{
	DBusMenuExporter *menu = DBUS_MENU_EXPORTER(object);
	switch (property_id)
	{
	case PROP_BUS_NAME:
		g_value_set_static_string(value, menu->bus_name);
		break;

	case PROP_OBJECT_PATH:
		g_value_set_static_string(value, menu->object_path);
		break;

	case PROP_MODEL:
		g_value_set_object(value, menu->source_menu);
		break;
	case PROP_ACTION_GROUP:
		g_value_set_object(value, menu->source_actions);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void install_properties(GObjectClass *object_class)
{
	properties[PROP_BUS_NAME] = g_param_spec_string("bus-name",
	                                                "bus-name",
	                                                "bus-name",
	                                                NULL,
	                                                G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
	                                                    G_PARAM_STATIC_STRINGS);

	properties[PROP_OBJECT_PATH] =
	    g_param_spec_string("object-path",
	                        "object-path",
	                        "object-path",
	                        NULL,
	                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
	                            G_PARAM_STATIC_STRINGS);

	properties[PROP_MODEL] = g_param_spec_object("model",
	                                             "model",
	                                             "model",
	                                             G_TYPE_MENU_MODEL,
	                                             G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
	                                                 G_PARAM_STATIC_STRINGS);
	properties[PROP_ACTION_GROUP] =
	    g_param_spec_object("action-group",
	                        "action-group",
	                        "action-group",
	                        G_TYPE_ACTION_GROUP,
	                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, LAST_PROP, properties);
}

static void dbus_menu_exporter_constructed(GObject *object)
{
	G_OBJECT_CLASS(dbus_menu_exporter_parent_class)->constructed(object);
	DBusMenuExporter *menu = DBUS_MENU_EXPORTER(object);

	// Connect signals
	g_signal_connect(menu->xml,
	                 "handle-about-to-show",
	                 G_CALLBACK(dbus_menu_exporter_handle_about_to_show),
	                 menu);
	g_signal_connect(menu->xml,
	                 "handle-event",
	                 G_CALLBACK(dbus_menu_exporter_handle_event),
	                 menu);
	g_signal_connect(menu->xml,
	                 "handle-get-property",
	                 G_CALLBACK(dbus_menu_exporter_handle_get_property),
	                 menu);

	DBusMenuExporterItem *root = (DBusMenuExporterItem *)g_ptr_array_index(menu->items, 0);
	menu->xml                  = dbus_menu_xml_skeleton_new();
	root->child_tracker = gtk_menu_tracker_new(GTK_ACTION_OBSERVABLE(menu->source_actions),
	                                           menu->source_menu,
	                                           TRUE,
	                                           TRUE,
	                                           FALSE,
	                                           NULL,
	                                           dbus_menu_exporter_tracker_add_func,
	                                           dbus_menu_exporter_tracker_remove_func,
	                                           root);
	root->is_submenu    = TRUE;

	menu->name_id = g_bus_own_name(G_BUS_TYPE_SESSION,
	                               menu->bus_name,
	                               G_BUS_NAME_OWNER_FLAGS_DO_NOT_QUEUE,
	                               NULL,
	                               name_acquired_cb,
	                               name_lost_cb,
	                               menu,
	                               NULL);
}

static void dbus_menu_exporter_dispose(GObject *object)
{
	DBusMenuExporter *menu = DBUS_MENU_EXPORTER(object);

	g_cancellable_cancel(menu->cancellable);

	if (menu->name_id)
	{
		g_bus_unown_name(menu->name_id);
		menu->name_id = 0;
	}

	g_signal_handlers_disconnect_by_data(menu, menu);
	G_OBJECT_CLASS(dbus_menu_exporter_parent_class)->dispose(object);
}

static void dbus_menu_exporter_finalize(GObject *object)
{
	DBusMenuExporter *menu = DBUS_MENU_EXPORTER(object);

	if (menu->flush_timeout_id)
	{
		g_source_remove(menu->flush_timeout_id);
		menu->flush_timeout_id = 0;
	}
	g_clear_pointer(&menu->pending_properties, g_hash_table_destroy);

	g_clear_object(&menu->cancellable);
	g_clear_object(&menu->source_menu);
	g_clear_object(&menu->xml);
	g_clear_object(&menu->source_actions);
	g_clear_pointer(&menu->bus_name, g_free);
	g_clear_pointer(&menu->object_path, g_free);
	g_clear_pointer(&menu->free_ids, g_slist_free);
	g_clear_pointer(&menu->items, g_ptr_array_unref);

	G_OBJECT_CLASS(dbus_menu_exporter_parent_class)->finalize(object);
}

static void dbus_menu_exporter_class_init(DBusMenuExporterClass *menu_class)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(menu_class);

	object_class->constructed  = dbus_menu_exporter_constructed;
	object_class->dispose      = dbus_menu_exporter_dispose;
	object_class->finalize     = dbus_menu_exporter_finalize;
	object_class->set_property = dbus_menu_exporter_set_property;
	object_class->get_property = dbus_menu_exporter_get_property;

	install_properties(object_class);
}

static void dbus_menu_exporter_init(DBusMenuExporter *menu)
{
	menu->free_ids = NULL;
	menu->items    = g_ptr_array_new_full(1, dbus_menu_exporter_item_free);
	g_ptr_array_set_size(menu->items, 1);
	g_ptr_array_index(menu->items, 0) = dbus_menu_exporter_item_new(menu, 0, NULL, NULL);
	menu->source_actions              = gtk_action_muxer_new();
	menu->cancellable                 = g_cancellable_new();
	menu->pending_properties          = g_hash_table_new_full(g_direct_hash,
                                                         g_direct_equal,
                                                         NULL,
                                                         (GDestroyNotify)g_variant_unref);
	menu->flush_timeout_id            = 0;
}
