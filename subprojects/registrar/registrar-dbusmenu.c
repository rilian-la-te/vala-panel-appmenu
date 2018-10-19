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

#include "registrar-dbusmenu.h"
#include "registrar-marshal.h"
#include <stdbool.h>

extern const char *introspection_xml;

typedef struct
{
	char *bus_name;
	char *object_path;
} DBusAddress;

DBusAddress *dbus_address_new(const char *bus_name, const char *object_path)
{
	DBusAddress *ret = (DBusAddress *)g_slice_alloc0(sizeof(DBusAddress));
	ret->bus_name    = g_strdup(bus_name);
	ret->object_path = g_strdup(object_path);
	return ret;
}

DBusAddress *dbus_address_copy(const DBusAddress *src)
{
	DBusAddress *ret = (DBusAddress *)g_slice_new(DBusAddress);
	ret->bus_name    = g_strdup(src->bus_name);
	ret->object_path = g_strdup(src->object_path);
	return ret;
}

void dbus_address_free(void *obj)
{
	DBusAddress *addr = (DBusAddress *)obj;
	g_free(addr->bus_name);
	g_free(addr->object_path);
	g_slice_free1(sizeof(DBusAddress), addr);
}

G_DEFINE_BOXED_TYPE(DBusAddress, dbus_address, dbus_address_copy, dbus_address_free)

struct _RegistrarDBusMenu
{
	GObject parent;
	GHashTable *menus;
	uint registered_object;
};

G_DEFINE_TYPE(RegistrarDBusMenu, registrar_dbus_menu, G_TYPE_OBJECT)

enum
{
	WINDOW_REGISTERED_SIGNAL,
	WINDOW_UNREGISTERED_SIGNAL,
	NUM_SIGNALS
};
static uint registrar_dbus_menu_signals[NUM_SIGNALS] = { 0 };

void registrar_dbus_menu_register_window(RegistrarDBusMenu *self, uint window_id,
                                         const char *menu_object_path, const char *sender)
{
	g_return_if_fail(self != NULL);
	g_return_if_fail(menu_object_path != NULL);
	g_return_if_fail(sender != NULL);
	DBusAddress *addr = dbus_address_new(sender, menu_object_path);
	g_hash_table_insert(self->menus, GUINT_TO_POINTER(window_id), addr);
	g_signal_emit(self,
	              registrar_dbus_menu_signals[WINDOW_REGISTERED_SIGNAL],
	              0,
	              window_id,
	              sender,
	              menu_object_path);
}

void registrar_dbus_menu_unregister_window(RegistrarDBusMenu *self, uint window_id)
{
	g_return_if_fail(self != NULL);
	g_hash_table_remove(self->menus, GUINT_TO_POINTER(window_id));
	g_signal_emit(self, registrar_dbus_menu_signals[WINDOW_UNREGISTERED_SIGNAL], 0, window_id);
}

void registrar_dbus_menu_get_menu_for_window(RegistrarDBusMenu *self, uint window_id,
                                             char **service, char **object_path)
{
	DBusAddress *addr =
	    (DBusAddress *)g_hash_table_lookup(self->menus, GUINT_TO_POINTER(window_id));
	if (addr)
	{
		*service     = addr->bus_name;
		*object_path = addr->object_path;
	}
	else
	{
		*service     = "";
		*object_path = "/";
	}
	//            if (menu != null && !DBusMenu.GtkClient.check(service,path))
	//            {
	//                service = "";
	//                path = new ObjectPath("/");
	//                unregister_window(window);
	//            }
}

void registrar_dbus_menu_get_menus(RegistrarDBusMenu *self, GVariant **menus)
{
	GVariantBuilder bldr;
	GHashTableIter iter;
	gpointer key, value;

	g_variant_builder_init(&bldr, (const GVariantType *)"a(uso)");
	g_hash_table_iter_init(&iter, self->menus);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		DBusAddress *val = (DBusAddress *)value;
		char *name;
		char *path;
		registrar_dbus_menu_get_menu_for_window(self, GPOINTER_TO_UINT(key), &name, &path);
		g_variant_builder_add(&bldr, "(uso)", GPOINTER_TO_UINT(key), name, path);
	}
	*menus = g_variant_builder_end(&bldr);
}

static void registrar_dbus_menu_init(RegistrarDBusMenu *self)
{
	self->menus = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, dbus_address_free);
}

static void registrar_dbus_menu_finalize(GObject *obj)
{
	RegistrarDBusMenu *self = REGISTRAR_DBUS_MENU(obj);
	g_hash_table_unref(self->menus);
	G_OBJECT_CLASS(registrar_dbus_menu_parent_class)->finalize(obj);
}

static void registrar_dbus_menu_class_init(RegistrarDBusMenuClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = registrar_dbus_menu_finalize;
	registrar_dbus_menu_signals[WINDOW_REGISTERED_SIGNAL] =
	    g_signal_new("window-registered",
	                 registrar_dbus_menu_get_type(),
	                 G_SIGNAL_RUN_LAST,
	                 0,
	                 NULL,
	                 NULL,
	                 g_cclosure_user_marshal_VOID__UINT_STRING_STRING,
	                 G_TYPE_NONE,
	                 3,
	                 G_TYPE_UINT,
	                 G_TYPE_STRING,
	                 G_TYPE_STRING);
	registrar_dbus_menu_signals[WINDOW_UNREGISTERED_SIGNAL] =
	    g_signal_new("window-unregistered",
	                 registrar_dbus_menu_get_type(),
	                 G_SIGNAL_RUN_LAST,
	                 0,
	                 NULL,
	                 NULL,
	                 g_cclosure_marshal_VOID__UINT,
	                 G_TYPE_NONE,
	                 1,
	                 G_TYPE_UINT);
}

static void _dbus_registrar_dbus_menu_register_window(RegistrarDBusMenu *self,
                                                      GVariant *_parameters_,
                                                      GDBusMethodInvocation *invocation)
{
	g_autoptr(GError) error = NULL;
	GVariantIter _arguments_iter;
	uint window_id               = 0U;
	char *menu_object_path       = NULL;
	GDBusMessage *_reply_message = NULL;
	GVariant *_reply;
	GVariantBuilder _reply_builder;
	g_variant_iter_init(&_arguments_iter, _parameters_);
	GVariant *value = g_variant_iter_next_value(&_arguments_iter);
	window_id       = g_variant_get_uint32(value);
	g_variant_unref(value);
	value            = g_variant_iter_next_value(&_arguments_iter);
	menu_object_path = g_variant_dup_string(value, NULL);
	g_variant_unref(value);
	registrar_dbus_menu_register_window(self,
	                                    window_id,
	                                    menu_object_path,
	                                    g_dbus_method_invocation_get_sender(invocation));
	_reply_message =
	    g_dbus_message_new_method_reply(g_dbus_method_invocation_get_message(invocation));
	g_variant_builder_init(&_reply_builder, G_VARIANT_TYPE_TUPLE);
	_reply = g_variant_builder_end(&_reply_builder);
	g_dbus_message_set_body(_reply_message, _reply);
	g_dbus_connection_send_message(g_dbus_method_invocation_get_connection(invocation),
	                               _reply_message,
	                               G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                               NULL,
	                               NULL);
	g_object_unref(invocation);
	g_object_unref(_reply_message);
	g_free(menu_object_path);
}

static void _dbus_registrar_dbus_menu_unregister_window(RegistrarDBusMenu *self,
                                                        GVariant *_parameters_,
                                                        GDBusMethodInvocation *invocation)
{
	g_autoptr(GError) error = NULL;
	GVariantIter _arguments_iter;
	uint window_id               = 0U;
	GDBusMessage *_reply_message = NULL;
	GVariant *_reply;
	GVariantBuilder _reply_builder;
	g_variant_iter_init(&_arguments_iter, _parameters_);
	GVariant *value = g_variant_iter_next_value(&_arguments_iter);
	window_id       = g_variant_get_uint32(value);
	g_variant_unref(value);
	registrar_dbus_menu_unregister_window(self, window_id);
	_reply_message =
	    g_dbus_message_new_method_reply(g_dbus_method_invocation_get_message(invocation));
	g_variant_builder_init(&_reply_builder, G_VARIANT_TYPE_TUPLE);
	_reply = g_variant_builder_end(&_reply_builder);
	g_dbus_message_set_body(_reply_message, _reply);
	g_dbus_connection_send_message(g_dbus_method_invocation_get_connection(invocation),
	                               _reply_message,
	                               G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                               NULL,
	                               NULL);
	g_object_unref(invocation);
	g_object_unref(_reply_message);
}

static void _dbus_registrar_dbus_menu_get_menu_for_window(RegistrarDBusMenu *self,
                                                          GVariant *_parameters_,
                                                          GDBusMethodInvocation *invocation)
{
	GError *error = NULL;
	GVariantIter _arguments_iter;
	uint window = 0U;
	GVariant *_tmp3_;
	GDBusMessage *_reply_message = NULL;
	GVariant *_reply;
	GVariantBuilder _reply_builder;
	char *service = NULL;
	char *path    = NULL;
	g_variant_iter_init(&_arguments_iter, _parameters_);
	GVariant *value = g_variant_iter_next_value(&_arguments_iter);
	window          = g_variant_get_uint32(value);
	g_variant_unref(value);
	registrar_dbus_menu_get_menu_for_window(self, window, &service, &path);
	_reply_message =
	    g_dbus_message_new_method_reply(g_dbus_method_invocation_get_message(invocation));
	g_variant_builder_init(&_reply_builder, G_VARIANT_TYPE_TUPLE);
	g_variant_builder_add_value(&_reply_builder, g_variant_new_string(service));
	g_variant_builder_add_value(&_reply_builder, g_variant_new_object_path(path));
	_reply = g_variant_builder_end(&_reply_builder);
	g_dbus_message_set_body(_reply_message, _reply);
	g_dbus_connection_send_message(g_dbus_method_invocation_get_connection(invocation),
	                               _reply_message,
	                               G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                               NULL,
	                               NULL);
	g_object_unref(invocation);
	g_object_unref(_reply_message);
}

static void _dbus_registrar_dbus_menu_get_menus(RegistrarDBusMenu *self, GVariant *_parameters_,
                                                GDBusMethodInvocation *invocation)
{
	GError *error = NULL;
	GVariantIter _arguments_iter;
	GDBusMessage *_reply_message = NULL;
	GVariant *_reply;
	GVariantBuilder _reply_builder;
	GVariant *menus = NULL;
	g_variant_iter_init(&_arguments_iter, _parameters_);
	registrar_dbus_menu_get_menus(self, &menus);
	_reply_message =
	    g_dbus_message_new_method_reply(g_dbus_method_invocation_get_message(invocation));
	g_variant_builder_init(&_reply_builder, G_VARIANT_TYPE_TUPLE);
	g_variant_builder_add_value(&_reply_builder, menus);
	_reply = g_variant_builder_end(&_reply_builder);
	g_dbus_message_set_body(_reply_message, _reply);
	g_dbus_connection_send_message(g_dbus_method_invocation_get_connection(invocation),
	                               _reply_message,
	                               G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                               NULL,
	                               NULL);
	g_object_unref(invocation);
	g_object_unref(_reply_message);
	g_variant_unref(menus);
}

static void registrar_dbus_menu_dbus_interface_method_call(
    GDBusConnection *connection, const char *sender, const char *object_path,
    const char *interface_name, const char *method_name, GVariant *parameters,
    GDBusMethodInvocation *invocation, gpointer user_data)
{
	RegistrarDBusMenu *object = REGISTRAR_DBUS_MENU(user_data);
	if (g_strcmp0(method_name, "RegisterWindow") == 0)
	{
		_dbus_registrar_dbus_menu_register_window(object, parameters, invocation);
	}
	else if (g_strcmp0(method_name, "UnregisterWindow") == 0)
	{
		_dbus_registrar_dbus_menu_unregister_window(object, parameters, invocation);
	}
	else if (g_strcmp0(method_name, "GetMenuForWindow") == 0)
	{
		_dbus_registrar_dbus_menu_get_menu_for_window(object, parameters, invocation);
	}
	else if (g_strcmp0(method_name, "GetMenus") == 0)
	{
		_dbus_registrar_dbus_menu_get_menus(object, parameters, invocation);
	}
	else
	{
		g_object_unref(invocation);
	}
}

static void _dbus_registrar_dbus_menu_window_registered(GObject *_sender, uint window_id,
                                                        const char *service, const char *path,
                                                        gpointer *_data)
{
	GDBusConnection *_connection;
	GVariant *_arguments;
	GVariantBuilder _arguments_builder;
	_connection = (GDBusConnection *)_data;
	g_variant_builder_init(&_arguments_builder, G_VARIANT_TYPE_TUPLE);
	g_variant_builder_add_value(&_arguments_builder, g_variant_new_uint32(window_id));
	g_variant_builder_add_value(&_arguments_builder, g_variant_new_string(service));
	g_variant_builder_add_value(&_arguments_builder, g_variant_new_object_path(path));
	_arguments = g_variant_builder_end(&_arguments_builder);
	g_dbus_connection_emit_signal(_connection,
	                              NULL,
	                              DBUSMENU_REG_OBJECT,
	                              DBUSMENU_REG_IFACE,
	                              "WindowRegistered",
	                              _arguments,
	                              NULL);
}

static void _dbus_registrar_dbus_menu_window_unregistered(GObject *_sender, uint window_id,
                                                          gpointer *_data)
{
	GDBusConnection *_connection;
	const char *_path;
	GVariant *_arguments;
	GVariantBuilder _arguments_builder;
	_connection = (GDBusConnection *)_data;
	g_variant_builder_init(&_arguments_builder, G_VARIANT_TYPE_TUPLE);
	g_variant_builder_add_value(&_arguments_builder, g_variant_new_uint32(window_id));
	_arguments = g_variant_builder_end(&_arguments_builder);
	g_dbus_connection_emit_signal(_connection,
	                              NULL,
	                              DBUSMENU_REG_OBJECT,
	                              DBUSMENU_REG_IFACE,
	                              "WindowUnregistered",
	                              _arguments,
	                              NULL);
}
static const GDBusInterfaceVTable _interface_vtable = {
	registrar_dbus_menu_dbus_interface_method_call, NULL, NULL
};

void registrar_dbus_menu_unregister(RegistrarDBusMenu *data, GDBusConnection *con)
{
	g_dbus_connection_unregister_object(con, data->registered_object);
	g_signal_handlers_disconnect_by_func(data,
	                                     _dbus_registrar_dbus_menu_window_registered,
	                                     con);
	g_signal_handlers_disconnect_by_func(data,
	                                     _dbus_registrar_dbus_menu_window_unregistered,
	                                     con);
	g_object_unref(data);
}

uint registrar_dbus_menu_register(RegistrarDBusMenu *object, GDBusConnection *connection,
                                  GError **error)
{
	GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
	uint result         = g_dbus_connection_register_object(connection,
                                                        DBUSMENU_REG_OBJECT,
                                                        (GDBusInterfaceInfo *)info->interfaces[0],
                                                        &_interface_vtable,
                                                        object,
                                                        NULL,
                                                        error);
	if (!result)
	{
		return 0;
	}
	object->registered_object = result;
	g_signal_connect(object,
	                 "window-registered",
	                 (GCallback)_dbus_registrar_dbus_menu_window_registered,
	                 connection);
	g_signal_connect(object,
	                 "window-unregistered",
	                 (GCallback)_dbus_registrar_dbus_menu_window_unregistered,
	                 connection);
	return result;
}
