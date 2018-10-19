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

#include "registrar-main.h"
#include "config.h"
#include "registrar-dbusmenu.h"
#include <glib/gi18n.h>
#include <stdbool.h>
#include <stdint.h>

#define MAIN_OBJECT_PATH "/Registrar"

struct _RegistrarApplication
{
	GApplication parent;
	RegistrarDBusMenu *registrar;
	u_int32_t dbusmenu_binding;
	u_int32_t private_binding;
};

extern const char *private_xml;

G_DEFINE_TYPE(RegistrarApplication, registrar_application, G_TYPE_APPLICATION)

static const GOptionEntry options[4] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, NULL, N_("Print version and exit"), NULL },
	{ "reference",
	  'r',
	  0,
	  G_OPTION_ARG_NONE,
	  NULL,
	  N_("Reference a registrar (need more unreferences to quit automatically)"),
	  NULL },
	{ "unreference",
	  'u',
	  0,
	  G_OPTION_ARG_NONE,
	  NULL,
	  N_("Unreference a registrar (need less unreferences to quit automatically, or quits if "
	     "refcount reaches zero)"),
	  NULL },
	{ NULL }
};

RegistrarApplication *registrar_application_new()
{
	return REGISTRAR_APPLICATION(
	    g_object_new(registrar_application_get_type(),
	                 "application-id",
	                 "org.valapanel.AppMenu.Registrar",
	                 "flags",
	                 G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_IS_SERVICE,
	                 "resource-base-path",
	                 "/org/valapanel/registrar",
	                 NULL));
}
static void registrar_application_activate(GApplication *base)
{
}
static int registrar_application_handle_local_options(GApplication *application,
                                                      GVariantDict *options)
{
	if (g_variant_dict_contains(options, "version"))
	{
		g_print(_("%s - Version %s\n"), g_get_application_name(), VERSION);
		return 0;
	}
	return -1;
}
static int registrar_application_command_line(GApplication *application,
                                              GApplicationCommandLine *commandline)
{
	GVariantDict *options = g_application_command_line_get_options_dict(commandline);
	if (g_variant_dict_contains(options, "reference"))
		g_application_hold(application);
	if (g_variant_dict_contains(options, "unreference"))
		g_application_release(application);
	return 0;
}
static void registrar_application_on_dbus_name_aquired(GDBusConnection *connection,
                                                       const char *name, gpointer user_data)
{
	RegistrarApplication *self = REGISTRAR_APPLICATION(user_data);
	g_autoptr(GError) err      = NULL;
	registrar_dbus_menu_register(self->registrar, connection, &err);
	if (err)
	{
		g_print("%s\n", err->message);
	}
}
static void registrar_application_on_dbus_name_lost(GDBusConnection *connection, const char *name,
                                                    gpointer user_data)
{
	GApplication *app = G_APPLICATION(user_data);
}

static void registrar_application_method_call(GDBusConnection *connection, const char *sender,
                                              const char *object_path, const char *interface_name,
                                              const char *method_name, GVariant *parameters,
                                              GDBusMethodInvocation *invocation, gpointer user_data)
{
	GApplication *app = G_APPLICATION(user_data);
	if (g_strcmp0(method_name, "Reference") == 0)
	{
		g_application_hold(app);
	}
	else if (g_strcmp0(method_name, "UnReference") == 0)
	{
		g_application_release(app);
	}
	else
	{
		g_object_unref(invocation);
	}
}
static const GDBusInterfaceVTable _interface_vtable = { registrar_application_method_call,
	                                                NULL,
	                                                NULL };

static int registrar_application_dbus_register(GApplication *base, GDBusConnection *connection,
                                               const char *object_path, GError **error)
{
	RegistrarApplication *self = REGISTRAR_APPLICATION(base);
	g_return_val_if_fail(connection != NULL, false);
	g_return_val_if_fail(object_path != NULL, false);
	bool ret = G_APPLICATION_CLASS(registrar_application_parent_class)
	               ->dbus_register(base, connection, object_path, error);
	if (*error != NULL)
		return false;
	self->dbusmenu_binding =
	    g_bus_own_name_on_connection(connection,
	                                 DBUSMENU_REG_IFACE,
	                                 G_BUS_NAME_OWNER_FLAGS_NONE,
	                                 registrar_application_on_dbus_name_aquired,
	                                 registrar_application_on_dbus_name_lost,
	                                 self,
	                                 NULL);
	GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(private_xml, NULL);
	self->private_binding =
	    g_dbus_connection_register_object(connection,
	                                      MAIN_OBJECT_PATH,
	                                      (GDBusInterfaceInfo *)info->interfaces[0],
	                                      &_interface_vtable,
	                                      self,
	                                      NULL,
	                                      error);

	return ret && self->dbusmenu_binding && self->private_binding;
}
static void registrar_application_dbus_unregister(GApplication *base, GDBusConnection *connection,
                                                  const char *object_path)
{
	RegistrarApplication *self = REGISTRAR_APPLICATION(base);
	g_return_if_fail(connection != NULL);
	g_return_if_fail(object_path != NULL);
	g_bus_unown_name(self->dbusmenu_binding);
	registrar_dbus_menu_unregister(self->registrar, connection);
	g_dbus_connection_unregister_object(connection, self->private_binding);
	self->dbusmenu_binding = 0;
	G_APPLICATION_CLASS(registrar_application_parent_class)
	    ->dbus_unregister(base, connection, object_path);
}
static void registrar_application_finalize(GObject *obj)
{
	RegistrarApplication *self = REGISTRAR_APPLICATION(obj);
	g_clear_object(&self->registrar);
}

static void registrar_application_init(RegistrarApplication *application)
{
	bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
	application->registrar =
	    REGISTRAR_DBUS_MENU(g_object_new(registrar_dbus_menu_get_type(), NULL));
	g_application_add_main_option_entries(G_APPLICATION(application), options);
}

static void registrar_application_class_init(RegistrarApplicationClass *klass)
{
	((GApplicationClass *)klass)->handle_local_options =
	    registrar_application_handle_local_options;
	((GApplicationClass *)klass)->command_line    = registrar_application_command_line;
	((GApplicationClass *)klass)->dbus_register   = registrar_application_dbus_register;
	((GApplicationClass *)klass)->activate        = registrar_application_activate;
	((GApplicationClass *)klass)->dbus_unregister = registrar_application_dbus_unregister;
	G_OBJECT_CLASS(klass)->finalize               = registrar_application_finalize;
}

int main(int argc, char *argv[])
{
	RegistrarApplication *app = registrar_application_new();
	return g_application_run(G_APPLICATION(app), argc, argv);
}
