// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exporter-item.h"
#include "gtkmenutrackeritem.h"
#include "dbusmenu-interface.h"

struct _DBusMenuExporterItem
{
  GObject parent_instance;

  GtkMenuTrackerItem      *tracker_item;
  DBusMenuExporterSection *section;
};
