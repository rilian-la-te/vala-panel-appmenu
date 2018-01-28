#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdbool.h>

#include "definitions.h"
#include "item.h"
#include "utils.h"

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data);
G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_copy(DBusMenuItem *src);
G_DEFINE_BOXED_TYPE(DBusMenuItem, dbus_menu_item, dbus_menu_item_copy, dbus_menu_item_free)

// static GIcon *g_icon_new_pixbuf_from_variant(GVariant *variant)
//{
//	gsize length;
//	const unsigned char *data =
//	    (const unsigned char *)g_variant_get_fixed_array(variant, &length, sizeof(guchar));
//	if (length == 0)
//		return NULL;

//	g_autoptr(GInputStream) stream = g_memory_input_stream_new_from_data(data, length, NULL);
//	if (stream == NULL)
//		return NULL;

//	g_autoptr(GError) error = NULL;
//	GdkPixbuf *pixbuf       = gdk_pixbuf_new_from_stream(stream, NULL, &error);
//	if (error != NULL)
//		g_warning("Unable to build GdkPixbuf from icon data: %s", error->message);

//	return G_ICON(pixbuf);
//}

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new_first_section(u_int32_t id,
                                                               GActionGroup *action_group)
{
	DBusMenuItem *item = g_slice_new0(DBusMenuItem);
	item->id           = id;
	item->is_section   = true;
	item->enabled      = false;
	item->toggled      = false;
	item->attributes =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);
	item->links =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_object_unref);
	item->referenced_action_group = action_group;
	return item;
}

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new(u_int32_t id, DBusMenuModel *parent_model,
                                                 GVariant *props)
{
	DBusMenuItem *item = g_slice_new0(DBusMenuItem);
	DBusMenuXml *xml;
	GVariantIter iter;
	const gchar *prop;
	GVariant *value;
	item->is_section = false;
	item->enabled    = false;
	item->toggled    = false;
	item->id         = id;
	item->attributes =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);
	item->links =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_object_unref);
	g_object_get(parent_model,
	             "action-group",
	             &item->referenced_action_group,
	             "xml",
	             &xml,
	             NULL);
	g_variant_iter_init(&iter, props);
	// Iterate by immutable properties, it is construct_only
	bool action_creator_found = false;
	while (g_variant_iter_loop(&iter, "{&sv}", &prop, &value))
	{
		if (g_strcmp0(prop, DBUS_MENU_PROP_CHILDREN_DISPLAY) == 0)
		{
			if (value == NULL)
			{
				g_hash_table_remove(item->attributes,
				                    G_MENU_ATTRIBUTE_SUBMENU_ACTION);
				continue;
			}
			else if (g_strcmp0(g_variant_get_string(value, NULL),
			                   DBUS_MENU_CHILDREN_DISPLAY_SUBMENU) == 0)
			{
				g_autofree char *name =
				    g_strdup_printf(DBUS_MENU_ACTION_NAMESPACE_PREFIX SUBMENU_PREFIX
				                    "%u",
				                    id);
				item->action_type = DBUS_MENU_ACTION_SUBMENU;
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_SUBMENU_ACTION),
				                    g_variant_new_string(name));
				action_creator_found = true;
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TOGGLE_TYPE) == 0)
		{
			g_autofree char *name =
			    g_strdup_printf(DBUS_MENU_ACTION_NAMESPACE_PREFIX ACTION_PREFIX "%u",
			                    id);
			if (g_strcmp0(g_variant_get_string(value, NULL),
			              DBUS_MENU_TOGGLE_TYPE_CHECK) == 0)
			{
				item->action_type = DBUS_MENU_ACTION_CHECKMARK;
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
				                    g_variant_new_string(name));
				action_creator_found = true;
			}
			else if (g_strcmp0(g_variant_get_string(value, NULL),
			                   DBUS_MENU_TOGGLE_TYPE_RADIO) == 0)
			{
				item->action_type = DBUS_MENU_ACTION_RADIO;
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
				                    g_variant_new_string(name));
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_TARGET),
				                    g_variant_new_string(
				                        DBUS_MENU_ACTION_RADIO_SELECTED));
				action_creator_found = true;
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TYPE) == 0)
		{
			const char *type = g_variant_get_string(value, NULL);
			if (!g_strcmp0(type, DBUS_MENU_TYPE_SEPARATOR))
			{
				item->is_section = true;
			}
			else if (!g_strcmp0(type, DBUS_MENU_TYPE_NORMAL))
			{
				g_autofree char *name =
				    g_strdup_printf(DBUS_MENU_ACTION_NAMESPACE_PREFIX ACTION_PREFIX
				                    "%u",
				                    id);
				item->action_type = DBUS_MENU_ACTION_NORMAL;
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
				                    g_variant_new_string(name));
				action_creator_found = true;
			}
		}
		else if (g_strcmp0(prop, "x-kde-title") == 0)
		{
			item->is_section = true;
			g_hash_table_insert(item->attributes,
			                    g_strdup(G_MENU_ATTRIBUTE_LABEL),
			                    value);
		}
		else if (!action_creator_found)
		{
			g_autofree char *name =
			    g_strdup_printf(DBUS_MENU_ACTION_NAMESPACE_PREFIX ACTION_PREFIX "%u",
			                    id);
			item->action_type = DBUS_MENU_ACTION_NORMAL;
			g_hash_table_insert(item->attributes,
			                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
			                    g_variant_new_string(name));
			action_creator_found = true;
		}
	}
	dbus_menu_item_update_props(item, props);
	return item;
}

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data)
{
	DBusMenuItem *item = (DBusMenuItem *)data;
	g_print("Item %u is destroyed\n", item->id);
	if (item == NULL)
		return;
	g_clear_pointer(&item->attributes, g_hash_table_destroy);
	g_clear_pointer(&item->links, g_hash_table_destroy);
	g_clear_object(&item->referenced_action);
	g_slice_free(DBusMenuItem, data);
}

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_copy(DBusMenuItem *src)
{
	DBusMenuItem *dst            = g_slice_new0(DBusMenuItem);
	dst->is_section              = src->is_section;
	dst->id                      = src->id;
	dst->action_type             = src->action_type;
	dst->enabled                 = src->enabled;
	dst->toggled                 = src->toggled;
	dst->referenced_action       = G_ACTION(g_object_ref(src->referenced_action));
	dst->referenced_action_group = src->referenced_action_group;
	dst->attributes              = g_hash_table_ref(src->attributes);
	dst->links                   = g_hash_table_ref(src->links);
	return dst;
}

static bool check_and_update_mutable_attribute(DBusMenuItem *item, const char *key, GVariant *value)
{
	GVariant *old  = (GVariant *)g_hash_table_lookup(item->attributes, key);
	bool are_equal = false;
	if (old != NULL)
		are_equal = g_variant_equal(old, value);
	if (!are_equal)
	{
		g_hash_table_insert(item->attributes, g_strdup(key), g_variant_ref_sink(value));
		return true;
	}
	return false;
}

G_GNUC_INTERNAL bool dbus_menu_item_copy_attributes(DBusMenuItem *src, DBusMenuItem *dst)
{
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, src->attributes);
	bool is_updated = false;
	char *key;
	GVariant *value;
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		is_updated = is_updated || check_and_update_mutable_attribute(dst, key, value);
	}
	return is_updated;
}

static void dbus_menu_item_try_to_update_action_properties(DBusMenuItem *item)
{
	if (!G_IS_ACTION(item->referenced_action))
		return;
	g_simple_action_set_enabled(G_SIMPLE_ACTION(item->referenced_action), item->enabled);
	if (item->action_type == DBUS_MENU_ACTION_RADIO)
	{
		dbus_menu_action_lock(item->referenced_action);
		g_action_change_state((item->referenced_action),
		                      g_variant_new_string(
		                          item->toggled ? DBUS_MENU_ACTION_RADIO_SELECTED
		                                        : DBUS_MENU_ACTION_RADIO_UNSELECTED));
		dbus_menu_action_unlock(item->referenced_action);
	}
	else if (item->action_type == DBUS_MENU_ACTION_CHECKMARK)
	{
		dbus_menu_action_lock(item->referenced_action);
		g_action_change_state((item->referenced_action),
		                      g_variant_new_boolean(item->toggled));
		dbus_menu_action_unlock(item->referenced_action);
	}
}

G_GNUC_INTERNAL bool dbus_menu_item_update_props(DBusMenuItem *item, GVariant *props)
{
	GVariantIter iter;
	const char *prop;
	GVariant *value;
	bool properties_is_updated = false;

	g_variant_iter_init(&iter, props);
	while (g_variant_iter_loop(&iter, "{&sv}", &prop, &value))
	{
		if (g_strcmp0(prop, "accessible-desc") == 0)
		{
			// TODO: Can we supported this property?
			// properties_is_updated = true;
		}
		if (g_strcmp0(prop, "enabled") == 0)
		{
			item->enabled = g_variant_get_boolean(value);
			dbus_menu_item_try_to_update_action_properties(item);
		}
		//		else if (g_strcmp0(prop, "icon-data") == 0)
		//		{
		//			// icon-name has more priority
		//			if (!g_hash_table_lookup(item->attributes,
		// G_MENU_ATTRIBUTE_ICON))
		//			{
		//				g_autoptr(GIcon) icon =
		// g_icon_new_pixbuf_from_variant(value);
		//				GVariant *value       = g_icon_serialize(icon);
		//				properties_is_updated =
		//				    properties_is_updated ||
		//				    check_and_update_mutable_attribute(item,
		//				                                       G_MENU_ATTRIBUTE_ICON,
		//				                                       value);
		//				properties_is_updated =
		//				    properties_is_updated ||
		//				    check_and_update_mutable_attribute(item,
		//				                                       G_MENU_ATTRIBUTE_VERB_ICON,
		//				                                       value);
		//			}
		//		}
		//		else if (g_strcmp0(prop, "icon-name") == 0)
		//		{
		//			g_autoptr(GIcon) icon =
		//			    g_themed_icon_new(g_variant_get_string(value, NULL));
		//			GVariant *value             = g_icon_serialize(icon);
		//			g_autoptr(GVariant) boolvar = g_variant_new_boolean(true);
		//			properties_is_updated =
		//			    properties_is_updated ||
		//			    check_and_update_mutable_attribute(item,
		// G_MENU_ATTRIBUTE_ICON, value); 			properties_is_updated =
		//			    properties_is_updated ||
		//			    check_and_update_mutable_attribute(item,
		//			                                       G_MENU_ATTRIBUTE_VERB_ICON,
		//			                                       value);
		//			properties_is_updated =
		//			    properties_is_updated ||
		//			    check_and_update_mutable_attribute(item, HAS_ICON_NAME,
		// boolvar);
		//		}
		else if (g_strcmp0(prop, "label") == 0)
		{
			properties_is_updated =
			    properties_is_updated ||
			    check_and_update_mutable_attribute(item, G_MENU_ATTRIBUTE_LABEL, value);
		}
		else if (g_strcmp0(prop, "shortcut") == 0)
		{
			// TODO: Shortcut translator to Gtk without Gtk linked
			properties_is_updated =
			    properties_is_updated ||
			    check_and_update_mutable_attribute(item, G_MENU_ATTRIBUTE_ACCEL, value);
		}
		else if (g_strcmp0(prop, "toggle-state") == 0)
		{
			item->toggled = g_variant_get_int32(value) > 0;
			dbus_menu_item_try_to_update_action_properties(item);
		}
		else if (g_strcmp0(prop, "visible") == 0)
		{
			bool vis = g_variant_get_boolean(value);
			if (vis)
			{
				g_autofree char *name =
				    dbus_menu_action_get_name(item->id, item->action_type, false);
				bool found = g_hash_table_remove(item->attributes,
				                                 G_MENU_ATTRIBUTE_HIDDEN_WHEN);
				if (found)
				{
					g_hash_table_insert(item->attributes,
					                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
					                    g_variant_new_string(name));
					properties_is_updated = true;
				}
			}
			else
			{
				bool found = g_hash_table_contains(item->attributes,
				                                   G_MENU_ATTRIBUTE_HIDDEN_WHEN);
				if (!found)
				{
					g_hash_table_insert(item->attributes,
					                    g_strdup(G_MENU_ATTRIBUTE_HIDDEN_WHEN),
					                    g_variant_new_string(
					                        G_MENU_HIDDEN_WHEN_ACTION_MISSING));
					g_hash_table_insert(item->attributes,
					                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
					                    g_variant_new_string(
					                        DBUS_MENU_DISABLED_ACTION));
					properties_is_updated = true;
				}
			}
		}
		else
		{
			g_debug("updating unsupported property - '%s'", prop);
		}
	}
	return properties_is_updated;
}

G_GNUC_INTERNAL bool dbus_menu_item_remove_props(DBusMenuItem *item, GVariant *props)
{
	GVariantIter iter;
	const gchar *prop;
	bool properties_is_updated = false;

	g_variant_iter_init(&iter, props);
	while (g_variant_iter_next(&iter, "&s", &prop))
	{
		if (g_strcmp0(prop, "accessible-desc") == 0)
		{
			// TODO: Can we supported this property?
			// properties_is_updated = true;
		}
		else if (g_strcmp0(prop, "enabled") == 0)
		{
			item->enabled = true;
			dbus_menu_item_try_to_update_action_properties(item);
		}
		else if (g_strcmp0(prop, "icon-name") == 0)
		{
			if (g_hash_table_lookup(item->attributes, HAS_ICON_NAME))
			{
				g_hash_table_remove(item->attributes, G_MENU_ATTRIBUTE_ICON);
				g_hash_table_remove(item->attributes, G_MENU_ATTRIBUTE_VERB_ICON);
				g_hash_table_remove(item->attributes, HAS_ICON_NAME);
				properties_is_updated = true;
			}
		}
		else if (g_strcmp0(prop, "icon-data") == 0)
		{
			if (!g_hash_table_lookup(item->attributes, HAS_ICON_NAME))
			{
				g_hash_table_remove(item->attributes, G_MENU_ATTRIBUTE_ICON);
				g_hash_table_remove(item->attributes, G_MENU_ATTRIBUTE_VERB_ICON);
				properties_is_updated = true;
			}
		}
		else if (g_strcmp0(prop, "label") == 0)
		{
			g_hash_table_remove(item->attributes, G_MENU_ATTRIBUTE_LABEL);
			properties_is_updated = true;
		}
		else if (g_strcmp0(prop, "shortcut") == 0)
		{
			g_hash_table_remove(item->attributes, G_MENU_ATTRIBUTE_ACCEL);
			properties_is_updated = true;
		}
		else if (g_strcmp0(prop, "visible") == 0)
		{
			g_autofree char *name =
			    dbus_menu_action_get_name(item->id, item->action_type, false);
			g_hash_table_remove(item->attributes, G_MENU_ATTRIBUTE_HIDDEN_WHEN);
			g_hash_table_insert(item->attributes,
			                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
			                    g_variant_new_string(name));
			properties_is_updated = true;
		}
		else
		{
			g_debug("removing unsupported property - '%s'", prop);
		}
	}
	return properties_is_updated;
}

G_GNUC_INTERNAL int dbus_menu_item_compare_func(const DBusMenuItem *a, const DBusMenuItem *b,
                                                gpointer user_data)
{
	return b->id - a->id;
}

G_GNUC_INTERNAL int dbus_menu_item_id_compare_func(const DBusMenuItem *a, gconstpointer b,
                                                   gpointer user_data)
{
	return GPOINTER_TO_UINT(b) - a->id;
}

G_GNUC_INTERNAL bool dbus_menu_item_compare_immutable(DBusMenuItem *a, DBusMenuItem *b)
{
	if (a->id != b->id)
		return false;
	if (a->is_section != b->is_section)
		return false;
	if (a->referenced_action_group != b->referenced_action_group)
		return false;
	if (a->action_type != b->action_type)
		return false;
	return true;
}

G_GNUC_INTERNAL void dbus_menu_item_generate_action(DBusMenuItem *item, DBusMenuModel *parent)
{
	DBusMenuXml *xml;
	DBusMenuModel *submenu = NULL;
	g_object_get(parent, "xml", &xml, NULL);
	if (item->action_type == DBUS_MENU_ACTION_SUBMENU)
	{
		submenu = dbus_menu_model_new(item->id, parent, xml, item->referenced_action_group);
		g_hash_table_insert(item->links, g_strdup(G_MENU_LINK_SUBMENU), submenu);
	}
	item->referenced_action =
	    dbus_menu_action_reference(item->id,
	                               xml,
	                               submenu,
	                               G_ACTION_MAP(item->referenced_action_group),
	                               item->action_type);
	dbus_menu_item_try_to_update_action_properties(item);
}
