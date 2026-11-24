/* SPDX-License-Identifier: LGPL-3.0-or-later */

#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuMockServer, dbus_menu_mock_server, DBUS_MENU, MOCK_SERVER, GObject)
DBusMenuMockServer *dbus_menu_mock_server_new(GVariant *layout, const char *bus_name,
                                              const char *object_path);

G_END_DECLS
