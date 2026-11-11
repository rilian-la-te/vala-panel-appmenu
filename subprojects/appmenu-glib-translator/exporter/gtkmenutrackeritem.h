/*
 * Copyright © 2011, 2013 Canonical Limited
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#pragma once

#include <gio/gio.h>

G_DECLARE_FINAL_TYPE(DBusMenuExporterTrackerItem, dbus_menu_exporter_tracker_item, DBUS_MENU, EXPORTER_TRACKER_ITEM, GObject)

typedef enum _DBusMenuRole DBusMenuRole;

typedef struct _DBusMenuExporterTrackerItem DBusMenuExporterTrackerItem;
typedef struct _GtkActionNode GtkActionNode;

DBusMenuExporterTrackerItem     *_dbus_menu_exporter_tracker_item_new                    (GtkActionNode      *action_node,
                                                                       GMenuModel         *model,
                                                                       int                 item_index,
                                                                       gboolean            mac_os_mode,
                                                                       const char         *action_namespace,
                                                                       gboolean            is_separator);
const char             *dbus_menu_exporter_tracker_item_get_action_name         (DBusMenuExporterTrackerItem *self);
GVariant               *dbus_menu_exporter_tracker_item_get_action_target       (DBusMenuExporterTrackerItem *self);
const char             *dbus_menu_exporter_tracker_item_get_special             (DBusMenuExporterTrackerItem *self);
const char             *dbus_menu_exporter_tracker_item_get_custom              (DBusMenuExporterTrackerItem *self);
const char             *dbus_menu_exporter_tracker_item_get_display_hint        (DBusMenuExporterTrackerItem *self);
const char             *dbus_menu_exporter_tracker_item_get_text_direction      (DBusMenuExporterTrackerItem *self);
GtkActionNode          *_dbus_menu_exporter_tracker_item_get_action_node        (DBusMenuExporterTrackerItem *self);
gboolean                dbus_menu_exporter_tracker_item_get_is_separator        (DBusMenuExporterTrackerItem *self);
gboolean                dbus_menu_exporter_tracker_item_get_has_link            (DBusMenuExporterTrackerItem *self,
                                                                       const char         *link_name);
const char             *dbus_menu_exporter_tracker_item_get_label               (DBusMenuExporterTrackerItem *self);
gboolean                dbus_menu_exporter_tracker_item_get_use_markup          (DBusMenuExporterTrackerItem *self);
GIcon                  *dbus_menu_exporter_tracker_item_get_icon                (DBusMenuExporterTrackerItem *self);
GIcon                  *dbus_menu_exporter_tracker_item_get_verb_icon           (DBusMenuExporterTrackerItem *self);
gboolean                dbus_menu_exporter_tracker_item_get_sensitive           (DBusMenuExporterTrackerItem *self);
DBusMenuRole            dbus_menu_exporter_tracker_item_get_role                (DBusMenuExporterTrackerItem *self);
gboolean                dbus_menu_exporter_tracker_item_get_toggled             (DBusMenuExporterTrackerItem *self);
const char             *dbus_menu_exporter_tracker_item_get_accel               (DBusMenuExporterTrackerItem *self);
GMenuModel             *_dbus_menu_exporter_tracker_item_get_link               (DBusMenuExporterTrackerItem *self,
                                                                       const char         *link_name);
char                   *_dbus_menu_exporter_tracker_item_get_link_namespace     (DBusMenuExporterTrackerItem *self);
gboolean                dbus_menu_exporter_tracker_item_may_disappear           (DBusMenuExporterTrackerItem *self);
gboolean                dbus_menu_exporter_tracker_item_get_is_visible          (DBusMenuExporterTrackerItem *self);
gboolean                dbus_menu_exporter_tracker_item_get_should_request_show (DBusMenuExporterTrackerItem *self);
void                    dbus_menu_exporter_tracker_item_activated               (DBusMenuExporterTrackerItem *self);
void                    dbus_menu_exporter_tracker_item_request_submenu_shown   (DBusMenuExporterTrackerItem *self,
                                                                       gboolean            shown);
gboolean                dbus_menu_exporter_tracker_item_get_submenu_shown       (DBusMenuExporterTrackerItem *self);

