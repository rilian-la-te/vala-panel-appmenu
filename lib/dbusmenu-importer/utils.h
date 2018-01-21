#ifndef UTILS_H
#define UTILS_H

#include "dbusmenu-interface.h"
#include <gio/gio.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL GAction *dbus_menu_action_new(DBusMenuXml *xml, u_int32_t id,
                                              const char *action_type);

G_END_DECLS

#endif // UTILS_H
