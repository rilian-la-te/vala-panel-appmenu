#ifndef MODEL_H
#define MODEL_H

#include "dbusmenu-interface.h"
#include <gio/gio.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuModel, dbus_menu_model, DBusMenu, Model, GMenuModel)

G_GNUC_INTERNAL DBusMenuModel *dbus_menu_model_new(uint parent_id, DBusMenuModel *parent, DBusMenuXml *xml,
                                   GActionGroup *action_group);
G_GNUC_INTERNAL void dbus_menu_model_update_layout(DBusMenuModel *menu);

G_END_DECLS

#endif
