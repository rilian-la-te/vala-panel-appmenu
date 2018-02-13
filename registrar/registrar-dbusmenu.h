#ifndef REGISTRARDBUSMENU_H
#define REGISTRARDBUSMENU_H

#include <gio/gio.h>

#define DBUSMENU_REG_IFACE "com.canonical.AppMenu.Registrar"
#define DBUSMENU_REG_OBJECT "/com/canonical/AppMenu/Registrar"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(RegistrarDBusMenu, registrar_dbus_menu, REGISTRAR, DBUS_MENU, GObject)
uint registrar_dbus_menu_register(RegistrarDBusMenu *object, GDBusConnection *connection,
                                  GError **error);

G_END_DECLS

#endif // REGISTRARDBUSMENU_H
