// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exporter-section.h"
#include "gtkmenutracker.h"
#include "gtkmenutrackeritem.h"
#include "dbusmenu-interface.h"

/**
 * DBusMenuExporterSection:
 * @tracker: the #MenuTracker that owns the model side of this section.
 * @items:   a #GPtrArray of #DBusMenuExporterItem* owned by this section.
 *           The array preserves the order of the items as they appear in the
 *           menu model and allows O(1) access by position.
 * @parent:  the parent section, or %NULL for a top‑level section.
 * @xml:     DBusMenu interface for accessing dbus and answering requests
 *
 * A section groups a set of #DBusMenuExporterItem instances that belong to the
 * same #MenuTracker.  The items are kept in a #GPtrArray so that a position
 * lookup can be performed in constant time while still allowing efficient
 * appends and removals (the latter at the cost of shifting subsequent elements,
 * which is the usual trade‑off for an ordered array).
 */
struct _DBusMenuExporterSection
{
  GtkMenuTracker       *tracker;
  GPtrArray            *items;   /* element-type: DBusMenuExporterItem* */
  DBusMenuExporterSection *parent;
  DBusMenuXml* xml;
};
