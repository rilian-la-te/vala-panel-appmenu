#ifndef ITEM_H
#define ITEM_H

#include <gio/gio.h>
#include <stdbool.h>

#include "dbusmenu-interface.h"
#include "definitions.h"
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
	DBusMenuActionType action_type;
	bool enabled;
	bool toggled;
} DBusMenuItem;

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new(u_int32_t id, DBusMenuModel *parent_model,
                                                 GVariant *props);
G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new_first_section(u_int32_t id,
                                                               GActionGroup *action_group);

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data);

G_GNUC_INTERNAL bool dbus_menu_item_update_props(DBusMenuItem *item, GVariant *props);

G_GNUC_INTERNAL bool dbus_menu_item_remove_props(DBusMenuItem *item, GVariant *props);

G_GNUC_INTERNAL bool dbus_menu_item_compare_immutable(DBusMenuItem *a, DBusMenuItem *b);

G_GNUC_INTERNAL bool dbus_menu_item_copy_attributes(DBusMenuItem *src, DBusMenuItem *dst);

G_GNUC_INTERNAL void dbus_menu_item_generate_action(DBusMenuItem *item, DBusMenuModel *parent);

G_GNUC_INTERNAL int dbus_menu_item_id_compare_func(const DBusMenuItem *a, gconstpointer b,
                                                   gpointer user_data);

G_GNUC_INTERNAL int dbus_menu_item_compare_func(const DBusMenuItem *a, const DBusMenuItem *b,
                                                gpointer user_data);

G_END_DECLS

#endif // ITEM_H
