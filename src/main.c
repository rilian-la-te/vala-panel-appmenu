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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "unity-gtk-menu-shell.h"
#include "unity-gtk-action-group.h"

#define WINDOW_OBJECT_PATH "/com/canonical/unity/gtk/window"

G_DEFINE_QUARK (window_data, window_data);

typedef struct _WindowData WindowData;

struct _WindowData
{
  guint                window_id;
  GSList              *menus;
  GMenu               *menu_model;
  guint                menu_model_export_id;
  UnityGtkActionGroup *action_group;
  guint                action_group_export_id;
};

static void (* pre_hijacked_window_realize)                          (GtkWidget     *widget);

static void (* pre_hijacked_window_unrealize)                        (GtkWidget     *widget);

static void (* pre_hijacked_application_window_realize)              (GtkWidget     *widget);

static void (* pre_hijacked_menu_bar_realize)                        (GtkWidget     *widget);

static void (* pre_hijacked_menu_bar_unrealize)                      (GtkWidget     *widget);

static void (* pre_hijacked_widget_size_allocate)                    (GtkWidget     *widget,
                                                                      GtkAllocation *allocation);

static void (* pre_hijacked_menu_bar_size_allocate)                  (GtkWidget     *widget,
                                                                      GtkAllocation *allocation);

static void (* pre_hijacked_menu_bar_get_preferred_width)            (GtkWidget     *widget,
                                                                      gint          *minimum_width,
                                                                      gint          *natural_width);

static void (* pre_hijacked_menu_bar_get_preferred_height)           (GtkWidget     *widget,
                                                                      gint          *minimum_height,
                                                                      gint          *natural_height);

static void (* pre_hijacked_menu_bar_get_preferred_width_for_height) (GtkWidget     *widget,
                                                                      gint           height,
                                                                      gint          *minimum_width,
                                                                      gint          *natural_width);

static void (* pre_hijacked_menu_bar_get_preferred_height_for_width) (GtkWidget     *widget,
                                                                      gint           width,
                                                                      gint          *minimum_height,
                                                                      gint          *natural_height);

static gboolean
gtk_widget_has_x11_property (GtkWidget   *widget,
                             const gchar *name)
{
  GdkWindow *window;
  GdkDisplay *display;
  Display *xdisplay;
  Window xwindow;
  Atom property;
  Atom actual_type;
  int actual_format;
  unsigned long nitems;
  unsigned long bytes_after;
  unsigned char *prop;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  window = gtk_widget_get_window (widget);
  display = gdk_window_get_display (window);
  xdisplay = GDK_DISPLAY_XDISPLAY (display);
  xwindow = GDK_WINDOW_XID (window);

  property = None;

  if (display != NULL)
    property = gdk_x11_get_xatom_by_name_for_display (display, name);

  if (property == None)
    property = gdk_x11_get_xatom_by_name (name);

  g_return_val_if_fail (property != None, FALSE);

  if (XGetWindowProperty (xdisplay,
                          xwindow,
                          property,
                          0,
                          G_MAXLONG,
                          False,
                          AnyPropertyType,
                          &actual_type,
                          &actual_format,
                          &nitems,
                          &bytes_after,
                          &prop) == Success)
    {
      if (actual_format)
        {
          if (prop != NULL)
            XFree (prop);

          return TRUE;
        }
      else
        return FALSE;
    }

  return FALSE;
}

static gchar *
gtk_widget_get_x11_property_string (GtkWidget   *widget,
                                    const gchar *name)
{
  GdkWindow *window;
  GdkDisplay *display;
  Display *xdisplay;
  Window xwindow;
  Atom property;
  Atom actual_type;
  int actual_format;
  unsigned long nitems;
  unsigned long bytes_after;
  unsigned char *prop;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  window = gtk_widget_get_window (widget);
  display = gdk_window_get_display (window);
  xdisplay = GDK_DISPLAY_XDISPLAY (display);
  xwindow = GDK_WINDOW_XID (window);

  property = None;

  if (display != NULL)
    property = gdk_x11_get_xatom_by_name_for_display (display, name);

  if (property == None)
    property = gdk_x11_get_xatom_by_name (name);

  g_return_val_if_fail (property != None, NULL);

  if (XGetWindowProperty (xdisplay,
                          xwindow,
                          property,
                          0,
                          G_MAXLONG,
                          False,
                          AnyPropertyType,
                          &actual_type,
                          &actual_format,
                          &nitems,
                          &bytes_after,
                          &prop) == Success)
    {
      if (actual_format)
        {
          gchar *string = g_strdup ((const gchar *) prop);

          if (prop != NULL)
            XFree (prop);

          return string;
        }
      else
        return NULL;
    }

  return NULL;
}

static WindowData *
window_data_new (void)
{
  return g_slice_new0 (WindowData);
}

static void
window_data_free (gpointer data)
{
  WindowData *window_data = data;

  if (window_data != NULL)
    {
      GDBusConnection *session = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

      if (window_data->action_group_export_id)
        g_dbus_connection_unexport_action_group (session, window_data->action_group_export_id);

      if (window_data->menu_model_export_id)
        g_dbus_connection_unexport_menu_model (session, window_data->menu_model_export_id);

      if (window_data->action_group != NULL)
        g_object_unref (window_data->action_group);

      if (window_data->menu_model != NULL)
        g_object_unref (window_data->menu_model);

      if (window_data->menus != NULL)
        {
          GSList *iter;

          for (iter = window_data->menus; iter != NULL; iter = g_slist_next (iter))
            g_object_run_dispose (iter->data);

          g_slist_free_full (window_data->menus, g_object_unref);
        }

      g_slice_free (WindowData, window_data);
    }
}

static WindowData *
window_get_window_data (GtkWidget *widget)
{
  WindowData *window_data;

  g_return_val_if_fail (GTK_IS_WINDOW (widget), NULL);

  window_data = g_object_get_qdata (G_OBJECT (widget), window_data_quark ());

  if (window_data == NULL)
    {
      static guint window_id;

      GDBusConnection *session = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
      GdkX11Window *window = GDK_X11_WINDOW (gtk_widget_get_window (widget));
      gchar *window_object_path = g_strdup_printf (WINDOW_OBJECT_PATH "/%d", window_id);
      gchar *old_unique_bus_name = gtk_widget_get_x11_property_string (widget, "_GTK_UNIQUE_BUS_NAME");
      gchar *old_window_object_path = gtk_widget_get_x11_property_string (widget, "_GTK_WINDOW_OBJECT_PATH");
      GDBusActionGroup *old_action_group = NULL;

      if (old_unique_bus_name != NULL && old_window_object_path != NULL)
        old_action_group = g_dbus_action_group_get (session, old_unique_bus_name, old_window_object_path);

      window_data = window_data_new ();
      window_data->window_id = window_id++;
      window_data->menu_model = g_menu_new ();
      window_data->action_group = unity_gtk_action_group_new (G_ACTION_GROUP (old_action_group));
      window_data->menu_model_export_id = g_dbus_connection_export_menu_model (session, window_object_path, G_MENU_MODEL (window_data->menu_model), NULL);
      window_data->action_group_export_id = g_dbus_connection_export_action_group (session, window_object_path, G_ACTION_GROUP (window_data->action_group), NULL);

      if (old_unique_bus_name == NULL)
        gdk_x11_window_set_utf8_property (window, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name (session));

      gdk_x11_window_set_utf8_property (window, "_GTK_WINDOW_OBJECT_PATH", window_object_path);

      if (!gtk_widget_has_x11_property (widget, "_GTK_MENUBAR_OBJECT_PATH"))
        gdk_x11_window_set_utf8_property (window, "_GTK_MENUBAR_OBJECT_PATH", window_object_path);

      g_object_set_qdata_full (G_OBJECT (widget), window_data_quark (), window_data, window_data_free);

      g_free (old_window_object_path);
      g_free (old_unique_bus_name);
      g_free (window_object_path);
    }

  return window_data;
}

static void
hijacked_window_realize (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WINDOW (widget));

  (* pre_hijacked_window_realize) (widget);

  if (!GTK_IS_APPLICATION_WINDOW (widget))
    window_get_window_data (widget);
}

static void
hijacked_window_unrealize (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WINDOW (widget));

  (* pre_hijacked_window_unrealize) (widget);

  g_object_set_qdata (G_OBJECT (widget), window_data_quark (), NULL);
}

static void
hijacked_application_window_realize (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_APPLICATION_WINDOW (widget));

  (* pre_hijacked_application_window_realize) (widget);

  window_get_window_data (widget);
}

static void
hijacked_menu_bar_realize (GtkWidget *widget)
{
  GtkWidget *window;
  WindowData *window_data;

  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  (* pre_hijacked_menu_bar_realize) (widget);

  window = gtk_widget_get_toplevel (widget);
  window_data = window_get_window_data (window);

  if (window_data != NULL)
    {
      GSList *iter;

      for (iter = window_data->menus; iter != NULL; iter = g_slist_next (iter))
        if (UNITY_GTK_MENU_SHELL (iter->data)->menu_shell == GTK_MENU_SHELL (widget))
          break;

      if (iter == NULL)
        {
          UnityGtkMenuShell *shell = unity_gtk_menu_shell_new (GTK_MENU_SHELL (widget));

          unity_gtk_action_group_connect_shell (window_data->action_group, shell);

          g_menu_append_section (window_data->menu_model, NULL, G_MENU_MODEL (shell));

          window_data->menus = g_slist_append (window_data->menus, shell);
        }
    }
}

static void
hijacked_menu_bar_unrealize (GtkWidget *widget)
{
  GtkWidget *window;
  WindowData *window_data;

  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  (* pre_hijacked_menu_bar_unrealize) (widget);

  window = gtk_widget_get_toplevel (widget);
  window_data = window_get_window_data (window);

  if (window_data != NULL)
    {
      GSList *iter;
      guint i = 0;

      for (iter = window_data->menus; iter != NULL; iter = g_slist_next (iter), i++)
        if (UNITY_GTK_MENU_SHELL (iter->data)->menu_shell == GTK_MENU_SHELL (widget))
          break;

      if (iter != NULL)
        {
          g_menu_remove (window_data->menu_model, i);

          unity_gtk_action_group_disconnect_shell (window_data->action_group, iter->data);
          g_object_run_dispose (iter->data);
          g_object_unref (iter->data);

          window_data->menus = g_slist_delete_link (window_data->menus, iter);
        }
    }
}

static void
hijacked_menu_bar_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  GtkAllocation zero = { 0, 0, 0, 0 };
  GdkWindow *window;

  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  /*
   * We manually assign an empty allocation to the menu bar to
   * prevent the container from attempting to draw it at all.
   */
  (* pre_hijacked_widget_size_allocate) (widget, &zero);

  /*
   * Then we move the GdkWindow belonging to the menu bar outside of
   * the clipping rectangle of the parent window so that we can't
   * see it.
   */
  window = gtk_widget_get_window (widget);

  if (window != NULL)
    gdk_window_move_resize (window, -1, -1, 1, 1);
}

static void
hijacked_menu_bar_get_preferred_width (GtkWidget *widget,
                                       gint      *minimum_width,
                                       gint      *natural_width)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  (* pre_hijacked_menu_bar_get_preferred_width) (widget, minimum_width, natural_width);

  *minimum_width = 0;
  *natural_width = 0;
}

static void
hijacked_menu_bar_get_preferred_height (GtkWidget *widget,
                                        gint      *minimum_height,
                                        gint      *natural_height)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  (* pre_hijacked_menu_bar_get_preferred_height) (widget, minimum_height, natural_height);

  *minimum_height = 0;
  *natural_height = 0;
}

static void
hijacked_menu_bar_get_preferred_width_for_height (GtkWidget *widget,
                                                  gint       height,
                                                  gint      *minimum_width,
                                                  gint      *natural_width)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  (* pre_hijacked_menu_bar_get_preferred_width_for_height) (widget, height, minimum_width, natural_width);

  *minimum_width = 0;
  *natural_width = 0;
}

static void
hijacked_menu_bar_get_preferred_height_for_width (GtkWidget *widget,
                                                  gint       width,
                                                  gint      *minimum_height,
                                                  gint      *natural_height)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  (* pre_hijacked_menu_bar_get_preferred_height_for_width) (widget, width, minimum_height, natural_height);

  *minimum_height = 0;
  *natural_height = 0;
}

static void
hijack_window_class_vtable (GType type)
{
  GtkWidgetClass *widget_class = g_type_class_ref (type);
  GType *children;
  guint n;
  guint i;

  if (widget_class->realize == pre_hijacked_window_realize)
    widget_class->realize = hijacked_window_realize;

  if (widget_class->realize == pre_hijacked_application_window_realize)
    widget_class->realize = hijacked_application_window_realize;

  if (widget_class->unrealize == pre_hijacked_window_unrealize)
    widget_class->unrealize = hijacked_window_unrealize;

  children = g_type_children (type, &n);

  for (i = 0; i < n; i++)
    hijack_window_class_vtable (children[i]);

  g_free (children);
}

static void
hijack_menu_bar_class_vtable (GType type)
{
  GtkWidgetClass *widget_class = g_type_class_ref (type);
  GType *children;
  guint n;
  guint i;

  if (widget_class->get_preferred_width == pre_hijacked_menu_bar_get_preferred_width)
    widget_class->get_preferred_width = hijacked_menu_bar_get_preferred_width;

  if (widget_class->get_preferred_height == pre_hijacked_menu_bar_get_preferred_height)
    widget_class->get_preferred_height = hijacked_menu_bar_get_preferred_height;

  if (widget_class->get_preferred_width_for_height == pre_hijacked_menu_bar_get_preferred_width_for_height)
    widget_class->get_preferred_width_for_height = hijacked_menu_bar_get_preferred_width_for_height;

  if (widget_class->get_preferred_height_for_width == pre_hijacked_menu_bar_get_preferred_height_for_width)
    widget_class->get_preferred_height_for_width = hijacked_menu_bar_get_preferred_height_for_width;

  if (widget_class->size_allocate == pre_hijacked_menu_bar_size_allocate)
    widget_class->size_allocate = hijacked_menu_bar_size_allocate;

  if (widget_class->realize == pre_hijacked_menu_bar_realize)
    widget_class->realize = hijacked_menu_bar_realize;

  if (widget_class->unrealize == pre_hijacked_menu_bar_unrealize)
    widget_class->unrealize = hijacked_menu_bar_unrealize;

  children = g_type_children (type, &n);

  for (i = 0; i < n; i++)
    hijack_menu_bar_class_vtable (children[i]);

  g_free (children);
}

void
gtk_module_init (void)
{
  GtkWidgetClass *widget_class;

  /* store the base GtkWidget size_allocate vfunc */
  widget_class = g_type_class_ref (GTK_TYPE_WIDGET);
  pre_hijacked_widget_size_allocate = widget_class->size_allocate;

  /* store the base GtkApplicationWindow realize vfunc */
  widget_class = g_type_class_ref (GTK_TYPE_APPLICATION_WINDOW);
  pre_hijacked_application_window_realize = widget_class->realize;

  /* intercept window realize vcalls on GtkWindow */
  widget_class = g_type_class_ref (GTK_TYPE_WINDOW);
  pre_hijacked_window_realize = widget_class->realize;
  pre_hijacked_window_unrealize = widget_class->unrealize;
  hijack_window_class_vtable (GTK_TYPE_WINDOW);

  /* intercept size request and allocate vcalls on GtkMenuBar (for hiding) */
  widget_class = g_type_class_ref (GTK_TYPE_MENU_BAR);
  pre_hijacked_menu_bar_realize = widget_class->realize;
  pre_hijacked_menu_bar_unrealize = widget_class->unrealize;
  pre_hijacked_menu_bar_size_allocate = widget_class->size_allocate;
  pre_hijacked_menu_bar_get_preferred_width = widget_class->get_preferred_width;
  pre_hijacked_menu_bar_get_preferred_height = widget_class->get_preferred_height;
  pre_hijacked_menu_bar_get_preferred_width_for_height = widget_class->get_preferred_width_for_height;
  pre_hijacked_menu_bar_get_preferred_height_for_width = widget_class->get_preferred_height_for_width;
  hijack_menu_bar_class_vtable (GTK_TYPE_MENU_BAR);
}
