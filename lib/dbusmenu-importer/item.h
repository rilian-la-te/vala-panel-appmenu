#ifndef ITEM_H
#define ITEM_H

#include <gio/gio.h>
#include <stdbool.h>

#include "dbusmenu-interface.h"
#include "model.h"

G_BEGIN_DECLS

typedef struct
{
	u_int32_t id;
	bool is_section;
	GActionGroup *referenced_action_group;
	// FIXME: Cannot have activatable submenu item.
	GAction *referenced_action;
	GHashTable *attributes;
	GHashTable *links;
} DBusMenuItem;

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new(u_int32_t id, DBusMenuItem *section_item,
                                                 DBusMenuModel *parent_model, GVariant *props);
G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new_first_section(u_int32_t id, DBusMenuXml *xml,
                                                               GActionGroup *action_group);

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data);

G_GNUC_INTERNAL bool dbus_menu_item_update_props(DBusMenuItem *item, GVariant *props);

G_GNUC_INTERNAL bool dbus_menu_item_remove_props(DBusMenuItem *item, GVariant *props);

G_END_DECLS

#endif // ITEM_H
