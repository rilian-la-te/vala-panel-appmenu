// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "exporter.h"

G_BEGIN_DECLS

typedef struct _GtkMenuTracker GtkMenuTracker;
typedef struct _DBusMenuExporterTrackerItem DBusMenuExporterTrackerItem;

typedef struct _DBusMenuExporterItem
{
	u_int32_t id;
	u_int32_t parent_id;
	DBusMenuExporterTrackerItem *tracker_item;
	GArray *children;
	GtkMenuTracker *child_tracker;
	DBusMenuExporter *exporter;
	gboolean is_submenu;
} DBusMenuExporterItem;

G_GNUC_INTERNAL DBusMenuExporterItem *dbus_menu_exporter_get_by_id(DBusMenuExporter *exporter, u_int32_t id);
G_GNUC_INTERNAL void
dbus_menu_exporter_queue_property_change (DBusMenuExporter *self,
                                          DBusMenuExporterItem *item,
                                          const gchar *dbus_prop);
G_GNUC_INTERNAL DBusMenuExporterItem *dbus_menu_exporter_item_new(DBusMenuExporter *exporter, u_int32_t id,
                                                  DBusMenuExporterTrackerItem *item,
                                                  DBusMenuExporterItem *parent);
void dbus_menu_exporter_item_free(gpointer data);
G_GNUC_INTERNAL DBusMenuExporter *dbus_menu_exporter_item_get_exporter(DBusMenuExporterItem *self);
G_GNUC_INTERNAL guint dbus_menu_exporter_item_get_id(DBusMenuExporterItem *self);
G_GNUC_INTERNAL void dbus_menu_exporter_item_add_child(DBusMenuExporterItem *self,
                                                       DBusMenuExporterItem *child, u_int32_t pos);
G_GNUC_INTERNAL void dbus_menu_exporter_item_remove_child(DBusMenuExporterItem *parent,
                                                          u_int32_t pos);
G_GNUC_INTERNAL void dbus_menu_exporter_item_event(DBusMenuExporterItem* self, const char* event);
G_GNUC_INTERNAL void dbus_menu_exporter_item_about_to_show(DBusMenuExporterItem* self);
G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_get_variant_for_prop(DBusMenuExporterItem *self,
                                                                       const char *prop);
G_GNUC_INTERNAL const gchar *dbus_menu_prop_for_tracker_prop(const gchar *name);
G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_build_properties_variant(DBusMenuExporterItem *self,
                                                                           const gchar *const *requested_props);
G_GNUC_INTERNAL GVariant *dbus_menu_exporter_item_build_representation(DBusMenuExporterItem *self,
                                                                       int children_depth, const gchar *const *requested_props);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (DBusMenuExporterItem, dbus_menu_exporter_item_free)
G_END_DECLS
