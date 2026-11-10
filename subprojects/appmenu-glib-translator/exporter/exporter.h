// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuExporter, dbus_menu_exporter, DBUS_MENU, EXPORTER, GObject)

DBusMenuExporter *dbus_menu_exporter_new(GMenuModel *menu_model, gchar *action_prefix,
                                         GActionGroup *action_group, gchar *bus_name,
                                         gchar *object_path);

G_END_DECLS
