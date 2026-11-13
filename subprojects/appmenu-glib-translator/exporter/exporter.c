// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exporter.h"
#include "dbusmenu-interface.h"
#include "exporter-item.h"
#include "glib-object.h"
#include "exporter-muxer.h"
#include "gtkmenutracker.h"
#include "gtkmenutrackeritem.h"

#include <stdbool.h>

struct _DBusMenuExporter
{
	GObject parent_instance;
	DBusMenuXml *xml;
	gchar *bus_name;
	guint name_id;
	gchar *object_path;
	GMenuModel *source_menu;
	DBusMenuExporterMuxer *source_actions;
	GPtrArray *items;
	GSList *free_ids;
	guint revision;

	GHashTable *pending_properties;
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

static void dbus_menu_exporter_tracker_add_func(DBusMenuExporterTrackerItem *item, gint position,
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

	g_variant_builder_init(&updated_builder, G_VARIANT_TYPE("a(ia{sv})"));
	g_variant_builder_init(&removed_builder, G_VARIANT_TYPE("a(ias)"));

	GHashTableIter iter;
	gpointer key_id, value_table;
	g_hash_table_iter_init(&iter, self->pending_properties);

	while (g_hash_table_iter_next(&iter, &key_id, &value_table))
	{
		guint item_id       = GPOINTER_TO_UINT(key_id);
		GHashTable *props   = (GHashTable *)value_table;
		gpointer propd, valued;
		GHashTableIter niter;

		bool has_updated = false, has_removed = false;
		GVariantBuilder item_updated, item_removed;
		g_variant_builder_init(&item_updated, G_VARIANT_TYPE("a{sv}"));
		g_variant_builder_init(&item_removed, G_VARIANT_TYPE("as"));

		g_hash_table_iter_init(&niter, props);
		while (g_hash_table_iter_next(&niter, &propd, &valued))
		{
			const char* prop = (const char*)propd;
			GVariant* value = (GVariant*)valued;

			if (g_variant_is_of_type(value, G_VARIANT_TYPE("mv")))
			{
				g_variant_builder_add(&item_removed, "s", prop);
				has_removed = true;
			}
			else
			{
				g_variant_builder_add(&item_updated, "{sv}", prop, value);
				has_updated = true;
			}
		}
		if (has_updated)
		{
			g_variant_builder_open(&updated_builder, G_VARIANT_TYPE("(ia{sv})"));
			g_variant_builder_add(&updated_builder, "i", (gint)item_id);
			g_variant_builder_add_value(&updated_builder, g_variant_builder_end(&item_updated));
			g_variant_builder_close(&updated_builder);
		}

		if (has_removed)
		{
			g_variant_builder_open(&removed_builder, G_VARIANT_TYPE("(ias)"));
			g_variant_builder_add(&removed_builder, "i", (gint)item_id);
			g_variant_builder_add_value(&removed_builder, g_variant_builder_end(&item_removed));
			g_variant_builder_close(&removed_builder);
		}
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
		value = g_variant_new_maybe(G_VARIANT_TYPE_VARIANT, NULL);

	gpointer id_ptr          = GUINT_TO_POINTER(item->id);
	GHashTable *item_props = g_hash_table_lookup(self->pending_properties, id_ptr);
	if (!item_props)
	{
		item_props = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)g_variant_unref);
		g_hash_table_insert(self->pending_properties, id_ptr, item_props);
	}

	g_hash_table_insert(item_props, (gpointer)dbus_prop, g_variant_ref_sink(value));
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

static gboolean dbus_menu_exporter_handle_about_to_show_group(DBusMenuXml *object,
                                                              GDBusMethodInvocation *invocation,
                                                              GVariant *ids,
                                                              gpointer user_data)
{
	DBusMenuExporter *self = DBUS_MENU_EXPORTER(user_data);
	GVariantBuilder updates_builder, errors_builder;
	GVariantIter iter;
	gint32 id;

	g_variant_builder_init(&updates_builder, G_VARIANT_TYPE("ai"));
	g_variant_builder_init(&errors_builder, G_VARIANT_TYPE("ai"));

	g_variant_iter_init(&iter, ids);
	while (g_variant_iter_next(&iter, "i", &id))
	{
		DBusMenuExporterItem *item = dbus_menu_exporter_get_by_id(self, id);

		if (id < 0 || item == NULL)
		{
			g_variant_builder_add(&errors_builder, "i", id);
			continue;
		}

		dbus_menu_exporter_item_about_to_show(item);
		g_variant_builder_add(&updates_builder, "i", id);
	}

	dbus_menu_xml_complete_about_to_show_group(object, invocation,
												g_variant_builder_end(&updates_builder),
												g_variant_builder_end(&errors_builder));
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

static gboolean dbus_menu_exporter_handle_event_group(DBusMenuXml *object,
                                                      GDBusMethodInvocation *invocation,
                                                      GVariant *events,
                                                      gpointer user_data)
{
	DBusMenuExporter *self = DBUS_MENU_EXPORTER(user_data);
	GVariantBuilder errors_builder;
	GVariantIter iter;
	gint32 id;
	const gchar *event_id;
	GVariant *data;
	guint32 timestamp;

	g_variant_builder_init(&errors_builder, G_VARIANT_TYPE("ai"));

	g_variant_iter_init(&iter, events);
	while (g_variant_iter_next(&iter, "(i&s&vu)", &id, &event_id, &data, &timestamp))
	{
		DBusMenuExporterItem *item = dbus_menu_exporter_get_by_id(self, id);

		if (id < 0 || item == NULL)
		{
			g_variant_builder_add(&errors_builder, "i", id);
			continue;
		}

		dbus_menu_exporter_item_event(item, event_id);
	}

	dbus_menu_xml_complete_event_group(object, invocation,
										g_variant_builder_end(&errors_builder));
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

static gboolean dbus_menu_exporter_handle_get_group_properties(DBusMenuXml *object,
                                                               GDBusMethodInvocation *invocation,
                                                               GVariant *ids,
                                                               const gchar *const *requested_props,
                                                               gpointer user_data)
{
	DBusMenuExporter *self = DBUS_MENU_EXPORTER(user_data);
	GVariantBuilder items_builder;
	GVariantIter iter;
	gint32 id;

	g_variant_builder_init(&items_builder, G_VARIANT_TYPE("a(ia{sv})"));

	g_variant_iter_init(&iter, ids);
	while (g_variant_iter_next(&iter, "i", &id))
	{
		DBusMenuExporterItem *item = dbus_menu_exporter_get_by_id(self, id);

		if (id < 0 || item == NULL)
			continue;

		GVariant *props = dbus_menu_exporter_item_build_properties_variant(item,
																			requested_props);
		g_variant_builder_add(&items_builder, "(i@a{sv})", id, props);
	}

	dbus_menu_xml_complete_get_group_properties(object, invocation,
												g_variant_builder_end(&items_builder));
	return TRUE;
}

static gboolean dbus_menu_exporter_handle_get_layout(DBusMenuXml *object,
                                                     GDBusMethodInvocation *invocation,
                                                     gint parent_id, gint arg_recursionDepth,
                                                     const gchar *const *arg_propertyNames,
                                                     gpointer user_data)
{
	DBusMenuExporter *self = DBUS_MENU_EXPORTER(user_data);
	DBusMenuExporterItem *item;
	GVariant *layout = NULL;

	if (!check_invocation(self, invocation, parent_id))
		return TRUE;
	item = dbus_menu_exporter_get_by_id(self, parent_id);
	if (item)
		layout = dbus_menu_exporter_item_build_representation(item,
		                                                      arg_recursionDepth,
		                                                      arg_propertyNames);
	else
		layout = g_variant_new_parsed("(ia{sv}av)", parent_id, "", "");
	dbus_menu_xml_complete_get_layout(object, invocation, self->revision, layout);
	return TRUE;
}

static void dbus_menu_exporter_tracker_add_func(DBusMenuExporterTrackerItem *item, gint position,
                                                gpointer user_data)
{
	DBusMenuExporterItem *parent = (DBusMenuExporterItem *)user_data;
	DBusMenuExporter *self       = dbus_menu_exporter_item_get_exporter(parent);
	u_int32_t id                 = dbus_menu_exporter_get_free_id(self);
	DBusMenuExporterItem *child  = dbus_menu_exporter_item_new(self, id, item, parent);

	if(id < self->items->len)
		g_ptr_array_index(self->items, id) = child;
	else
		g_ptr_array_add(self->items, child);
	dbus_menu_exporter_item_add_child(parent, child, position);

	if (dbus_menu_exporter_tracker_item_get_has_link(item, G_MENU_LINK_SUBMENU))
	{
		child->is_submenu = TRUE;
		child->child_tracker =
		    gtk_menu_tracker_new_for_item_link(item,
		                                       G_MENU_LINK_SUBMENU,
		                                       TRUE,
		                                       FALSE,
		                                       dbus_menu_exporter_tracker_add_func,
		                                       dbus_menu_exporter_tracker_remove_func,
		                                       child);
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
	menu->xml                  = dbus_menu_xml_skeleton_new();

	// Connect signals
	g_signal_connect(menu->xml,
	                 "handle-about-to-show",
	                 G_CALLBACK(dbus_menu_exporter_handle_about_to_show),
	                 menu);
	g_signal_connect(menu->xml,
	                 "handle-about-to-show-group",
	                 G_CALLBACK(dbus_menu_exporter_handle_about_to_show_group),
	                 menu);
	g_signal_connect(menu->xml,
	                 "handle-event",
	                 G_CALLBACK(dbus_menu_exporter_handle_event),
	                 menu);
	g_signal_connect(menu->xml,
	                 "handle-event-group",
	                 G_CALLBACK(dbus_menu_exporter_handle_event_group),
	                 menu);
	g_signal_connect(menu->xml,
	                 "handle-get-property",
	                 G_CALLBACK(dbus_menu_exporter_handle_get_property),
	                 menu);
	g_signal_connect(menu->xml,
	                 "handle-get-group-properties",
	                 G_CALLBACK(dbus_menu_exporter_handle_get_group_properties),
	                 menu);
	g_signal_connect(menu->xml,
	                 "handle-get-layout",
	                 G_CALLBACK(dbus_menu_exporter_handle_get_layout),
	                 menu);

	DBusMenuExporterItem *root = (DBusMenuExporterItem *)g_ptr_array_index(menu->items, 0);
	root->child_tracker = gtk_menu_tracker_new(dbus_menu_exporter_muxer_get_node(menu->source_actions),
	                                           menu->source_menu,
	                                           TRUE,
	                                           TRUE,
	                                           FALSE,
	                                           NULL,
	                                           dbus_menu_exporter_tracker_add_func,
	                                           dbus_menu_exporter_tracker_remove_func,
	                                           root);
	root->is_submenu    = TRUE;

	if(g_dbus_is_unique_name(menu->bus_name)) {
		g_autoptr(GDBusConnection) conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
		if(conn)
			name_acquired_cb(conn, menu->bus_name, menu);
	}
	else 
    {
		menu->name_id = g_bus_own_name(G_BUS_TYPE_SESSION,
									menu->bus_name,
									G_BUS_NAME_OWNER_FLAGS_DO_NOT_QUEUE,
									NULL,
									name_acquired_cb,
									name_lost_cb,
									menu,
									NULL);
	}
}

static void dbus_menu_exporter_dispose(GObject *object)
{
	DBusMenuExporter *menu = DBUS_MENU_EXPORTER(object);

	g_clear_handle_id(&menu->flush_timeout_id, g_source_remove);
	g_clear_handle_id(&menu->name_id, g_bus_unown_name);

	if (menu->xml)
	{
		if (g_dbus_is_unique_name(menu->bus_name))
		{
			GDBusConnection *conn = g_dbus_interface_skeleton_get_connection(
			    G_DBUS_INTERFACE_SKELETON(menu->xml));
			if (conn)
				name_lost_cb(conn, menu->bus_name, menu);
		}

		g_signal_handlers_disconnect_by_data(menu->xml, menu);
		g_clear_object(&menu->xml);
	}

	g_clear_object(&menu->source_menu);
	g_clear_object(&menu->source_actions);
	g_clear_pointer(&menu->items, g_ptr_array_unref);

	G_OBJECT_CLASS(dbus_menu_exporter_parent_class)->dispose(object);
}

static void dbus_menu_exporter_finalize(GObject *object)
{
	DBusMenuExporter *menu = DBUS_MENU_EXPORTER(object);

	g_clear_pointer(&menu->pending_properties, g_hash_table_destroy);
	g_clear_pointer(&menu->bus_name, g_free);
	g_clear_pointer(&menu->object_path, g_free);
	g_clear_pointer(&menu->free_ids, g_slist_free);

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
	menu->source_actions              = dbus_menu_exporter_muxer_new();
	menu->pending_properties          = g_hash_table_new_full(g_direct_hash,
                                                         g_direct_equal,
                                                         NULL,
                                                         (GDestroyNotify)g_hash_table_unref);
	menu->flush_timeout_id            = 0;
}

void dbus_menu_exporter_insert(DBusMenuExporter* self, const gchar* prefix, GActionGroup* group)
{
	if(prefix && group)
		dbus_menu_exporter_muxer_insert(self->source_actions, prefix, group);
}

void dbus_menu_exporter_remove(DBusMenuExporter* self, const gchar* prefix)
{
	if(prefix)
		dbus_menu_exporter_muxer_remove(self->source_actions, prefix);
}

DBusMenuExporter *dbus_menu_exporter_new(GMenuModel *menu_model, gchar *action_prefix,
                                         GActionGroup *action_group, gchar *bus_name,
                                         gchar *object_path)
{
	DBusMenuExporter* self =  DBUS_MENU_EXPORTER(g_object_new(dbus_menu_exporter_get_type(),
						"model",
						menu_model,
	                    "bus-name",
	                    bus_name,
	                    "object-path",
	                    object_path,
	                    NULL));
	dbus_menu_exporter_insert(self, action_prefix, action_group);
	return self;
}
