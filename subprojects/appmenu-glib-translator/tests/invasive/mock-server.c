/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* com.canonical.dbusmenu server whose whole content is a caller-supplied layout
 * variant. It implements every method and property of the interface, but it has
 * no GMenuModel behind it: GetLayout always answers with the very variant that
 * was passed to the constructor.
 *
 * The real exporter derives its layouts from a model, so this class exists to
 * drive the importer with layouts the exporter cannot produce. */

#include <string.h>

#include "comparator.h"
#include "dbusmenu-interface.h"
#include "definitions.h"
#include "mock-server.h"

typedef struct
{
	GVariantBuilder *builder;
} PropFilter;

struct _DBusMenuMockServer
{
	GObject parent_instance;
	GVariant *layout;
	/* id -> property dictionary (a{sv}) borrowed from @layout. */
	GHashTable *layout_index;
	gint32 root_id;

	DBusMenuXml *skeleton;
	gchar *bus_name;
	gchar *object_path;
	guint name_id;
	gboolean exported;

	guint revision;
};

struct _DBusMenuMockServerClass
{
	GObjectClass parent_class;
};

G_DEFINE_TYPE(DBusMenuMockServer, dbus_menu_mock_server, G_TYPE_OBJECT)

static void layout_menu_name_acquired(GDBusConnection *connection, const char *name,
                                      gpointer user_data);
static void layout_menu_name_lost(GDBusConnection *connection, const char *name,
                                  gpointer user_data);

enum
{
	PROP_NULL = 0,
	PROP_LAYOUT,
	PROP_BUS_NAME,
	PROP_OBJECT_PATH,
	LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

/* Recursively collects the property dictionaries of a (ia{sv}av) subtree. They
 * are copied out of @item so that the index owns them and stays valid no matter
 * what happens to the caller's own reference to the layout. */
static void index_layout(GVariant *item, GHashTable *index)
{
	g_autoptr(GVariant) props    = NULL;
	g_autoptr(GVariant) children = NULL;

	if (!g_variant_is_of_type(item, G_VARIANT_TYPE("(ia{sv}av)")))
		return;

	gint32 id = -1;
	g_variant_get(item, "(i@a{sv}@av)", &id, &props, &children);
	g_hash_table_replace(index, GINT_TO_POINTER(id), g_variant_ref(props));

	GVariantIter iter;
	g_variant_iter_init(&iter, children);
	GVariant *child;
	while ((child = g_variant_iter_next_value(&iter)))
	{
		g_autoptr(GVariant) sub = g_variant_get_variant(child);
		index_layout(sub, index);
		g_variant_unref(child);
	}
}

/* Depth-first search for the (ia{sv}av) item with id @wanted inside the child
 * list @children. Returns a new reference, or NULL when the id is absent. */
static GVariant *find_subtree(GVariant *children, gint32 wanted)
{
	GVariantIter iter;
	g_variant_iter_init(&iter, children);
	GVariant *child;
	while ((child = g_variant_iter_next_value(&iter)))
	{
		g_autoptr(GVariant) item = g_variant_get_variant(child);
		gint32 id                = -1;
		g_variant_get(item, "(i@a{sv}@av)", &id, NULL, NULL);
		if (id == wanted)
			return g_variant_ref(item);

		g_autoptr(GVariant) sub = g_variant_get_child_value(item, 2);
		GVariant *found         = find_subtree(sub, wanted);
		g_variant_unref(child);
		if (found != NULL)
			return found;
	}
	return NULL;
}

/* Borrowed property dictionary of @id, or NULL when the layout has no such id. */
static GVariant *lookup_item(DBusMenuMockServer *self, gint id)
{
	if (id < 0)
		return NULL;
	return g_hash_table_lookup(self->layout_index, GINT_TO_POINTER(id));
}

/* --- com.canonical.dbusmenu implementation --------------------------------- */

static gboolean layout_menu_handle_get_layout(DBusMenuXml *object,
                                              GDBusMethodInvocation *invocation, gint parent_id,
                                              gint recursion_depth,
                                              const gchar *const *property_names,
                                              gpointer user_data)
{
	(void)object;
	(void)recursion_depth;
	(void)property_names;
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(user_data);

	self->revision = MAX(self->revision, 1);

	/* The importer may ask for a subtree instead of the root. Answer from the very
	 * same layout, never inventing an item that is not in it. */
	GVariant *item              = self->layout;
	g_autoptr(GVariant) subtree = NULL;
	if (parent_id != self->root_id)
	{
		g_autoptr(GVariant) children = g_variant_get_child_value(self->layout, 2);
		subtree                      = find_subtree(children, parent_id);
		if (subtree != NULL)
			item = subtree;
		else
			/* An id that is not in the layout gets the emptiest answer we can
			 * give: an item with no properties and no children. */
			item = g_variant_new_parsed("(%i, {}, @av [])", parent_id);
	}

	dbus_menu_xml_complete_get_layout(object, invocation, self->revision, item);
	return TRUE;
}

static gboolean layout_menu_handle_about_to_show(DBusMenuXml *object,
                                                 GDBusMethodInvocation *invocation, gint id,
                                                 gpointer user_data)
{
	(void)user_data;
	dbus_menu_xml_complete_about_to_show(object, invocation, FALSE);
	return TRUE;
}

static gboolean layout_menu_handle_about_to_show_group(DBusMenuXml *object,
                                                       GDBusMethodInvocation *invocation,
                                                       GVariant *ids, gpointer user_data)
{
	(void)user_data;
	GVariantBuilder updates, errors;
	g_variant_builder_init(&updates, G_VARIANT_TYPE("ai"));
	g_variant_builder_init(&errors, G_VARIANT_TYPE("ai"));

	GVariantIter iter;
	gint32 id;
	g_variant_iter_init(&iter, ids);
	while (g_variant_iter_next(&iter, "i", &id))
		g_variant_builder_add(&updates, "i", id);

	dbus_menu_xml_complete_about_to_show_group(object,
	                                           invocation,
	                                           g_variant_builder_end(&updates),
	                                           g_variant_builder_end(&errors));
	return TRUE;
}

static gboolean layout_menu_handle_event(DBusMenuXml *object, GDBusMethodInvocation *invocation,
                                         gint id, const gchar *event_id, GVariant *data,
                                         guint timestamp, gpointer user_data)
{
	(void)id;
	(void)event_id;
	(void)data;
	(void)timestamp;
	(void)user_data;
	dbus_menu_xml_complete_event(object, invocation);
	return TRUE;
}

static gboolean layout_menu_handle_event_group(DBusMenuXml *object,
                                               GDBusMethodInvocation *invocation, GVariant *events,
                                               gpointer user_data)
{
	(void)user_data;
	GVariantBuilder errors;
	g_variant_builder_init(&errors, G_VARIANT_TYPE("ai"));

	/* The generated complete_* helpers take the return values as GVariant, not
	 * as GVariantBuilder, so end the builders first. */
	dbus_menu_xml_complete_event_group(object, invocation, g_variant_builder_end(&errors));
	return TRUE;
}

static gboolean layout_menu_handle_get_property(DBusMenuXml *object,
                                                GDBusMethodInvocation *invocation, gint id,
                                                const gchar *property, gpointer user_data)
{
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(user_data);
	GVariant *props          = lookup_item(self, id);
	GVariant *value          = NULL;

	if (props != NULL)
		value = g_variant_lookup_value(props, property, NULL);

	if (value == NULL)
	{
		g_dbus_method_invocation_return_error(invocation,
		                                      G_DBUS_ERROR,
		                                      G_DBUS_ERROR_INVALID_ARGS,
		                                      "Item %d has no property '%s'",
		                                      id,
		                                      property);
		return TRUE;
	}

	/* The complete_ helper sinks the floating variant we hand it. */
	dbus_menu_xml_complete_get_property(object, invocation, value);
	return TRUE;
}

/* Filters the properties of one already-found item down to @requested (all of
 * them when @requested is empty), appending the resulting (ia{sv}) to @builder. */
static void append_filtered_props(PropFilter *filter, gint id, GVariant *props,
                                  const gchar *const *requested)
{
	g_variant_builder_open(filter->builder, G_VARIANT_TYPE("(ia{sv})"));
	g_variant_builder_add(filter->builder, "i", id);

	GVariantBuilder dict;
	g_variant_builder_init(&dict, G_VARIANT_TYPE("a{sv}"));

	GVariantIter iter;
	const char *key;
	GVariant *value;
	g_variant_iter_init(&iter, props);
	while (g_variant_iter_loop(&iter, "{&sv}", &key, &value))
	{
		if (*requested == NULL)
		{
			g_variant_builder_add(&dict, "{sv}", key, value);
			continue;
		}
		for (const gchar *const *name = requested; *name != NULL; name++)
		{
			if (g_str_equal(*name, key))
			{
				g_variant_builder_add(&dict, "{sv}", key, value);
				break;
			}
		}
	}

	g_variant_builder_add_value(filter->builder, g_variant_builder_end(&dict));
	g_variant_builder_close(filter->builder);
}

static gboolean layout_menu_handle_get_group_properties(DBusMenuXml *object,
                                                        GDBusMethodInvocation *invocation,
                                                        GVariant *ids,
                                                        const gchar *const *property_names,
                                                        gpointer user_data)
{
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(user_data);
	GVariantBuilder items;
	g_variant_builder_init(&items, G_VARIANT_TYPE("a(ia{sv})"));

	GVariantIter iter;
	gint32 id;
	g_variant_iter_init(&iter, ids);
	while (g_variant_iter_next(&iter, "i", &id))
	{
		GVariant *props = lookup_item(self, id);
		if (props == NULL)
			continue;
		PropFilter filter = { .builder = &items };
		append_filtered_props(&filter, id, props, property_names);
	}

	dbus_menu_xml_complete_get_group_properties(object,
	                                            invocation,
	                                            g_variant_builder_end(&items));
	return TRUE;
}

/* --- bus ownership --------------------------------------------------------- */

static void layout_menu_name_acquired(GDBusConnection *connection, const char *name,
                                      gpointer user_data)
{
	(void)name;
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(user_data);
	g_autoptr(GError) error  = NULL;

	/* The same defaults a real menu shell expects to read off the interface. */
	DBusMenuXml *skeleton                    = self->skeleton;
	static const char *const no_icon_theme[] = { NULL };
	g_object_set(skeleton,
	             "version",
	             3,
	             "status",
	             "normal",
	             "text-direction",
	             "ltr",
	             "icon-theme-path",
	             no_icon_theme,
	             NULL);

	if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton),
	                                      connection,
	                                      self->object_path,
	                                      &error))
	{
		g_warning("Unable to export %s at %s: %s",
		          name,
		          self->object_path,
		          error->message);
		return;
	}

	self->exported = TRUE;
}

static void layout_menu_name_lost(GDBusConnection *connection, const char *name, gpointer user_data)
{
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(user_data);

	if (self->exported)
	{
		if (connection != NULL)
			g_dbus_interface_skeleton_unexport_from_connection(
			    G_DBUS_INTERFACE_SKELETON(self->skeleton), connection);
		else
			g_dbus_interface_skeleton_unexport(
			    G_DBUS_INTERFACE_SKELETON(self->skeleton));
		self->exported = FALSE;
	}

	if (name != NULL)
		g_warning("Name %s is lost", name);
}

static void dbus_menu_mock_server_reindex(DBusMenuMockServer *self)
{
	g_variant_get(self->layout, "(i@a{sv}@av)", &self->root_id, NULL, NULL);
	g_hash_table_remove_all(self->layout_index);
	index_layout(self->layout, self->layout_index);
}

/* --- GObject plumbing ------------------------------------------------------ */

static void dbus_menu_mock_server_constructed(GObject *object)
{
	G_OBJECT_CLASS(dbus_menu_mock_server_parent_class)->constructed(object);

	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(object);

	self->skeleton = dbus_menu_xml_skeleton_new();
	g_signal_connect(self->skeleton,
	                 "handle-get-layout",
	                 G_CALLBACK(layout_menu_handle_get_layout),
	                 self);
	g_signal_connect(self->skeleton,
	                 "handle-about-to-show",
	                 G_CALLBACK(layout_menu_handle_about_to_show),
	                 self);
	g_signal_connect(self->skeleton,
	                 "handle-about-to-show-group",
	                 G_CALLBACK(layout_menu_handle_about_to_show_group),
	                 self);
	g_signal_connect(self->skeleton,
	                 "handle-event",
	                 G_CALLBACK(layout_menu_handle_event),
	                 self);
	g_signal_connect(self->skeleton,
	                 "handle-event-group",
	                 G_CALLBACK(layout_menu_handle_event_group),
	                 self);
	g_signal_connect(self->skeleton,
	                 "handle-get-property",
	                 G_CALLBACK(layout_menu_handle_get_property),
	                 self);
	g_signal_connect(self->skeleton,
	                 "handle-get-group-properties",
	                 G_CALLBACK(layout_menu_handle_get_group_properties),
	                 self);

	if (self->bus_name != NULL)
		self->name_id = g_bus_own_name(G_BUS_TYPE_SESSION,
		                                        self->bus_name,
		                                        G_BUS_NAME_OWNER_FLAGS_NONE,
		                                        NULL,
		                                        layout_menu_name_acquired,
		                                        layout_menu_name_lost,
		                                        self,
		                                        NULL);
}

static void dbus_menu_mock_server_dispose(GObject *object)
{
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(object);

	if (self->name_id > 0)
	{
		g_bus_unown_name(self->name_id);
		self->name_id = 0;
	}

	if (self->skeleton != NULL)
	{
		g_signal_handlers_disconnect_matched(self->skeleton,
		                                     G_SIGNAL_MATCH_DATA,
		                                     0,
		                                     0,
		                                     NULL,
		                                     NULL,
		                                     self);
		if (self->exported)
			g_dbus_interface_skeleton_unexport(
			    G_DBUS_INTERFACE_SKELETON(self->skeleton));
		self->exported = FALSE;
		g_clear_object(&self->skeleton);
	}

	G_OBJECT_CLASS(dbus_menu_mock_server_parent_class)->dispose(object);
}

static void dbus_menu_mock_server_finalize(GObject *object)
{
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(object);

	g_clear_pointer(&self->layout_index, g_hash_table_destroy);
	g_clear_pointer(&self->layout, g_variant_unref);
	g_clear_pointer(&self->bus_name, g_free);
	g_clear_pointer(&self->object_path, g_free);

	G_OBJECT_CLASS(dbus_menu_mock_server_parent_class)->finalize(object);
}

static void dbus_menu_mock_server_set_property(GObject *object, guint property_id,
                                               const GValue *value, GParamSpec *pspec)
{
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(object);

	switch (property_id)
	{
	case PROP_LAYOUT: {
		/* Keep our own reference: the value only lives for this call, and the
		 * variant may well be floating when it arrives. */
		g_autoptr(GVariant) old = self->layout;
		self->layout  = g_variant_ref_sink(g_value_get_variant(value));
		dbus_menu_mock_server_reindex(self);
		if(self->skeleton)
			dbus_menu_xml_emit_layout_updated(self->skeleton, ++self->revision, 0);
		break;
	}
	case PROP_BUS_NAME:
		g_free(self->bus_name);
		self->bus_name = g_value_dup_string(value);
		break;
	case PROP_OBJECT_PATH:
		g_free(self->object_path);
		self->object_path = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void dbus_menu_mock_server_get_property(GObject *object, guint property_id, GValue *value,
                                               GParamSpec *pspec)
{
	DBusMenuMockServer *self = DBUS_MENU_MOCK_SERVER(object);

	switch (property_id)
	{
	case PROP_LAYOUT:
		g_value_set_variant(value, self->layout);
		break;
	case PROP_BUS_NAME:
		g_value_set_string(value, self->bus_name);
		break;
	case PROP_OBJECT_PATH:
		g_value_set_string(value, self->object_path);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void dbus_menu_mock_server_class_init(DBusMenuMockServerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->constructed  = dbus_menu_mock_server_constructed;
	object_class->dispose      = dbus_menu_mock_server_dispose;
	object_class->finalize     = dbus_menu_mock_server_finalize;
	object_class->set_property = dbus_menu_mock_server_set_property;
	object_class->get_property = dbus_menu_mock_server_get_property;

	properties[PROP_LAYOUT] =
	    g_param_spec_variant("layout",
	                         "layout",
	                         "Menu layout answered to GetLayout, as (ia{sv}av)",
	                         G_VARIANT_TYPE("(ia{sv}av)"),
	                         NULL,
	                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
	                             G_PARAM_STATIC_STRINGS);

	properties[PROP_BUS_NAME] = g_param_spec_string("bus-name",
	                                                "bus name",
	                                                "Bus name owned to publish the menu on",
	                                                TEST_BUS_NAME,
	                                                G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
	                                                    G_PARAM_STATIC_STRINGS);

	properties[PROP_OBJECT_PATH] =
	    g_param_spec_string("object-path",
	                        "object path",
	                        "Path the menu is exported at",
	                        TEST_OBJECT_PATH,
	                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
	                            G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, LAST_PROP, properties);
}

static void dbus_menu_mock_server_init(DBusMenuMockServer *self)
{
	self->layout_index = g_hash_table_new_full(g_direct_hash,
	                                                    g_direct_equal,
	                                                    NULL,
	                                                    (GDestroyNotify)g_variant_unref);
	self->revision     = 1;
	self->root_id      = 0;
}

/* --- public API ------------------------------------------------------------ */

DBusMenuMockServer *dbus_menu_mock_server_new(GVariant *layout, const char *bus_name,
                                              const char *object_path)
{
	g_return_val_if_fail(layout != NULL, NULL);
	g_return_val_if_fail(g_variant_is_of_type(layout, G_VARIANT_TYPE("(ia{sv}av)")), NULL);

	/* The caller may hand us a floating ref; sinking it here means the value we
	 * pass to g_object_new is an ordinary one, so no extra unref is needed. */
	g_variant_ref_sink(layout);

	DBusMenuMockServer *self =
	    g_object_new(dbus_menu_mock_server_get_type(),
	                 "layout",
	                 layout,
	                 "bus-name",
	                 bus_name != NULL ? bus_name : TEST_BUS_NAME,
	                 "object-path",
	                 object_path != NULL ? object_path : TEST_OBJECT_PATH,
	                 NULL);

	/* Our set_property already took a reference for the instance, so drop the one
	 * this function acquired on the caller's behalf. */
	g_variant_unref(layout);
	return self;
}
