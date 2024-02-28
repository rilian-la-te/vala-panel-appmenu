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

#ifndef __UNITY_GTK_MENU_ITEM_PRIVATE_H__
#define __UNITY_GTK_MENU_ITEM_PRIVATE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define UNITY_GTK_TYPE_MENU_ITEM (unity_gtk_menu_item_get_type())
G_DECLARE_FINAL_TYPE(UnityGtkMenuItem, unity_gtk_menu_item, UNITY_GTK, MENU_ITEM, GObject)

G_END_DECLS

#include "unity-gtk-action-private.h"
#include "unity-gtk-menu-shell-private.h"

G_BEGIN_DECLS

struct _UnityGtkMenuItem
{
	GObject parent_instance;

	/*< private >*/
	GtkMenuItem *menu_item;
	UnityGtkMenuShell *parent_shell;
	UnityGtkMenuShell *child_shell;
	guchar child_shell_valid : 1;
	guint item_index;
	UnityGtkAction *action;
	GtkLabel *first_label;
	GtkLabel *second_label;
	char *label_label;
};

GType unity_gtk_menu_item_get_type(void) G_GNUC_INTERNAL;

UnityGtkMenuItem *unity_gtk_menu_item_new(GtkMenuItem *menu_item, UnityGtkMenuShell *parent_shell,
                                          guint item_index) G_GNUC_INTERNAL;

UnityGtkMenuShell *unity_gtk_menu_item_get_child_shell(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

void unity_gtk_menu_item_set_action(UnityGtkMenuItem *item, UnityGtkAction *action) G_GNUC_INTERNAL;

const char *unity_gtk_menu_item_get_label(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

GIcon *unity_gtk_menu_item_get_icon(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

gboolean unity_gtk_menu_item_is_visible(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

gboolean unity_gtk_menu_item_is_sensitive(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

gboolean unity_gtk_menu_item_is_active(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

gboolean unity_gtk_menu_item_is_separator(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

gboolean unity_gtk_menu_item_is_check(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

gboolean unity_gtk_menu_item_is_radio(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

gboolean unity_gtk_menu_item_get_draw_as_radio(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

void unity_gtk_menu_item_activate(UnityGtkMenuItem *item) G_GNUC_INTERNAL;

void unity_gtk_menu_item_print(UnityGtkMenuItem *item, guint indent) G_GNUC_INTERNAL;

GtkLabel *gtk_menu_item_get_nth_label(GtkMenuItem *menu_item, guint index) G_GNUC_INTERNAL;

const char *gtk_menu_item_get_nth_label_label(GtkMenuItem *menu_item, guint index) G_GNUC_INTERNAL;

G_END_DECLS

#endif /* __UNITY_GTK_MENU_ITEM_PRIVATE_H__ */
