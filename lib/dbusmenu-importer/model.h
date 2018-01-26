#ifndef MODEL_H
#define MODEL_H

#include "dbusmenu-interface.h"
#include <gio/gio.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuModel, dbus_menu_model, DBUS_MENU, MODEL, GMenuModel)
G_GNUC_INTERNAL DBusMenuModel *dbus_menu_model_new(uint parent_id, DBusMenuModel *parent,
                                                   DBusMenuXml *xml, GActionGroup *action_group);
G_GNUC_INTERNAL void dbus_menu_model_update_layout(DBusMenuModel *menu);
G_GNUC_INTERNAL GSequenceIter *dbus_menu_model_get_section_iter(DBusMenuModel *model,
                                                                uint section_index);

G_END_DECLS

#endif
