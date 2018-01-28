#ifndef UTILS_H
#define UTILS_H

#include "dbusmenu-interface.h"
#include "definitions.h"
#include "model.h"
#include <gio/gio.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL GAction *dbus_menu_action_reference(u_int32_t id, DBusMenuXml *xml,
                                                    DBusMenuModel *submenu,
                                                    GActionMap *action_group,
                                                    DBusMenuActionType type);

G_GNUC_INTERNAL char *dbus_menu_action_get_name(uint id, DBusMenuActionType action_type,
                                                bool use_prefix);
G_GNUC_INTERNAL void dbus_menu_action_replace_signals(GAction *action, DBusMenuXml *xml,
                                                      DBusMenuModel *submenu,
                                                      DBusMenuActionType action_type);

G_GNUC_INTERNAL void dbus_menu_action_lock(GAction *action);
G_GNUC_INTERNAL void dbus_menu_action_unlock(GAction *action);

G_END_DECLS

#endif // UTILS_H
