// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exporter-item.h"
#include "dbusmenu-interface.h"
#include "definitions.h"
#include "exporter/exporter.h"
#include "glib.h"
#include "glibconfig.h"
#include "gtkmenutracker.h"
#include "gtkmenutrackeritem.h"

G_GNUC_INTERNAL void dbus_menu_exporter_item_free(gpointer data);
G_GNUC_INTERNAL DBusMenuExporterItem *dbus_menu_exporter_item_copy(DBusMenuExporterItem *src);
G_DEFINE_BOXED_TYPE(DBusMenuExporterItem, dbus_menu_exporter_item, dbus_menu_exporter_item_copy,
                    dbus_menu_exporter_item_free)

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

G_GNUC_INTERNAL GVariant *
dbus_menu_exporter_item_shortcut_from_accel (const char *accel)
{
    if (!accel || !*accel)
        return NULL;

    GPtrArray *parts = g_ptr_array_new_with_free_func (g_free);
    const char *p = accel;

    while (*p) {
        if (*p == '<') {
            const char *end = strchr (p + 1, '>');
            if (!end)
                goto fail;

            g_autofree char *mod = g_strndup (p + 1, end - p - 1);
            const char *dbus_mod = NULL;

            if (g_ascii_strcasecmp (mod, DBUS_MENU_SHORTCUT_CONTROL) == 0)
                dbus_mod = DBUS_MENU_SHORTCUT_CONTROL;
            else if (g_ascii_strcasecmp (mod, DBUS_MENU_SHORTCUT_ALT)   == 0)
                dbus_mod = DBUS_MENU_SHORTCUT_ALT;
            else if (g_ascii_strcasecmp (mod, DBUS_MENU_SHORTCUT_SHIFT) == 0)
                dbus_mod = DBUS_MENU_SHORTCUT_SHIFT;
            else if (g_ascii_strcasecmp (mod, DBUS_MENU_SHORTCUT_SUPER) == 0)
                dbus_mod = DBUS_MENU_SHORTCUT_SUPER;
            else
                goto fail;

            g_ptr_array_add (parts, g_strdup (dbus_mod));
            p = end + 1;
        } else if (*p == ' ') {
            p++;
        } else {
            const char *end = p;
            while (*end && *end != '<' && *end != ' ')
                end++;
            if (end == p)
                goto fail;
            g_ptr_array_add (parts, g_strndup (p, end - p));
            p = end;
        }
    }

    if (parts->len == 0)
        goto fail;

    GVariantBuilder outer, inner;
    g_variant_builder_init (&outer, G_VARIANT_TYPE ("aas"));
    g_variant_builder_init (&inner, G_VARIANT_TYPE ("as"));
    for (guint i = 0; i < parts->len; i++)
        g_variant_builder_add (&inner, "s",
                               (const char *) g_ptr_array_index (parts, i));
    g_variant_builder_add_value (&outer, g_variant_builder_end (&inner));

    g_ptr_array_unref (parts);
    return g_variant_builder_end (&outer);

fail:
    g_ptr_array_unref (parts);
    return NULL;
}

G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_get_variant_for_prop(DBusMenuExporterItem *self,
                                                                       const char *prop)
{
	GtkMenuTrackerItem *item = self->tracker_item;

	if (g_str_equal(prop, DBUS_MENU_PROP_TYPE))
	{
		return g_variant_new_string(gtk_menu_tracker_item_get_is_separator(item)
		                                ? DBUS_MENU_TYPE_SEPARATOR
		                                : DBUS_MENU_TYPE_NORMAL);
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_LABEL))
	{
		const char *label = gtk_menu_tracker_item_get_label(item);
		if (label)
			return g_variant_new_string(label);
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_ICON_NAME))
	{
		GIcon *icon = gtk_menu_tracker_item_get_icon(item);
		if (!icon)
			icon = gtk_menu_tracker_item_get_verb_icon(item);
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
		return g_variant_new_boolean(gtk_menu_tracker_item_get_sensitive(item));
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_TOGGLE_TYPE))
	{
		GtkMenuTrackerItemRole role = gtk_menu_tracker_item_get_role(item);
		gboolean want_toggle        = !self->is_submenu;

		switch (role)
		{
		case GTK_MENU_TRACKER_ITEM_ROLE_CHECK:
			if (want_toggle)
				return g_variant_new_string(DBUS_MENU_TOGGLE_TYPE_CHECK);
			break;
		case GTK_MENU_TRACKER_ITEM_ROLE_RADIO:
			if (want_toggle)
				return g_variant_new_string(DBUS_MENU_TOGGLE_TYPE_RADIO);
			break;
		case GTK_MENU_TRACKER_ITEM_ROLE_NORMAL:
		default:
			break;
		}
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_TOGGLE_STATE))
	{
		return g_variant_new_int32(gtk_menu_tracker_item_get_toggled(item));
	}
	if (g_str_equal(prop, DBUS_MENU_PROP_SHORTCUT))
	{
		const char *accel = gtk_menu_tracker_item_get_accel(item);
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
		return g_variant_new_boolean(gtk_menu_tracker_item_get_is_visible(item));
	}

	return NULL;
}

G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_build_properties_variant(
    DBusMenuExporterItem *self)
{
	GVariantDict vardict;
	guint n_properties;
	GObjectClass *obj_class       = G_OBJECT_GET_CLASS(self->tracker_item);
	g_autofree GParamSpec **props = g_object_class_list_properties(obj_class, &n_properties);

	g_variant_dict_init(&vardict, NULL);
	if (self->is_submenu)
	{
		g_variant_dict_insert_value(&vardict,
		                            DBUS_MENU_PROP_CHILDREN_DISPLAY,
		                            g_variant_new_string(
		                                DBUS_MENU_CHILDREN_DISPLAY_SUBMENU));
	}
	for (guint i = 0; i < n_properties; i++)
	{
		GParamSpec *in = props[i];

		if (in->owner_type == GTK_TYPE_MENU_TRACKER_ITEM)
		{
			const gchar *prop = dbus_menu_prop_for_tracker_prop(in->name);
			if(!prop)
				continue;
			GVariant *value   = dbus_menu_exporter_item_get_variant_for_prop(self, prop);
			if (value && !g_variant_dict_contains(&vardict, prop))
				g_variant_dict_insert_value(&vardict, prop, value);
		}
	}

	return g_variant_dict_end(&vardict);
}

G_GNUC_INTERNAL void dbus_menu_exporter_item_event(DBusMenuExporterItem *self, const char *event)
{
	if (g_str_equal(event, DBUS_MENU_EVENT_ACTIVATED))
		gtk_menu_tracker_item_activated(self->tracker_item);
	if (g_str_equal(event, DBUS_MENU_EVENT_CLOSED))
		gtk_menu_tracker_item_request_submenu_shown(self->tracker_item, FALSE);
	if (g_str_equal(event, DBUS_MENU_EVENT_OPENED))
		gtk_menu_tracker_item_request_submenu_shown(self->tracker_item, TRUE);
}

G_GNUC_INTERNAL void dbus_menu_exporter_item_about_to_show(DBusMenuExporterItem *self)
{
	gtk_menu_tracker_item_request_submenu_shown(self->tracker_item, TRUE);
}

static void
on_tracker_item_notify (GObject *object, GParamSpec *pspec, gpointer user_data)
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
                                                                  GtkMenuTrackerItem *tr_item,
                                                                  DBusMenuExporterItem *parent)
{
	DBusMenuExporterItem *item = g_slice_new0(DBusMenuExporterItem);
	item->tracker_item         = tr_item;
	item->id                   = id;
	item->parent_id            = parent ? parent->id : 0;
	item->exporter             = exporter;
	item->children             = g_array_new(FALSE, TRUE, sizeof(u_int32_t));
	if (tr_item)
		g_signal_connect (tr_item, "notify", G_CALLBACK(on_tracker_item_notify), item);
	return item;
}

/* This function alone would produce dangling children on parent */
void dbus_menu_exporter_item_free(gpointer data)
{
	DBusMenuExporterItem *item = (DBusMenuExporterItem *)data;
	if (item == NULL)
		return;
	if(item->tracker_item)
		g_signal_handlers_disconnect_by_data(item->tracker_item, item);
	g_clear_object(&item->child_tracker);
	g_clear_pointer(&item->children, g_array_unref);
	g_slice_free(DBusMenuExporterItem, data);
}

G_GNUC_INTERNAL DBusMenuExporterItem *dbus_menu_exporter_item_copy(DBusMenuExporterItem *src)
{
	DBusMenuExporterItem *dst = g_slice_new0(DBusMenuExporterItem);
	dst->id                   = src->id;
	dst->parent_id            = src->parent_id;
	dst->tracker_item         = src->tracker_item;
	dst->exporter             = src->exporter;
	dst->child_tracker        = src->child_tracker;
	dst->children             = g_array_ref(src->children);
	dst->child_tracker        = g_object_ref(src->child_tracker);
	dst->is_submenu           = src->is_submenu;
	return dst;
}
