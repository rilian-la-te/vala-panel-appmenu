#ifndef IMPORTER_H
#define IMPORTER_H

#include <gio/gio.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuImporter,dbus_menu_importer,DBusMenu,Importer,GObject)

DBusMenuImporter *dbus_menu_importer_new(const gchar *bus_name, const gchar *object_path);

G_END_DECLS

#endif
