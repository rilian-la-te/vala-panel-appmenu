#ifndef SECTION_H
#define SECTION_H

#include "model.h"
#include <gio/gio.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(DBusMenuSectionModel, dbus_menu_section_model, DBUS_MENU, SECTION_MODEL,
                     GMenuModel)

struct _DBusMenuSectionModel
{
	GMenuModel parent_instance;

	DBusMenuModel *parent_model;
	GSequence *items;
	uint section_index;
};

DBusMenuSectionModel *dbus_menu_section_model_new(DBusMenuModel *parent, int section_index);

G_END_DECLS

#endif // SECTION_H
