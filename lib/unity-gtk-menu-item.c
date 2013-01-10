/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#include "unity-gtk-menu-item-private.h"
#include "unity-gtk-action-group-private.h"

G_DEFINE_TYPE (UnityGtkMenuItem,
               unity_gtk_menu_item,
               G_TYPE_OBJECT);

static void
unity_gtk_menu_item_handle_item_notify (GObject    *object,
                                        GParamSpec *pspec,
                                        gpointer    user_data)
{
  UnityGtkMenuItem *item;
  UnityGtkMenuShell *parent_shell;
  GObject *menu_item;

  g_return_if_fail (UNITY_GTK_IS_MENU_ITEM (user_data));

  item = UNITY_GTK_MENU_ITEM (user_data);
  parent_shell = item->parent_shell;
  menu_item = G_OBJECT (item->menu_item);

  g_return_if_fail (parent_shell != NULL);
  g_warn_if_fail (object == menu_item);

  unity_gtk_menu_shell_handle_item_notify (parent_shell, item, pspec);
}

static void
unity_gtk_menu_item_set_menu_item (UnityGtkMenuItem *item,
                                   GtkMenuItem      *menu_item)
{
  g_return_if_fail (UNITY_GTK_IS_MENU_ITEM (item));

  if (menu_item != item->menu_item)
    {
      UnityGtkMenuShell *child_shell = item->child_shell;

      if (item->menu_item_notify_handler_id)
        {
          g_warn_if_fail (item->menu_item != NULL);
          g_signal_handler_disconnect (item->menu_item, item->menu_item_notify_handler_id);
          item->menu_item_notify_handler_id = 0;
        }

      if (child_shell != NULL)
        {
          g_warn_if_fail (item->child_shell_valid);
          item->child_shell = NULL;
          g_object_unref (child_shell);
        }

      item->child_shell_valid = FALSE;
      item->menu_item = menu_item;

      if (menu_item != NULL)
        item->menu_item_notify_handler_id = g_signal_connect (menu_item, "notify", G_CALLBACK (unity_gtk_menu_item_handle_item_notify), item);
    }
}

static void
unity_gtk_menu_item_set_parent_shell (UnityGtkMenuItem  *item,
                                      UnityGtkMenuShell *parent_shell)
{
  g_return_if_fail (UNITY_GTK_IS_MENU_ITEM (item));

  item->parent_shell = parent_shell;
}

static void
unity_gtk_menu_item_dispose (GObject *object)
{
  UnityGtkMenuItem *item;

  g_return_if_fail (UNITY_GTK_IS_MENU_ITEM (object));

  item = UNITY_GTK_MENU_ITEM (object);

  unity_gtk_menu_item_set_action (item, NULL);
  unity_gtk_menu_item_set_parent_shell (item, NULL);
  unity_gtk_menu_item_set_menu_item (item, NULL);

  G_OBJECT_CLASS (unity_gtk_menu_item_parent_class)->dispose (object);
}

static void
unity_gtk_menu_item_class_init (UnityGtkMenuItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = unity_gtk_menu_item_dispose;
}

static void
unity_gtk_menu_item_init (UnityGtkMenuItem *self)
{
}

UnityGtkMenuItem *
unity_gtk_menu_item_new (GtkMenuItem       *menu_item,
                         UnityGtkMenuShell *parent_shell,
                         guint              item_index)
{
  UnityGtkMenuItem *item = g_object_new (UNITY_GTK_TYPE_MENU_ITEM, NULL);

  unity_gtk_menu_item_set_menu_item (item, menu_item);
  unity_gtk_menu_item_set_parent_shell (item, parent_shell);
  item->item_index = item_index;

  return item;
}

UnityGtkMenuShell *
unity_gtk_menu_item_get_child_shell (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), NULL);

  if (!item->child_shell_valid)
    {
      GtkMenuItem *menu_item = item->menu_item;
      UnityGtkMenuShell *child_shell = item->child_shell;

      if (child_shell != NULL)
        {
          g_warn_if_reached ();
          item->child_shell = NULL;
          g_object_unref (child_shell);
        }

      if (menu_item != NULL)
        {
          GtkWidget *submenu = gtk_menu_item_get_submenu (menu_item);

          if (submenu != NULL)
            item->child_shell = unity_gtk_menu_shell_new (GTK_MENU_SHELL (submenu));
        }

      item->child_shell_valid = TRUE;

      if (unity_gtk_menu_item_is_visible (item) && item->child_shell != NULL)
        {
          UnityGtkMenuShell *parent_shell = item->parent_shell;

          if (parent_shell != NULL && parent_shell->action_group != NULL)
            unity_gtk_action_group_connect_shell (parent_shell->action_group, item->child_shell);
        }
    }

  return item->child_shell;
}

void
unity_gtk_menu_item_set_action (UnityGtkMenuItem *item,
                                UnityGtkAction   *action)
{
  UnityGtkAction *old_action;

  g_return_if_fail (UNITY_GTK_IS_MENU_ITEM (item));

  old_action = item->action;

  if (action != old_action)
    {
      if (old_action != NULL)
        {
          item->action = NULL;
          g_object_unref (old_action);
        }

      if (action != NULL)
        item->action = g_object_ref (action);
    }
}

const gchar *
unity_gtk_menu_item_get_label (UnityGtkMenuItem *item)
{
  const gchar *label;

  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), NULL);
  g_return_val_if_fail (item->menu_item != NULL, NULL);

  label = gtk_menu_item_get_label (item->menu_item);

  return label != NULL && label[0] != '\0' ? label : NULL;
}

gboolean
unity_gtk_menu_item_is_visible (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), FALSE);

  return item->menu_item != NULL && gtk_widget_get_visible (GTK_WIDGET (item->menu_item)) && !GTK_IS_TEAROFF_MENU_ITEM (item->menu_item);
}

gboolean
unity_gtk_menu_item_is_sensitive (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), FALSE);

  return item->menu_item != NULL && gtk_widget_is_sensitive (GTK_WIDGET (item->menu_item));
}

gboolean
unity_gtk_menu_item_is_active (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), FALSE);

  return GTK_IS_CHECK_MENU_ITEM (item->menu_item) && gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item->menu_item));
}

gboolean
unity_gtk_menu_item_is_separator (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), FALSE);

  return GTK_IS_SEPARATOR_MENU_ITEM (item->menu_item);
}

gboolean
unity_gtk_menu_item_is_check (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), FALSE);

  return GTK_IS_CHECK_MENU_ITEM (item->menu_item);
}

gboolean
unity_gtk_menu_item_is_radio (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), FALSE);

  return GTK_IS_RADIO_MENU_ITEM (item->menu_item);
}

gboolean
unity_gtk_menu_item_get_draw_as_radio (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_ITEM (item), FALSE);

  return GTK_IS_CHECK_MENU_ITEM (item->menu_item) && gtk_check_menu_item_get_draw_as_radio (GTK_CHECK_MENU_ITEM (item->menu_item));
}

void
unity_gtk_menu_item_print (UnityGtkMenuItem *item,
                           guint             indent)
{
  gchar *space;

  g_return_if_fail (item == NULL || UNITY_GTK_IS_MENU_ITEM (item));

  space = g_strnfill (indent, ' ');

  if (item != NULL)
    {
      g_print ("%s%u (%s *) %p\n", space, item->item_index, G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (item)), item);

      if (item->menu_item != NULL)
        g_print ("%s  %lu (%s *) %p\n", space, item->menu_item_notify_handler_id, G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (item->menu_item)), item->menu_item);
      else if (item->menu_item_notify_handler_id)
        g_print ("%s  %lu\n", space, item->menu_item_notify_handler_id);

      if (item->parent_shell != NULL)
        g_print ("%s  (%s *) %p\n", space, G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (item->parent_shell)), item->parent_shell);

      if (item->child_shell_valid || item->child_shell != NULL)
        {
          if (!item->child_shell_valid)
            g_print ("%s  invalid\n", space);

          unity_gtk_menu_shell_print (item->child_shell, indent + 2);
        }

      if (item->action != NULL)
        unity_gtk_action_print (item->action, indent + 2);
    }
  else
    g_print ("%sNULL\n", space);

  g_free (space);
}
