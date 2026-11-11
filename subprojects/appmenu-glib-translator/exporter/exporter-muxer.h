/* SPDX-License-Identifier: LGPL-3.0-or-later */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuExporterMuxer, dbus_menu_exporter_muxer, DBUS_MENU, EXPORTER_MUXER, GObject)
typedef struct _GtkActionNode GtkActionNode;

DBusMenuExporterMuxer * dbus_menu_exporter_muxer_new                            (void);

void                    dbus_menu_exporter_muxer_insert                         (DBusMenuExporterMuxer *muxer,
                                                                         const gchar    *prefix,
                                                                         GActionGroup   *action_group);
void                    dbus_menu_exporter_muxer_remove                         (DBusMenuExporterMuxer *muxer,
                                                                         const gchar    *prefix);
const gchar **          dbus_menu_exporter_muxer_list_prefixes                  (DBusMenuExporterMuxer *muxer);
GActionGroup *          dbus_menu_exporter_muxer_lookup                         (DBusMenuExporterMuxer *muxer,
                                                                         const gchar    *prefix);
GtkActionNode*          dbus_menu_exporter_muxer_get_node(DBusMenuExporterMuxer* self);

G_END_DECLS
