/*
 * vala-panel-appmenu
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ITEM_H
#define ITEM_H

#include <gio/gio.h>
#include <stdbool.h>

#include "definitions.h"
#include "model.h"

G_BEGIN_DECLS

typedef struct
{
	int section_num;
	int place;
	u_int32_t id;
	GActionGroup *ref_action_group;
	// FIXME: Cannot have activatable submenu item.
	GAction *ref_action;
	GHashTable *attrs;
	GHashTable *links;
	DBusMenuActionType action_type;
	bool enabled;
	bool toggled;
	gpointer magic;
} DBusMenuItem;

typedef enum 
{
	DBUS_MENU_UPDATER_NONE = 0,
	DBUS_MENU_UPDATER_MUTABLE = 1,
	DBUS_MENU_UPDATER_IMMUTABLE = 2
} DBusMenuUpdaterFlags;

typedef GQueue UpdaterQueue;

static void updater_queue_free(UpdaterQueue *q)
{
    g_queue_free_full(q, g_free);
}
G_DEFINE_AUTOPTR_CLEANUP_FUNC(UpdaterQueue, updater_queue_free)

G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new(u_int32_t id, DBusMenuModel *parent_model,
                                                 GVariant *props);
G_GNUC_INTERNAL DBusMenuItem *dbus_menu_item_new_first_section(u_int32_t id,
                                                               GActionGroup *action_group);

G_GNUC_INTERNAL void dbus_menu_item_free(gpointer data);

G_GNUC_INTERNAL bool dbus_menu_item_update_enabled(DBusMenuItem *item, bool enabled);

G_GNUC_INTERNAL DBusMenuUpdaterFlags dbus_menu_item_update_props(DBusMenuItem *item, GVariant *props);

G_GNUC_INTERNAL DBusMenuUpdaterFlags dbus_menu_item_remove_props(DBusMenuItem *item, GVariant *props);

G_GNUC_INTERNAL bool dbus_menu_item_compare_immutable(DBusMenuItem *a, DBusMenuItem *b);

G_GNUC_INTERNAL bool dbus_menu_item_is_firefox_stub(DBusMenuItem *item);

G_GNUC_INTERNAL bool dbus_menu_item_copy_submenu(DBusMenuItem *src, DBusMenuItem *dst,
                                                 DBusMenuModel *parent);

G_GNUC_INTERNAL void dbus_menu_item_generate_action(DBusMenuItem *item, DBusMenuModel *parent);

G_GNUC_INTERNAL void dbus_menu_item_preload(DBusMenuItem *item);

G_GNUC_INTERNAL int dbus_menu_item_id_compare_func(const DBusMenuItem *a, gconstpointer b,
                                                   gpointer user_data);

G_GNUC_INTERNAL int dbus_menu_item_compare_func(const DBusMenuItem *a, const DBusMenuItem *b,
                                                gpointer user_data);
G_DEFINE_AUTOPTR_CLEANUP_FUNC(DBusMenuItem, dbus_menu_item_free)

G_END_DECLS

#endif // ITEM_H
