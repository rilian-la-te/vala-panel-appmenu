// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exporter-item.h"
#include "dbusmenu-interface.h"
#include "definitions.h"
#include "exporter/exporter.h"
#include "gtkmenutracker.h"
#include "gtkmenutrackeritem.h"

G_GNUC_INTERNAL guint dbus_menu_exporter_item_get_id(DBusMenuExporterItem *self)
{
	return self->id;
}

G_GNUC_INTERNAL DBusMenuExporter *dbus_menu_exporter_item_get_exporter(DBusMenuExporterItem *self)
{
	return self->exporter;
}

G_GNUC_INTERNAL void dbus_menu_exporter_item_add_child(DBusMenuExporterItem *self,
                                                       DBusMenuExporterItem *child, u_int32_t pos)
{
	g_array_insert_val(self->children, pos, child->id);
}

G_GNUC_INTERNAL void dbus_menu_exporter_item_remove_child(DBusMenuExporterItem *parent,
                                                          u_int32_t pos)
{
	g_array_remove_index(parent->children, pos);
}

G_GNUC_INTERNAL const gchar *dbus_menu_prop_for_tracker_prop(const gchar *name)
{
	if (!name)
		return NULL;

	if (g_str_equal(name, "is-separator"))
		return DBUS_MENU_PROP_TYPE;

	if (g_str_equal(name, "label"))
		return DBUS_MENU_PROP_LABEL;

	if (g_str_equal(name, "icon") || g_str_equal(name, "verb-icon"))
		return DBUS_MENU_PROP_ICON_NAME;

	if (g_str_equal(name, "sensitive"))
		return DBUS_MENU_PROP_ENABLED;

	if (g_str_equal(name, "role"))
		return DBUS_MENU_PROP_TOGGLE_TYPE;

	if (g_str_equal(name, "toggled"))
		return DBUS_MENU_PROP_TOGGLE_STATE;

	if (g_str_equal(name, "accel"))
		return DBUS_MENU_PROP_SHORTCUT;

	if (g_str_equal(name, "submenu-shown"))
		return DBUS_MENU_PROP_CHILDREN_DISPLAY;

	if (g_str_equal(name, "is-visible"))
		return DBUS_MENU_PROP_VISIBLE;

	if (g_str_equal(name, DBUS_MENU_PROP_TYPE) || g_str_equal(name, DBUS_MENU_PROP_LABEL) ||
	    g_str_equal(name, DBUS_MENU_PROP_ICON_NAME) ||
	    g_str_equal(name, DBUS_MENU_PROP_ICON_DATA) ||
	    g_str_equal(name, DBUS_MENU_PROP_ENABLED) ||
	    g_str_equal(name, DBUS_MENU_PROP_TOGGLE_TYPE) ||
	    g_str_equal(name, DBUS_MENU_PROP_TOGGLE_STATE) ||
	    g_str_equal(name, DBUS_MENU_PROP_SHORTCUT) ||
	    g_str_equal(name, DBUS_MENU_PROP_CHILDREN_DISPLAY) ||
	    g_str_equal(name, DBUS_MENU_PROP_VISIBLE))
		return name;

	return NULL;
}

G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_shortcut_from_accel(const char *accel)
{
	if (!accel || !*accel)
		return NULL;

	GPtrArray *parts = g_ptr_array_new_with_free_func(g_free);
	const char *p    = accel;

	while (*p)
	{
		if (*p == '<')
		{
			const char *end = strchr(p + 1, '>');
			if (!end)
				goto fail;

			g_autofree char *mod = g_strndup(p + 1, end - p - 1);
			const char *dbus_mod = NULL;

			if (g_ascii_strcasecmp(mod, DBUS_MENU_SHORTCUT_CONTROL) == 0)
				dbus_mod = DBUS_MENU_SHORTCUT_CONTROL;
			else if (g_ascii_strcasecmp(mod, DBUS_MENU_SHORTCUT_ALT) == 0)
				dbus_mod = DBUS_MENU_SHORTCUT_ALT;
			else if (g_ascii_strcasecmp(mod, DBUS_MENU_SHORTCUT_SHIFT) == 0)
				dbus_mod = DBUS_MENU_SHORTCUT_SHIFT;
			else if (g_ascii_strcasecmp(mod, DBUS_MENU_SHORTCUT_SUPER) == 0)
				dbus_mod = DBUS_MENU_SHORTCUT_SUPER;
			else
				goto fail;

			g_ptr_array_add(parts, g_strdup(dbus_mod));
			p = end + 1;
		}
		else if (*p == ' ')
		{
			p++;
		}
		else
		{
			const char *end = p;
			while (*end && *end != '<' && *end != ' ')
				end++;
			if (end == p)
				goto fail;
			g_ptr_array_add(parts, g_strndup(p, end - p));
			p = end;
		}
	}

	if (parts->len == 0)
		goto fail;

	GVariantBuilder outer, inner;
	g_variant_builder_init(&outer, G_VARIANT_TYPE("aas"));
	g_variant_builder_init(&inner, G_VARIANT_TYPE("as"));
	for (guint i = 0; i < parts->len; i++)
		g_variant_builder_add(&inner, "s", (const char *)g_ptr_array_index(parts, i));
	g_variant_builder_add_value(&outer, g_variant_builder_end(&inner));

	g_ptr_array_unref(parts);
	return g_variant_builder_end(&outer);

fail:
	g_ptr_array_unref(parts);
	return NULL;
}

G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_get_variant_for_prop(DBusMenuExporterItem *self,
                                                                       const char *prop)
{
	DBusMenuExporterTrackerItem *item = self->tracker_item;

	if (g_str_equal(prop, DBUS_MENU_PROP_TYPE))
	{
		return g_variant_new_string(dbus_menu_exporter_tracker_item_get_is_separator(item)
		                                ? DBUS_MENU_TYPE_SEPARATOR
		                                : DBUS_MENU_TYPE_NORMAL);
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_LABEL))
	{
		const char *label = dbus_menu_exporter_tracker_item_get_label(item);
		if (label)
			return g_variant_new_string(label);
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_ICON_NAME))
	{
		GIcon *icon = dbus_menu_exporter_tracker_item_get_icon(item);
		if (!icon)
			icon = dbus_menu_exporter_tracker_item_get_verb_icon(item);
		if (icon)
		{
			if (G_IS_THEMED_ICON(icon))
			{
				const char *const *names =
				    g_themed_icon_get_names(G_THEMED_ICON(icon));
				if (names && names[0])
					return g_variant_new_string(names[0]);
			}
		}
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_ICON_DATA))
	{
		// TODO: icon-data with GdkPixbuf as optional
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_ENABLED))
	{
		return g_variant_new_boolean(dbus_menu_exporter_tracker_item_get_sensitive(item));
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_TOGGLE_TYPE))
	{
		DBusMenuRole role = dbus_menu_exporter_tracker_item_get_role(item);
		gboolean want_toggle        = !self->is_submenu;

		switch (role)
		{
		case DBUS_MENU_ROLE_CHECK:
			if (want_toggle)
				return g_variant_new_string(DBUS_MENU_TOGGLE_TYPE_CHECK);
			break;
		case DBUS_MENU_ROLE_RADIO:
			if (want_toggle)
				return g_variant_new_string(DBUS_MENU_TOGGLE_TYPE_RADIO);
			break;
		case DBUS_MENU_ROLE_NORMAL:
		default:
			break;
		}
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_TOGGLE_STATE))
	{
		return g_variant_new_int32(dbus_menu_exporter_tracker_item_get_toggled(item));
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_SHORTCUT))
	{
		const char *accel = dbus_menu_exporter_tracker_item_get_accel(item);
		if (accel)
			return dbus_menu_exporter_item_shortcut_from_accel(accel);
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_CHILDREN_DISPLAY))
	{
		if (self->is_submenu)
			return g_variant_new_string(DBUS_MENU_CHILDREN_DISPLAY_SUBMENU);
	}
	if (g_str_equal(prop, "is-visible") || g_str_equal(prop, DBUS_MENU_PROP_VISIBLE))
	{
		return g_variant_new_boolean(dbus_menu_exporter_tracker_item_get_is_visible(item));
	}

	return NULL;
}

G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_build_properties_variant(
    DBusMenuExporterItem *self, const gchar *const *requested_props)
{
	GVariantDict vardict;
	guint n_properties;

	g_variant_dict_init(&vardict, NULL);
	if (self->is_submenu && !requested_props)
	{
		g_variant_dict_insert_value(&vardict,
		                            DBUS_MENU_PROP_CHILDREN_DISPLAY,
		                            g_variant_new_string(
		                                DBUS_MENU_CHILDREN_DISPLAY_SUBMENU));
	}
	if (!self->tracker_item)
	{
		if (self->id)
			g_warning("%d: no tracker for non-root item!", self->id);
		return g_variant_dict_end(&vardict);
	}

	GObjectClass *obj_class       = G_OBJECT_GET_CLASS(self->tracker_item);
	g_autofree GParamSpec **props = g_object_class_list_properties(obj_class, &n_properties);

	if (requested_props)
		n_properties = g_strv_length((GStrv)requested_props);
	for (u_int32_t i = 0; i < n_properties; i++)
	{
		const gchar *prop = NULL;
		if (requested_props)
			prop = requested_props[i];
		else
		{
			GParamSpec *in = props[i];
			if (in->owner_type != dbus_menu_exporter_tracker_item_get_type())
				continue;
			prop = dbus_menu_prop_for_tracker_prop(in->name);
		}
		if (!prop)
			continue;

		GVariant *value = dbus_menu_exporter_item_get_variant_for_prop(self, prop);
		if (value && !g_variant_dict_contains(&vardict, prop))
			g_variant_dict_insert_value(&vardict, prop, value);
		else if (value)
			g_variant_unref(value);
	}

	return g_variant_dict_end(&vardict);
}

G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_build_representation(
    DBusMenuExporterItem *self, int children_depth, const gchar *const *requested_props)
{
	GVariantBuilder item_variant;
	g_variant_builder_init(&item_variant, G_VARIANT_TYPE("(ia{sv}av)"));
	g_variant_builder_add(&item_variant, "i", (gint)self->id);
	g_variant_builder_add_value(
	    &item_variant, dbus_menu_exporter_item_build_properties_variant(self, requested_props));
	g_variant_builder_open(&item_variant, G_VARIANT_TYPE("av"));
	if (children_depth != 0 && self->is_submenu)
	{
		for (int i = 0; i < self->children->len; i++)
		{
			u_int32_t child_id = g_array_index(self->children, u_int32_t, i);
			DBusMenuExporterItem *child =
			    dbus_menu_exporter_get_by_id(self->exporter, child_id);
			if (!child || !child_id)
			{
				g_warning("%d: bad child id %d", self->id, child_id);
				continue;
			}
			GVariant *child_repr =
			    dbus_menu_exporter_item_build_representation(child,
			                                                 children_depth - 1,
			                                                 requested_props);
			g_variant_builder_add(&item_variant, "v", child_repr);
		}
	}
	g_variant_builder_close(&item_variant);
	return g_variant_builder_end(&item_variant);
}

G_GNUC_INTERNAL void dbus_menu_exporter_item_event(DBusMenuExporterItem *self, const char *event)
{
	if (g_str_equal(event, DBUS_MENU_EVENT_ACTIVATED))
		dbus_menu_exporter_tracker_item_activated(self->tracker_item);
	if (g_str_equal(event, DBUS_MENU_EVENT_CLOSED))
		dbus_menu_exporter_tracker_item_request_submenu_shown(self->tracker_item, FALSE);
	if (g_str_equal(event, DBUS_MENU_EVENT_OPENED))
		dbus_menu_exporter_tracker_item_request_submenu_shown(self->tracker_item, TRUE);
}

G_GNUC_INTERNAL void dbus_menu_exporter_item_about_to_show(DBusMenuExporterItem *self)
{
	dbus_menu_exporter_tracker_item_request_submenu_shown(self->tracker_item, TRUE);
}

static void on_tracker_item_notify(GObject *object, GParamSpec *pspec, gpointer user_data)
{
	DBusMenuExporterItem *item = (DBusMenuExporterItem *)user_data;
	DBusMenuExporter *exporter = dbus_menu_exporter_item_get_exporter(item);

	const gchar *dbus_prop = dbus_menu_prop_for_tracker_prop(pspec->name);
	if (!dbus_prop)
		return;

	dbus_menu_exporter_queue_property_change(exporter, item, dbus_prop);
}

G_GNUC_INTERNAL DBusMenuExporterItem *dbus_menu_exporter_item_new(DBusMenuExporter *exporter,
                                                                  u_int32_t id,
                                                                  DBusMenuExporterTrackerItem *tr_item,
                                                                  DBusMenuExporterItem *parent)
{
	DBusMenuExporterItem *item = g_slice_new0(DBusMenuExporterItem);
	item->tracker_item         = tr_item ? g_object_ref(tr_item) : NULL;
	item->id                   = id;
	item->parent_id            = parent ? parent->id : 0;
	item->exporter             = exporter;
	item->children             = g_array_new(FALSE, TRUE, sizeof(u_int32_t));
	if (tr_item)
		g_signal_connect(tr_item, "notify", G_CALLBACK(on_tracker_item_notify), item);
	return item;
}

/* This function alone would produce dangling children on parent */
void dbus_menu_exporter_item_free(gpointer data)
{
	DBusMenuExporterItem *item = (DBusMenuExporterItem *)data;
	if (item == NULL)
		return;
	if (item->tracker_item)
	{
		g_signal_handlers_disconnect_by_data(item->tracker_item, item);
		dbus_menu_exporter_tracker_item_request_submenu_shown(item->tracker_item, FALSE);
		g_object_unref(item->tracker_item);
	}
	g_clear_pointer(&item->child_tracker, gtk_menu_tracker_free);
	g_clear_pointer(&item->children, g_array_unref);
	g_slice_free(DBusMenuExporterItem, data);
}
