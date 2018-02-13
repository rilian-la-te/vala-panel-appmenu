#include "registrar-main.h"
#include "config.h"
#include "registrar-dbusmenu.h"
#include <glib/gi18n.h>
#include <stdbool.h>
#include <stdint.h>

struct _RegistrarApplication
{
	GApplication parent;
	GObject *registrar;
	u_int32_t dbusmenu_binding;
};

G_DEFINE_TYPE(RegistrarApplication, registrar_application, G_TYPE_APPLICATION)

static const GOptionEntry options[4] =
    { { "version", 'v', 0, G_OPTION_ARG_NONE, NULL, N_("Print version and exit"), NULL },
      { "reference",'r', 0, G_OPTION_ARG_NONE, NULL, N_("Reference a registrar (need more unreferences to quit automatically)"), NULL },
      { "unreference", 'u', 0, G_OPTION_ARG_NONE, NULL, N_("Unreference a registrar (need less unreferences to quit automatically, or quits if refcount reaches zero)"), NULL },
      { NULL } };

RegistrarApplication* registrar_application_new ()
{
    return REGISTRAR_APPLICATION(
                g_object_new (registrar_application_get_type(),
                         "application-id",
                         "org.valapanel.AppMenu.Registrar",
                         "flags",
                         G_APPLICATION_HANDLES_COMMAND_LINE,
                         "resource-base-path",
                         "/org/valapanel/registrar", NULL));
}
static void registrar_application_activate(GApplication *base)
{
	RegistrarApplication *self = REGISTRAR_APPLICATION(base);
	if (self->dbusmenu_binding > 0)
		g_application_hold(base);
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
	//    		appmenu_dbus_menu_registrar_register_object (_tmp1_, _tmp0_,
	//    APPMENU_DBUSMENU_REG_OBJECT, &_inner_error_);
	g_application_hold(G_APPLICATION(user_data));
}
static void registrar_application_on_dbus_name_lost(GDBusConnection *connection, const char *name,
                                                    gpointer user_data)
{
	GApplication *app = G_APPLICATION(user_data);
	g_application_release(app);
}
static bool registrar_application_dbus_register(GApplication *base, GDBusConnection *connection,
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
	return ret && self->dbusmenu_binding;
}
static void registrar_application_dbus_unregister(GApplication *base, GDBusConnection *connection,
                                                  const char *object_path)
{
	RegistrarApplication *self = REGISTRAR_APPLICATION(base);
	g_return_if_fail(connection != NULL);
	g_return_if_fail(object_path != NULL);
	g_bus_unown_name(self->dbusmenu_binding);
	self->dbusmenu_binding = 0;
	g_application_release(base);
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

#if 0
int main(int argc, char *argv[])
{
    RegistrarApplication *app = registrar_application_new();
    return g_application_run(G_APPLICATION(app), argc, argv);
}
#endif
