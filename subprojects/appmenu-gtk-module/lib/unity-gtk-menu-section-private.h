/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Ryan Lortie <desrt@desrt.ca>
 *          William Hua <william.hua@canonical.com>
 */

#ifndef __UNITY_GTK_MENU_SECTION_PRIVATE_H__
#define __UNITY_GTK_MENU_SECTION_PRIVATE_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define UNITY_GTK_TYPE_MENU_SECTION (unity_gtk_menu_section_get_type())
G_DECLARE_FINAL_TYPE(UnityGtkMenuSection, unity_gtk_menu_section, UNITY_GTK, MENU_SECTION, GMenuModel)

G_END_DECLS

#include "unity-gtk-menu-shell-private.h"

G_BEGIN_DECLS

struct _UnityGtkMenuSection
{
	GMenuModel parent_instance;

	/*< private >*/
	UnityGtkMenuShell *parent_shell;
	guint section_index;
};

GType unity_gtk_menu_section_get_type(void) G_GNUC_INTERNAL;

UnityGtkMenuSection *unity_gtk_menu_section_new(UnityGtkMenuShell *parent_shell,
                                                guint section_index) G_GNUC_INTERNAL;

GSequenceIter *unity_gtk_menu_section_get_begin_iter(UnityGtkMenuSection *section) G_GNUC_INTERNAL;

GSequenceIter *unity_gtk_menu_section_get_end_iter(UnityGtkMenuSection *section) G_GNUC_INTERNAL;

GSequenceIter *unity_gtk_menu_section_get_iter(UnityGtkMenuSection *section,
                                               guint index) G_GNUC_INTERNAL;

void unity_gtk_menu_section_print(UnityGtkMenuSection *section, guint indent) G_GNUC_INTERNAL;

G_END_DECLS

#endif /* __UNITY_GTK_MENU_SECTION_PRIVATE_H__ */
