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

#include <stdbool.h>

#include "dbusmenu-interface.h"
#include "definitions.h"
#include "item.h"
#include "utils.h"

#define ITEM_MAGIC 0xDEADBEEF
#define item_set_magic(item) (item)->magic = GUINT_TO_POINTER(ITEM_MAGIC)

#define item_check_magic(item) (GPOINTER_TO_UINT((item)->magic) == ITEM_MAGIC)

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data);
G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_copy(DBusMenuItem *src);
G_DEFINE_BOXED_TYPE(DBusMenuItem, dbus_menu_item, dbus_menu_item_copy, dbus_menu_item_free)
#if 0
#include "item-pixbuf.c"
#endif

static void dbus_menu_item_try_to_update_action_properties(DBusMenuItem *item);

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new_first_section(u_int32_t id,
                                                               GActionGroup *action_group)
{
	DBusMenuItem *item = g_slice_new0(DBusMenuItem);
	item->id           = id;
	item->action_type  = DBUS_MENU_ACTION_SECTION;
	item->enabled      = false;
	item->toggled      = false;
	item->attributes =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);
	item->links =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_object_unref);
	item->referenced_action_group = action_group;
	item_set_magic(item);
	return item;
}

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new(u_int32_t id, DBusMenuModel *parent_model,
                                                 GVariant *props)
{
	DBusMenuItem *item = g_slice_new0(DBusMenuItem);
	DBusMenuXml *xml;
	GVariantIter iter;
	const char *prop;
	GVariant *value;
	item_set_magic(item);
	item->enabled = true;
	item->toggled = false;
	item->id      = id;
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
				item->action_type = DBUS_MENU_ACTION_SUBMENU;
				g_autofree char *name =
				    dbus_menu_action_get_name(id, item->action_type, true);
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_SUBMENU_ACTION),
				                    g_variant_new_string(name));
				action_creator_found = true;
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TOGGLE_TYPE) == 0)
		{
			g_autofree char *name =
			    dbus_menu_action_get_name(id, item->action_type, true);
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
				GVariant *vstr =
				    g_variant_new_string(DBUS_MENU_ACTION_RADIO_SELECTED);
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_TARGET),
				                    g_variant_ref_sink(vstr));
				action_creator_found = true;
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TYPE) == 0)
		{
			const char *type = g_variant_get_string(value, NULL);
			if (!g_strcmp0(type, DBUS_MENU_TYPE_SEPARATOR))
			{
				item->action_type    = DBUS_MENU_ACTION_SECTION;
				action_creator_found = true;
			}
			else if (!g_strcmp0(type, DBUS_MENU_TYPE_NORMAL))
			{
				item->action_type = DBUS_MENU_ACTION_NORMAL;
				g_autofree char *name =
				    dbus_menu_action_get_name(id, item->action_type, true);
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
				                    g_variant_new_string(name));
				action_creator_found = true;
			}
		}
		else if (g_strcmp0(prop, "x-kde-title") == 0)
		{
			item->action_type = DBUS_MENU_ACTION_SECTION;
			g_hash_table_insert(item->attributes,
			                    g_strdup(G_MENU_ATTRIBUTE_LABEL),
			                    value);
			action_creator_found = true;
		}
		else if (!action_creator_found)
		{
			item->action_type = DBUS_MENU_ACTION_NORMAL;
			g_autofree char *name =
			    dbus_menu_action_get_name(id, item->action_type, true);
			g_hash_table_insert(item->attributes,
			                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
			                    g_variant_new_string(name));
			action_creator_found = true;
		}
	}
	if (item->action_type != DBUS_MENU_ACTION_SECTION)
		g_hash_table_insert(item->attributes,
		                    g_strdup(G_MENU_ATTRIBUTE_LABEL),
		                    g_variant_new_string(""));
	dbus_menu_item_update_props(item, props);
	return item;
}

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data)
{
	DBusMenuItem *item = (DBusMenuItem *)data;
	if (item == NULL)
		return;
	item->magic = NULL;
	g_clear_pointer(&item->attributes, g_hash_table_destroy);
	g_clear_pointer(&item->links, g_hash_table_destroy);
	g_clear_object(&item->referenced_action);
	g_slice_free(DBusMenuItem, data);
}

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_copy(DBusMenuItem *src)
{
	DBusMenuItem *dst            = g_slice_new0(DBusMenuItem);
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

G_GNUC_INTERNAL bool dbus_menu_item_is_firefox_stub(DBusMenuItem *item)
{
	const char *hidden_when =
	    (const char *)g_hash_table_lookup(item->attributes, G_MENU_ATTRIBUTE_HIDDEN_WHEN);
	const char *action =
	    (const char *)g_hash_table_lookup(item->attributes, G_MENU_ATTRIBUTE_ACTION);
	const char *label =
	    (const char *)g_hash_table_lookup(item->attributes, G_MENU_ATTRIBUTE_LABEL);
	if (!g_strcmp0(hidden_when, G_MENU_HIDDEN_WHEN_ACTION_MISSING) &&
	    !g_strcmp0(action, DBUS_MENU_DISABLED_ACTION) && !g_strcmp0(label, "Label Empty"))
		return true;
	return false;
}

G_GNUC_INTERNAL void dbus_menu_item_preload(DBusMenuItem *item)
{
	if (!item_check_magic(item))
		return;
	if (item->action_type != DBUS_MENU_ACTION_SUBMENU)
		return;
	int id;
	DBusMenuXml *xml = NULL;
	bool need_update;
	DBusMenuModel *submenu = DBUS_MENU_MODEL(
	    g_hash_table_lookup(item->links,
	                        item->enabled ? G_MENU_LINK_SUBMENU : DBUS_MENU_DISABLED_SUBMENU));
	g_object_get(submenu, "parent-id", &id, "xml", &xml, NULL);
	if (!xml || !DBUS_MENU_IS_XML(xml))
		return;
	dbus_menu_xml_call_event_sync(xml,
	                              id,
	                              "opened",
	                              g_variant_new("v", g_variant_new_int32(0)),
	                              CURRENT_TIME,
	                              NULL,
	                              NULL);
	dbus_menu_xml_call_about_to_show_sync(xml, id, (gboolean *)&need_update, NULL, NULL);
	need_update = need_update || dbus_menu_model_is_layout_update_required(submenu);
	if (need_update)
	{
		if (DBUS_MENU_IS_MODEL(submenu))
			dbus_menu_model_update_layout(submenu);
	}
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
		is_updated = check_and_update_mutable_attribute(dst, key, value) || is_updated;
	}
	return is_updated;
}

G_GNUC_INTERNAL bool dbus_menu_item_update_enabled(DBusMenuItem *item, bool enabled)
{
	bool updated = false;
	if (item->action_type == DBUS_MENU_ACTION_SUBMENU && !item->toggled)
	{
		DBusMenuModel *submenu = DBUS_MENU_MODEL(
		    g_hash_table_lookup(item->links,
		                        item->enabled ? G_MENU_LINK_SUBMENU
		                                      : DBUS_MENU_DISABLED_SUBMENU));
		if (item->enabled != enabled)
		{
			if (submenu != NULL)
			{
				g_object_ref(submenu);
				g_hash_table_insert(item->links,
				                    g_strdup(enabled ? G_MENU_LINK_SUBMENU
				                                     : DBUS_MENU_DISABLED_SUBMENU),
				                    submenu);
				g_hash_table_remove(item->links,
				                    item->enabled ? G_MENU_LINK_SUBMENU
				                                  : DBUS_MENU_DISABLED_SUBMENU);
			}
			if (enabled)
			{
				g_hash_table_remove(item->attributes, G_MENU_ATTRIBUTE_ACTION);
			}
			else
			{
				g_hash_table_insert(item->attributes,
				                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
				                    g_variant_new_string(
				                        DBUS_MENU_DISABLED_ACTION));
			}
			updated = true;
		}
	}
	item->enabled = enabled;
	dbus_menu_item_try_to_update_action_properties(item);
	return updated;
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

static bool dbus_menu_item_update_shortcut(DBusMenuItem *item, GVariant *value)
{
	GString *new_accel_string = g_string_new(NULL);
	if (g_variant_n_children(value) != 1)
		g_debug("Unable to parse shortcut correctly, too many keys. Taking first.");

	GVariantIter iter;
	GVariant *child = g_variant_get_child_value(value, 0);
	g_variant_iter_init(&iter, child);
	char *string;

	while (g_variant_iter_loop(&iter, "s", &string))
	{
		if (g_strcmp0(string, DBUS_MENU_SHORTCUT_CONTROL) == 0)
			g_string_append(new_accel_string, DBUS_MENUMODEL_SHORTCUT_CONTROL);
		else if (g_strcmp0(string, DBUS_MENU_SHORTCUT_ALT) == 0)
			g_string_append(new_accel_string, DBUS_MENUMODEL_SHORTCUT_ALT);
		else if (g_strcmp0(string, DBUS_MENU_SHORTCUT_SHIFT) == 0)
			g_string_append(new_accel_string, DBUS_MENUMODEL_SHORTCUT_SHIFT);
		else if (g_strcmp0(string, DBUS_MENU_SHORTCUT_SUPER) == 0)
			g_string_append(new_accel_string, DBUS_MENUMODEL_SHORTCUT_SUPER);
		else
			g_string_append(new_accel_string, string);
	}
	g_variant_unref(child);
	g_autofree char *str = g_string_free(new_accel_string, false);
	GVariant *new_accel  = g_variant_new_string(str);
	bool updated = check_and_update_mutable_attribute(item, G_MENU_ATTRIBUTE_ACCEL, new_accel);
	if (!updated)
		g_variant_unref(new_accel);
	return updated;
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
		else if (g_strcmp0(prop, "enabled") == 0)
		{
			bool enabled = g_variant_get_boolean(value);
			properties_is_updated =
			    dbus_menu_item_update_enabled(item, enabled) || properties_is_updated;
		}
#if 0
		else if (g_strcmp0(prop, "icon-data") == 0)
		{
			// icon-name has more priority
			if (!g_hash_table_lookup(item->attributes, G_MENU_ATTRIBUTE_ICON))
			{
				g_autoptr(GIcon) icon = g_icon_new_pixbuf_from_variant(value);
				GVariant *value       = g_icon_serialize(icon);
				properties_is_updated =
				    properties_is_updated ||
				    check_and_update_mutable_attribute(item,
				                                       G_MENU_ATTRIBUTE_ICON,
				                                       value);
				properties_is_updated =
				    properties_is_updated ||
				    check_and_update_mutable_attribute(item,
				                                       G_MENU_ATTRIBUTE_VERB_ICON,
				                                       value);
			}
		}
		else if (g_strcmp0(prop, "icon-name") == 0)
		{
			g_autoptr(GIcon) icon =
			    g_themed_icon_new(g_variant_get_string(value, NULL));
			GVariant *value             = g_icon_serialize(icon);
			g_autoptr(GVariant) boolvar = g_variant_new_boolean(true);
			properties_is_updated =
			    properties_is_updated ||
			    check_and_update_mutable_attribute(item, G_MENU_ATTRIBUTE_ICON, value);
			properties_is_updated =
			    properties_is_updated ||
			    check_and_update_mutable_attribute(item,
			                                       G_MENU_ATTRIBUTE_VERB_ICON,
			                                       value);
			properties_is_updated =
			    properties_is_updated ||
			    check_and_update_mutable_attribute(item, HAS_ICON_NAME, boolvar);
		}
#endif
		else if (g_strcmp0(prop, "label") == 0)
		{
			properties_is_updated =
			    check_and_update_mutable_attribute(item,
			                                       G_MENU_ATTRIBUTE_LABEL,
			                                       value) ||
			    properties_is_updated;
		}
		else if (g_strcmp0(prop, "shortcut") == 0)
		{
			properties_is_updated =
			    dbus_menu_item_update_shortcut(item, value) || properties_is_updated;
		}
		else if (g_strcmp0(prop, "toggle-state") == 0)
		{
			item->toggled = g_variant_get_int32(value) > 0;
			dbus_menu_item_try_to_update_action_properties(item);
		}
		else if (g_strcmp0(prop, "visible") == 0)
		{
			bool vis = g_variant_get_boolean(value);
			if (item->action_type == DBUS_MENU_ACTION_SECTION)
			{
				item->toggled = !vis;
			}
			else if (vis)
			{
				g_autofree char *name =
				    dbus_menu_action_get_name(item->id, item->action_type, true);
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
	const char *prop;
	bool properties_is_updated = false;

	g_variant_iter_init(&iter, props);
	while (g_variant_iter_next(&iter, "&s", &prop))
	{
		if (g_strcmp0(prop, "accessible-desc") == 0)
		{
			// TODO: Can we support this property?
			// properties_is_updated = true;
		}
		else if (g_strcmp0(prop, "enabled") == 0)
		{
			bool enabled = true;
			dbus_menu_item_update_enabled(item, enabled);
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
	if (a->referenced_action_group != b->referenced_action_group)
		return false;
	if (a->action_type != b->action_type)
		return false;
	return true;
}

G_GNUC_INTERNAL void dbus_menu_item_copy_submenu(DBusMenuItem *src, DBusMenuItem *dst,
                                                 DBusMenuModel *parent)
{
	DBusMenuXml *xml;
	DBusMenuModel *submenu = NULL;
	g_object_get(parent, "xml", &xml, NULL);
	if (src == NULL)
	{
		if (dst->action_type == DBUS_MENU_ACTION_SUBMENU)
		{
			if (dst->toggled)
				dst->enabled = true;
			submenu =
			    dbus_menu_model_new(dst->id, parent, xml, dst->referenced_action_group);
			g_hash_table_insert(dst->links,
			                    g_strdup(dst->enabled ? G_MENU_LINK_SUBMENU
			                                          : DBUS_MENU_DISABLED_SUBMENU),
			                    submenu);
		}
		return;
	}
	if (dst->action_type == DBUS_MENU_ACTION_SUBMENU &&
	    src->action_type == DBUS_MENU_ACTION_SUBMENU)
	{
		if (src->toggled || dst->toggled)
			dst->enabled = dst->toggled = true;
		submenu =
		    DBUS_MENU_MODEL(g_hash_table_lookup(src->links,
		                                        src->enabled ? G_MENU_LINK_SUBMENU
		                                                     : DBUS_MENU_DISABLED_SUBMENU));
		g_hash_table_insert(dst->links,
		                    g_strdup(dst->enabled ? G_MENU_LINK_SUBMENU
		                                          : DBUS_MENU_DISABLED_SUBMENU),
		                    g_object_ref(submenu));
		g_object_set(submenu, "parent-id", dst->id, NULL);
	}
}

G_GNUC_INTERNAL void dbus_menu_item_generate_action(DBusMenuItem *item, DBusMenuModel *parent)
{
	if (item->action_type == DBUS_MENU_ACTION_SECTION)
		return;
	DBusMenuXml *xml;
	DBusMenuModel *submenu =
	    g_hash_table_lookup(item->links,
	                        item->enabled ? G_MENU_LINK_SUBMENU : DBUS_MENU_DISABLED_SUBMENU);
	g_object_get(parent, "xml", &xml, NULL);
	item->referenced_action =
	    dbus_menu_action_reference(item->id,
	                               xml,
	                               submenu,
	                               G_ACTION_MAP(item->referenced_action_group),
	                               item->action_type);
	dbus_menu_item_try_to_update_action_properties(item);
}
