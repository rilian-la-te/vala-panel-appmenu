#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define DBUS_MENU_ACTION_NAMESPACE "dbusmenu"
#define DBUS_MENU_ACTION_NAMESPACE_PREFIX DBUS_MENU_ACTION_NAMESPACE "."

#define ACTION_PREFIX "id-"
#define SUBMENU_PREFIX "submenu-"
#define CURRENT_TIME 0L
#define HAS_ICON_NAME "has-icon-name"

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

#define DBUS_MENU_DISABLED_ACTION "ls.disabled"
#define DBUS_MENU_ACTION_RADIO_SELECTED "+"
#define DBUS_MENU_ACTION_RADIO_UNSELECTED "-"

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

#endif // DEFINITIONS_H
