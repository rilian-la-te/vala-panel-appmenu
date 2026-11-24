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
#define item_set_magic(item) ((item)->magic = ITEM_MAGIC)
#define item_check_magic(item) ((item)->magic == ITEM_MAGIC)

#define submenu_str(en) ((en) ? G_MENU_LINK_SUBMENU : DBUS_MENU_DISABLED_SUBMENU)

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data);
#if 0
#include "item-pixbuf.c"
#endif

static void act_props_try_update(DBusMenuItem *item);
static void dbus_menu_item_generate_action(DBusMenuItem *item, DBusMenuModel *parent);

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new(u_int32_t id, GActionGroup *action_group)
{
	DBusMenuItem *item = g_slice_new0(DBusMenuItem);
	item->id           = id;
	item->action_type  = DBUS_MENU_ACTION_NONE;
	item->enabled      = true;
	item->toggled      = false;
	item->attrs =
	    g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);
	item->links = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);
	item->ref_action_group = g_object_ref(action_group);
	item_set_magic(item);
	return item;
}

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new_first_section(u_int32_t id,
                                                               GActionGroup *action_group)
{
	DBusMenuItem *item = dbus_menu_item_new(id, action_group);
	item->action_type  = DBUS_MENU_ACTION_SECTION;
	item->section_num = 0;
	item->place = -1;
	return item;
}

static inline DBusMenuActionType dbus_menu_item_update_action_type(DBusMenuActionType current,
                                                                   DBusMenuActionType requested)
{
	return current > requested ? current : requested;
}

G_GNUC_INTERNAL bool dbus_menu_item_is_submenu(DBusMenuItem *item)
{
	if (!item)
		return false;
	if (!item_check_magic(item))
		return false;
	if (item->action_type != DBUS_MENU_ACTION_SUBMENU)
		return false;
	return true;
}

G_GNUC_INTERNAL DBusMenuActionType dbus_menu_item_find_action_type(GVariant *props)
{
	GVariantIter iter;
	const char *prop;
	GVariant *value;
	DBusMenuActionType action_type = DBUS_MENU_ACTION_NORMAL;
	g_variant_iter_init(&iter, props);
	// We evaluate only properties, which affects action type - sorry for second pass of variant
	while (g_variant_iter_next(&iter, "{&sv}", &prop, &value))
	{
		g_autoptr(GVariant) raii = value;
		if (g_strcmp0(prop, DBUS_MENU_PROP_CHILDREN_DISPLAY) == 0)
		{
			if (g_str_equal(g_variant_get_string(value, NULL), DBUS_MENU_CHILDREN_DISPLAY_SUBMENU))
				action_type =
				    dbus_menu_item_update_action_type(action_type,
				                                      DBUS_MENU_ACTION_SUBMENU);
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TOGGLE_TYPE) == 0)
		{
			const char *val = g_variant_get_string(value, NULL);
			if (g_str_equal(val, DBUS_MENU_TOGGLE_TYPE_CHECK))
				action_type =
				    dbus_menu_item_update_action_type(action_type,
				                                      DBUS_MENU_ACTION_CHECKMARK);
			if (g_str_equal(val, DBUS_MENU_TOGGLE_TYPE_RADIO))
				action_type =
				    dbus_menu_item_update_action_type(action_type,
				                                      DBUS_MENU_ACTION_RADIO);
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TYPE) == 0)
		{
			const char *type = g_variant_get_string(value, NULL);
			if (g_str_equal(type, DBUS_MENU_TYPE_SEPARATOR))
				return DBUS_MENU_ACTION_SECTION;
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_KDE_TITLE) == 0)
		{
			gboolean section = g_variant_get_boolean(value);
			if (section)
				return DBUS_MENU_ACTION_SECTION;
		}
	}
	return action_type;
}

G_GNUC_INTERNAL void dbus_menu_item_init_ordinary(DBusMenuModel *parent, DBusMenuItem *item)
{
	g_autofree char* name = dbus_menu_action_name(parent, item, false, true);
	g_hash_table_insert(item->attrs, g_strdup(G_MENU_ATTRIBUTE_HIDDEN_WHEN), g_variant_new_string(G_MENU_HIDDEN_WHEN_ACTION_MISSING));
	g_hash_table_insert(item->attrs, g_strdup(G_MENU_ATTRIBUTE_ACTION), g_variant_new_string(name));
}


G_GNUC_INTERNAL bool dbus_menu_set_submenu_status(DBusMenuItem* item, DBusMenuModel *parent)
{
	g_autoptr(DBusMenuXml) xml = NULL;
	DBusMenuModel *submenu = NULL;
	g_object_get(parent, "xml", &xml, NULL);
	if (dbus_menu_item_is_submenu(item) && !dbus_menu_item_get_submenu(item))
	{
		if (item->toggled)
			item->enabled = true;
		g_autofree char* name = dbus_menu_action_name(parent,item, true, true);
		g_hash_table_insert(item->attrs, G_MENU_ATTRIBUTE_SUBMENU_ACTION, g_variant_new_string(name));
		submenu = dbus_menu_model_new(item->id,
										parent,
										xml,
										item->ref_action_group);
		g_hash_table_insert(item->links, submenu_str(item->enabled), submenu);
		if(!item->ref_submenu)
			item->ref_submenu = dbus_menu_submenu_action_new(item,submenu);
		g_action_map_add_action(G_ACTION_MAP(item->ref_action_group), item->ref_submenu);
		return true;
	} else if (dbus_menu_item_get_submenu(item) && !dbus_menu_item_is_submenu(item)) {
		g_hash_table_remove(item->links, submenu_str(item->enabled));
		g_action_map_remove_action(G_ACTION_MAP(item->ref_action_group), g_action_get_name(item->ref_submenu));
	}
	return false;
}

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_init(DBusMenuItem *item, DBusMenuModel *parent_model,
                                                 GVariant *props)
{
	GVariantIter iter;
	const char *prop;
	GVariant *value;
	dbus_menu_item_generate_action(item, parent_model);
	g_variant_iter_init(&iter, props);
	// Iterate by immutable properties, it is construct_only
	while (g_variant_iter_loop(&iter, "{&sv}", &prop, &value))
	{
		if (g_strcmp0(prop, DBUS_MENU_PROP_CHILDREN_DISPLAY) == 0)
		{
			if (value == NULL)
			{
				g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_SUBMENU_ACTION);
				continue;
			}
			else if (g_strcmp0(g_variant_get_string(value, NULL),
			                   DBUS_MENU_CHILDREN_DISPLAY_SUBMENU) == 0)
			{
				item->action_type =
				    dbus_menu_item_update_action_type(item->action_type,
				                                      DBUS_MENU_ACTION_SUBMENU);
				g_autofree char *name =
				    dbus_menu_action_name(parent_model, item, true, true);
				g_hash_table_insert(item->attrs,
				                    g_strdup(G_MENU_ATTRIBUTE_SUBMENU_ACTION),
				                    g_variant_ref_sink(g_variant_new_string(name)));
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TOGGLE_TYPE) == 0)
		{
			g_autofree char *name =
			    dbus_menu_action_name(parent_model, item, false, true);
			if (g_strcmp0(g_variant_get_string(value, NULL),
			              DBUS_MENU_TOGGLE_TYPE_CHECK) == 0)
			{
				item->action_type =
				    dbus_menu_item_update_action_type(item->action_type,
				                                      DBUS_MENU_ACTION_CHECKMARK);
				g_hash_table_insert(item->attrs,
				                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
				                    g_variant_ref_sink(g_variant_new_string(name)));
			}
			else if (g_strcmp0(g_variant_get_string(value, NULL),
			                   DBUS_MENU_TOGGLE_TYPE_RADIO) == 0)
			{
				item->action_type =
				    dbus_menu_item_update_action_type(item->action_type,
				                                      DBUS_MENU_ACTION_RADIO);
				g_hash_table_insert(item->attrs,
				                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
				                    g_variant_ref_sink(g_variant_new_string(name)));
				GVariant *vstr =
				    g_variant_new_string(DBUS_MENU_ACTION_RADIO_SELECTED);
				g_hash_table_insert(item->attrs,
				                    g_strdup(G_MENU_ATTRIBUTE_TARGET),
				                    g_variant_ref_sink(vstr));
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TYPE) == 0)
		{
			const char *type = g_variant_get_string(value, NULL);
			if (!g_strcmp0(type, DBUS_MENU_TYPE_SEPARATOR))
			{
				item->action_type =
				    dbus_menu_item_update_action_type(item->action_type,
				                                      DBUS_MENU_ACTION_SECTION);
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_KDE_TITLE) == 0)
		{
			item->action_type =
			    dbus_menu_item_update_action_type(item->action_type,
			                                      DBUS_MENU_ACTION_SECTION);
			g_hash_table_insert(item->attrs,
			                    g_strdup(G_MENU_ATTRIBUTE_LABEL),
			                    g_variant_ref_sink(value));
		}
	}
	if (item->action_type < DBUS_MENU_ACTION_CHECKMARK)
	{
		item->action_type     = DBUS_MENU_ACTION_NORMAL;
		g_autofree char *name = dbus_menu_action_name(parent_model, item, false, true);
		g_hash_table_insert(item->attrs,
		                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
		                    g_variant_ref_sink(g_variant_new_string(name)));
	}
	if (item->action_type != DBUS_MENU_ACTION_SECTION)
		g_hash_table_insert(item->attrs,
		                    g_strdup(G_MENU_ATTRIBUTE_LABEL),
		                    g_variant_ref_sink(g_variant_new_string("")));
	dbus_menu_item_update_props(item, props);
	return item;
}

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data)
{
	DBusMenuItem *item = (DBusMenuItem *)data;
	if (item == NULL)
		return;
	item->magic = 0;
	g_clear_pointer(&item->attrs, g_hash_table_destroy);
	g_clear_pointer(&item->links, g_hash_table_destroy);
	g_clear_object(&item->ref_action);
	g_clear_object(&item->ref_submenu);
	g_clear_object(&item->ref_action_group);
	g_source_remove_by_user_data(item);
	g_slice_free(DBusMenuItem, data);
}

static bool attr_update_checked(DBusMenuItem *item, const char *key, GVariant *value)
{
	GVariant *old  = (GVariant *)g_hash_table_lookup(item->attrs, key);
	bool are_equal = false;
	if (old != NULL)
		are_equal = g_variant_equal(old, value);
	if (!are_equal)
	{
		g_hash_table_insert(item->attrs, g_strdup(key), g_variant_ref_sink(value));
		return true;
	}
	return false;
}

G_GNUC_INTERNAL bool dbus_menu_item_is_firefox_stub(DBusMenuItem *item)
{
    GVariant *hidden_when_v = g_hash_table_lookup(item->attrs, G_MENU_ATTRIBUTE_HIDDEN_WHEN);
    GVariant *action_v = g_hash_table_lookup(item->attrs, G_MENU_ATTRIBUTE_ACTION);
    GVariant *label_v = g_hash_table_lookup(item->attrs, G_MENU_ATTRIBUTE_LABEL);
    const char *hidden_when = hidden_when_v ? g_variant_get_string(hidden_when_v, NULL) : NULL;
    const char *action = action_v ? g_variant_get_string(action_v, NULL) : NULL;
    const char *label = label_v ? g_variant_get_string(label_v, NULL) : NULL;
	if (!g_strcmp0(hidden_when, G_MENU_HIDDEN_WHEN_ACTION_MISSING) &&
	    !g_strcmp0(action, DBUS_MENU_DISABLED_ACTION) && !g_strcmp0(label, "Label Empty"))
		return true;
	return false;
}

G_GNUC_INTERNAL DBusMenuModel *dbus_menu_item_get_submenu(DBusMenuItem *item)
{
	if (!dbus_menu_item_is_submenu(item))
		return NULL;
	DBusMenuModel *submenu =
	    DBUS_MENU_MODEL(g_hash_table_lookup(item->links, submenu_str(item->enabled)));
	if (!submenu || !DBUS_MENU_IS_MODEL(submenu))
		return NULL;
	return submenu;
}

G_GNUC_INTERNAL void dbus_menu_item_preload(DBusMenuItem *item)
{
	DBusMenuModel *submenu = dbus_menu_item_get_submenu(item);
	if (!submenu)
		return;
	dbus_menu_model_load(submenu, NULL);
}

G_GNUC_INTERNAL bool dbus_menu_item_update_enabled(DBusMenuItem *item, bool enabled)
{
	bool updated = false;
	if (dbus_menu_item_is_submenu(item) && !item->toggled)
	{
		DBusMenuModel *submenu =
		    DBUS_MENU_MODEL(g_hash_table_lookup(item->links, submenu_str(item->enabled)));
		if (item->enabled != enabled)
		{
			if (submenu != NULL)
			{
				g_object_ref(submenu);
				g_hash_table_remove(item->links, submenu_str(item->enabled));
				g_hash_table_insert(item->links, submenu_str(enabled), submenu);
			}
			if (enabled)
			{
				g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_ACTION);
			}
			else
			{
				g_hash_table_insert(item->attrs,
				                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
				                    g_variant_ref_sink(g_variant_new_string(
				                        DBUS_MENU_DISABLED_ACTION)));
			}
			updated = true;
		}
	}
	item->enabled = enabled;
	act_props_try_update(item);
	return updated;
}

static void act_props_try_update(DBusMenuItem *item)
{
	if (!G_IS_ACTION(item->ref_action))
		return;
	g_simple_action_set_enabled(G_SIMPLE_ACTION(item->ref_action), item->enabled);
	if (item->action_type == DBUS_MENU_ACTION_RADIO)
	{
		dbus_menu_action_lock(item->ref_action);
		g_action_change_state((item->ref_action),
		                      g_variant_new_string(
		                          item->toggled ? DBUS_MENU_ACTION_RADIO_SELECTED
		                                        : DBUS_MENU_ACTION_RADIO_UNSELECTED));
		dbus_menu_action_unlock(item->ref_action);
	}
	else if (item->action_type == DBUS_MENU_ACTION_CHECKMARK)
	{
		dbus_menu_action_lock(item->ref_action);
		g_action_change_state((item->ref_action), g_variant_new_boolean(item->toggled));
		dbus_menu_action_unlock(item->ref_action);
	}
}

static bool dbus_menu_item_update_shortcut(DBusMenuItem *item, GVariant *value)
{
	if (g_variant_n_children(value) == 0)
        return false;
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
	GVariant *new_accel  = g_variant_ref_sink(g_variant_new_string(str));
	bool updated         = attr_update_checked(item, G_MENU_ATTRIBUTE_ACCEL, new_accel);
	g_variant_unref(new_accel);
	return updated;
}

G_GNUC_INTERNAL DBusMenuUpdaterFlags dbus_menu_item_update_props(DBusMenuItem *item,
                                                                 GVariant *props)
{
	GVariantIter iter;
	const char *prop;
	GVariant *value;
	bool properties_is_updated = false;
	bool immutable             = false;

	g_variant_iter_init(&iter, props);
	while (g_variant_iter_loop(&iter, "{&sv}", &prop, &value))
	{
		if (g_strcmp0(prop, DBUS_MENU_PROP_ENABLED) == 0)
		{
			bool enabled = g_variant_get_boolean(value);
			properties_is_updated |= dbus_menu_item_update_enabled(item, enabled);
		}
#if 0
		else if (g_strcmp0(prop, DBUS_MENU_PROP_ICON_DATA) == 0)
		{
			// icon-name has more priority
            if (!g_hash_table_lookup(item->attrs, G_MENU_ATTRIBUTE_ICON))
			{
				g_autoptr(GIcon) icon = g_icon_new_pixbuf_from_variant(value);
				GVariant *value       = g_icon_serialize(icon);
				properties_is_updated |=
                    attr_update_checked(item,
				                                       G_MENU_ATTRIBUTE_ICON,
				                                       value);
				properties_is_updated |=
                    attr_update_checked(item,
				                                       G_MENU_ATTRIBUTE_VERB_ICON,
				                                       value);
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_ICON_NAME) == 0)
		{
			g_autoptr(GIcon) icon =
			    g_themed_icon_new(g_variant_get_string(value, NULL));
			GVariant *value             = g_icon_serialize(icon);
			g_autoptr(GVariant) boolvar = g_variant_new_boolean(true);
			properties_is_updated |=
                attr_update_checked(item, G_MENU_ATTRIBUTE_ICON, value);
			properties_is_updated |=
                attr_update_checked(item,
			                                       G_MENU_ATTRIBUTE_VERB_ICON,
			                                       value);
			properties_is_updated |=
                attr_update_checked(item, HAS_ICON_NAME, boolvar);
		}
#endif
		else if (g_strcmp0(prop, DBUS_MENU_PROP_LABEL) == 0)
		{
			properties_is_updated |=
			    attr_update_checked(item, G_MENU_ATTRIBUTE_LABEL, value);
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_SHORTCUT) == 0)
		{
			properties_is_updated |=
			    dbus_menu_item_update_shortcut(item, value);
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TOGGLE_STATE) == 0)
		{
			item->toggled = g_variant_get_int32(value) > 0;
			act_props_try_update(item);
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_VISIBLE) == 0)
		{
			bool old_vis = item->visible;
			bool vis = g_variant_get_boolean(value);
			item->visible = vis;
			if (item->action_type == DBUS_MENU_ACTION_SECTION)
			{
				item->toggled = !vis;
			}
			else if (vis && !old_vis)
			{
				g_autofree char *name =
				    dbus_menu_item_get_action_name(item, false, true);

				bool found =
				    g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_HIDDEN_WHEN);
				if (found)
				{
					g_hash_table_insert(item->attrs,
					                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
					                    g_variant_ref_sink(g_variant_new_string(name)));
					properties_is_updated = true;
				}
			}
			else if(!vis && old_vis)
			{
				bool found = g_hash_table_contains(item->attrs,
				                                   G_MENU_ATTRIBUTE_HIDDEN_WHEN);
				if (!found)
				{
					g_hash_table_insert(item->attrs,
					                    g_strdup(G_MENU_ATTRIBUTE_HIDDEN_WHEN),
					                    g_variant_ref_sink(g_variant_new_string(
					                        G_MENU_HIDDEN_WHEN_ACTION_MISSING)));
					g_hash_table_insert(item->attrs,
					                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
					                    g_variant_ref_sink(g_variant_new_string(
					                        DBUS_MENU_DISABLED_ACTION)));
					properties_is_updated = true;
				}
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_TYPE) == 0)
		{
			bool is_section = g_str_equal(g_variant_get_string(value, NULL),
			                              DBUS_MENU_TYPE_SEPARATOR);
			immutable |= ((is_section && item->action_type != DBUS_MENU_ACTION_SECTION) ||
			            (!is_section && item->action_type == DBUS_MENU_ACTION_SECTION));
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_KDE_TITLE) == 0)
		{
			bool is_section = g_variant_get_boolean(value);
			immutable |= ((is_section && item->action_type != DBUS_MENU_ACTION_SECTION) ||
			            (!is_section && item->action_type == DBUS_MENU_ACTION_SECTION));
		}
		else if ((g_strcmp0(prop, DBUS_MENU_PROP_TOGGLE_TYPE) == 0) ||
		         (g_strcmp0(prop, DBUS_MENU_PROP_CHILDREN_DISPLAY) == 0))
		{
			immutable = true;
		}
		else
		{
			g_debug("updating unsupported property - '%s'", prop);
		}
	}
	return (properties_is_updated ? DBUS_MENU_UPDATER_MUTABLE : 0) | (immutable? DBUS_MENU_UPDATER_IMMUTABLE : 0);
}

G_GNUC_INTERNAL DBusMenuUpdaterFlags dbus_menu_item_remove_props(DBusMenuItem *item,
                                                                 GVariant *props)
{
	GVariantIter iter;
	const char *prop;
	bool properties_is_updated = false;
	bool immutable = false;

	g_variant_iter_init(&iter, props);
	while (g_variant_iter_next(&iter, "&s", &prop))
	{
		if (g_strcmp0(prop, DBUS_MENU_PROP_ACCESSIBLE_DESC) == 0)
		{
			// TODO: Can we support this property?
			// properties_is_updated = true;
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_ENABLED) == 0)
		{
			properties_is_updated |= dbus_menu_item_update_enabled(item, true);
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_ICON_NAME) == 0)
		{
			if (g_hash_table_lookup(item->attrs, HAS_ICON_NAME))
			{
				g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_ICON);
				g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_VERB_ICON);
				g_hash_table_remove(item->attrs, HAS_ICON_NAME);
				properties_is_updated = true;
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_ICON_DATA) == 0)
		{
			if (!g_hash_table_lookup(item->attrs, HAS_ICON_NAME))
			{
				g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_ICON);
				g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_VERB_ICON);
				properties_is_updated = true;
			}
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_LABEL) == 0)
		{
			g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_LABEL);
			properties_is_updated = true;
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_SHORTCUT) == 0)
		{
			g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_ACCEL);
			properties_is_updated = true;
		}
		else if (g_strcmp0(prop, DBUS_MENU_PROP_VISIBLE) == 0)
		{
			bool old_vis = item->visible;
			g_autofree char *name =
			    dbus_menu_item_get_action_name(item, false, true);
			g_hash_table_remove(item->attrs, G_MENU_ATTRIBUTE_HIDDEN_WHEN);
			g_hash_table_insert(item->attrs,
			                    g_strdup(G_MENU_ATTRIBUTE_ACTION),
			                    g_variant_ref_sink(g_variant_new_string(name)));
			item->visible = true;
			properties_is_updated = old_vis != item->visible;
		}
		else if ((g_strcmp0(prop, DBUS_MENU_PROP_TYPE) == 0) ||
		         (g_strcmp0(prop, DBUS_MENU_PROP_KDE_TITLE) == 0))
		{
			immutable |= item->action_type == DBUS_MENU_ACTION_SECTION;
		}
		else if ((g_strcmp0(prop, DBUS_MENU_PROP_TOGGLE_TYPE) == 0) ||
		         (g_strcmp0(prop, DBUS_MENU_PROP_CHILDREN_DISPLAY) == 0))
		{
			immutable = true;
		}
		else
		{
			g_debug("removing unsupported property - '%s'", prop);
		}
	}
	return (properties_is_updated ? DBUS_MENU_UPDATER_MUTABLE : 0) | (immutable? DBUS_MENU_UPDATER_IMMUTABLE : 0);
}

G_GNUC_INTERNAL bool dbus_menu_item_compare_immutable(DBusMenuItem *a, DBusMenuItem *b)
{
	if (a->ref_action_group != b->ref_action_group)
		return false;
	if (a->action_type != b->action_type)
		return false;
	return true;
}

G_GNUC_INTERNAL bool dbus_menu_item_copy_submenu(DBusMenuItem *src, DBusMenuItem *dst,
                                                 DBusMenuModel *parent)
{
	g_autoptr(DBusMenuXml) xml = NULL;
	DBusMenuModel *submenu = NULL;
	g_object_get(parent, "xml", &xml, NULL);
	if (!dbus_menu_item_is_submenu(src))
	{
		if (dbus_menu_item_is_submenu(dst))
		{
			if (dst->toggled)
				dst->enabled = true;
			submenu = dbus_menu_model_new(dst->id, parent, xml, dst->ref_action_group);
			g_hash_table_insert(dst->links, submenu_str(dst->enabled), submenu);
			return true;
		}
		return false;
	}
	if (dbus_menu_item_is_submenu(dst) &&
	    dbus_menu_item_is_submenu(src))
	{
		if (src->toggled || dst->toggled)
			dst->enabled = dst->toggled = true;
		submenu =
		    DBUS_MENU_MODEL(g_hash_table_lookup(src->links, submenu_str(src->enabled)));
		if (!submenu)
				return false;
		g_hash_table_insert(dst->links, submenu_str(dst->enabled), g_object_ref(submenu));
		g_object_set(submenu, "parent-id", dst->id, NULL);
		return true;
	}
	return false;
}

static void dbus_menu_item_generate_action(DBusMenuItem *item, DBusMenuModel *parent)
{
	if (item->action_type == DBUS_MENU_ACTION_SECTION)
		return;
	if(!item->ref_action_group)
		return;
	g_clear_object(&item->ref_action);
	item->ref_action = dbus_menu_action_reference(parent, item);
	act_props_try_update(item);
}

G_GNUC_INTERNAL char *dbus_menu_item_get_action_name(DBusMenuItem *item, bool for_submenu,
                                                     bool use_prefix)
{
	bool submenu     = dbus_menu_item_is_submenu(item) && for_submenu;
	const char *name = g_action_get_name(submenu ? item->ref_submenu : item->ref_action);
	return g_strdup_printf("%s%s", use_prefix ? DBUS_MENU_ACTION_NAMESPACE_PREFIX : "", name);
}
