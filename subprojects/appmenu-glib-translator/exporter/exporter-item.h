// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define DBUS_MENU_TYPE_EXPORTER_ITEM (dbus_menu_exporter_item_get_type ())
G_DECLARE_FINAL_TYPE (DBusMenuExporterItem,
                      dbus_menu_exporter_item,
                      DBUS_MENU,
                      EXPORTER_ITEM,
                      GObject)


DBusMenuExporterItem *dbus_menu_exporter_item_new (void);

G_END_DECLS
