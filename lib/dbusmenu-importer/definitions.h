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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define DBUS_MENU_ACTION_NAMESPACE "dbusmenu"
#define DBUS_MENU_ACTION_NAMESPACE_PREFIX DBUS_MENU_ACTION_NAMESPACE "."

#define ACTION_PREFIX "id-"
#define SUBMENU_PREFIX "submenu-"
#define CURRENT_TIME 0L
#define HAS_ICON_NAME "has-icon-name"

typedef enum
{
	DBUS_MENU_ACTION_SECTION,
	DBUS_MENU_ACTION_NORMAL,
	DBUS_MENU_ACTION_CHECKMARK,
	DBUS_MENU_ACTION_RADIO,
	DBUS_MENU_ACTION_SUBMENU,
	DBUS_MENU_ACTION_ALL
} DBusMenuActionType;

#define SUBMENU_ACTION_MENUMODEL_QUARK_STR "submenu-action_menumodel"
#define ACTIVATE_ID_QUARK_STR "checker-quark"
#define POPULATED_QUARK "is-populated"

#define DBUS_MENU_PROP_TYPE "type"
#define DBUS_MENU_TYPE_SEPARATOR "separator"
#define DBUS_MENU_TYPE_NORMAL "normal"

#define DBUS_MENU_PROP_TOGGLE_TYPE "toggle-type"
#define DBUS_MENU_TOGGLE_TYPE_CHECK "checkmark"
#define DBUS_MENU_TOGGLE_TYPE_RADIO "radio"

#define DBUS_MENU_PROP_CHILDREN_DISPLAY "children-display"
#define DBUS_MENU_CHILDREN_DISPLAY_SUBMENU "submenu"

#define DBUS_MENU_SHORTCUT_CONTROL "Control"
#define DBUS_MENU_SHORTCUT_ALT "Alt"
#define DBUS_MENU_SHORTCUT_SHIFT "Shift"
#define DBUS_MENU_SHORTCUT_SUPER "Super"

#define DBUS_MENUMODEL_SHORTCUT_SHIFT "<Shift>"
#define DBUS_MENUMODEL_SHORTCUT_CONTROL "<Control>"
#define DBUS_MENUMODEL_SHORTCUT_ALT "<Alt>"
#define DBUS_MENUMODEL_SHORTCUT_SUPER "<Super>"

#define DBUS_MENU_DISABLED_ACTION "ls.disabled"
#define DBUS_MENU_DISABLED_SUBMENU "disabled-submenu"
#define DBUS_MENU_PROPERTY_ENABLED "enabled"
#define DBUS_MENU_PROPERTY_TOGGLE_STATE "toggle-state"
#define DBUS_MENU_ACTION_RADIO_SELECTED "+"
#define DBUS_MENU_ACTION_RADIO_UNSELECTED "-"

#define G_MENU_ATTRIBUTE_ACCEL "accel"
#define G_MENU_ATTRIBUTE_ACCEL_TEXT "x-canonical-accel"
#define G_MENU_ATTRIBUTE_SUBMENU_ACTION "submenu-action"
#define G_MENU_ATTRIBUTE_HIDDEN_WHEN "hidden-when"
#define G_MENU_ATTRIBUTE_VERB_ICON "verb-icon"
#define G_MENU_HIDDEN_WHEN_ACTION_MISSING "action-missing"
#define G_MENU_HIDDEN_WHEN_ACTION_DISABLED "action-disabled"

#define g_signal_handlers_block_by_func_only(instance, func)                                       \
	g_signal_handlers_block_matched((instance),                                                \
	                                (GSignalMatchType)(G_SIGNAL_MATCH_FUNC),                   \
	                                0,                                                         \
	                                0,                                                         \
	                                NULL,                                                      \
	                                (gpointer)(func),                                          \
	                                NULL)

#define g_signal_handlers_unblock_by_func_only(instance, func)                                     \
	g_signal_handlers_unblock_matched((instance),                                              \
	                                  (GSignalMatchType)(G_SIGNAL_MATCH_FUNC),                 \
	                                  0,                                                       \
	                                  0,                                                       \
	                                  NULL,                                                    \
	                                  (gpointer)(func),                                        \
	                                  NULL)

#define g_signal_handlers_block_by_data(instance, data)                                            \
	g_signal_handlers_block_matched((instance),                                                \
	                                (GSignalMatchType)(G_SIGNAL_MATCH_DATA),                   \
	                                0,                                                         \
	                                0,                                                         \
	                                NULL,                                                      \
	                                NULL,                                                      \
	                                (data))

#define g_signal_handlers_unblock_by_data(instance, data)                                          \
	g_signal_handlers_unblock_matched((instance),                                              \
	                                  (GSignalMatchType)(G_SIGNAL_MATCH_DATA),                 \
	                                  0,                                                       \
	                                  0,                                                       \
	                                  NULL,                                                    \
	                                  NULL,                                                    \
	                                  (data))

#define g_signal_handlers_disconnect_by_func_only(instance, func)                                  \
	g_signal_handlers_disconnect_matched((instance),                                           \
	                                     (GSignalMatchType)(G_SIGNAL_MATCH_FUNC),              \
	                                     0,                                                    \
	                                     0,                                                    \
	                                     NULL,                                                 \
	                                     (gpointer)(func),                                     \
	                                     NULL)

#endif // DEFINITIONS_H
