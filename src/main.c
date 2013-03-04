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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "unity-gtk-parser.h"

#define _GTK_UNIQUE_BUS_NAME     "_GTK_UNIQUE_BUS_NAME"
#define _UNITY_OBJECT_PATH       "_UNITY_OBJECT_PATH"
#define _GTK_MENUBAR_OBJECT_PATH "_GTK_MENUBAR_OBJECT_PATH"
#define OBJECT_PATH              "/com/canonical/unity/gtk/window"

G_DEFINE_QUARK (window_data, window_data);
G_DEFINE_QUARK (menu_shell_data, menu_shell_data);

typedef struct _WindowData WindowData;
typedef struct _MenuShellData MenuShellData;

struct _WindowData
{
  guint                window_id;
  GMenu               *menu_model;
  guint                menu_model_export_id;
  GSList              *menus;
  GMenuModel          *old_model;
  UnityGtkActionGroup *action_group;
  guint                action_group_export_id;
};

struct _MenuShellData
{
  GtkWindow *window;
  gulong     settings_notify_gtk_shell_shows_menubar_handler_id;
};

static void (* pre_hijacked_window_realize)                          (GtkWidget      *widget);

static void (* pre_hijacked_window_unrealize)                        (GtkWidget      *widget);

#if GTK_MAJOR_VERSION == 3
static void (* pre_hijacked_application_window_realize)              (GtkWidget      *widget);
#endif

static void (* pre_hijacked_menu_bar_realize)                        (GtkWidget      *widget);

static void (* pre_hijacked_menu_bar_unrealize)                      (GtkWidget      *widget);

static void (* pre_hijacked_widget_size_allocate)                    (GtkWidget      *widget,
                                                                      GtkAllocation  *allocation);

static void (* pre_hijacked_menu_bar_size_allocate)                  (GtkWidget      *widget,
                                                                      GtkAllocation  *allocation);

#if GTK_MAJOR_VERSION == 2
static void (* pre_hijacked_menu_bar_size_request)                   (GtkWidget      *widget,
                                                                      GtkRequisition *requisition);
#elif GTK_MAJOR_VERSION == 3
static void (* pre_hijacked_menu_bar_get_preferred_width)            (GtkWidget      *widget,
                                                                      gint           *minimum_width,
                                                                      gint           *natural_width);

static void (* pre_hijacked_menu_bar_get_preferred_height)           (GtkWidget      *widget,
                                                                      gint           *minimum_height,
                                                                      gint           *natural_height);

static void (* pre_hijacked_menu_bar_get_preferred_width_for_height) (GtkWidget      *widget,
                                                                      gint            height,
                                                                      gint           *minimum_width,
                                                                      gint           *natural_width);

static void (* pre_hijacked_menu_bar_get_preferred_height_for_width) (GtkWidget      *widget,
                                                                      gint            width,
                                                                      gint           *minimum_height,
                                                                      gint           *natural_height);
#endif

static gboolean
is_blacklisted (const gchar *name)
{
  return g_strcmp0 (name, "Eclipse") == 0 ||
         g_strcmp0 (name, "emacs") == 0 ||
         g_strcmp0 (name, "emacs23") == 0 ||
         g_strcmp0 (name, "emacs23-lucid") == 0 ||
         g_strcmp0 (name, "emacs24") == 0 ||
         g_strcmp0 (name, "emacs24-lucid") == 0 ||
         g_strcmp0 (name, "glade") == 0;
}

static gboolean
is_true (const gchar *value)
{
  return value != NULL &&
         value[0] != '\0' &&
         g_ascii_strcasecmp (value, "0") != 0 &&
         g_ascii_strcasecmp (value, "no") != 0 &&
         g_ascii_strcasecmp (value, "off") != 0 &&
         g_ascii_strcasecmp (value, "false") != 0;
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

static void
gtk_widget_set_x11_property_string (GtkWidget   *widget,
                                    const gchar *name,
                                    const gchar *value)
{
  GdkWindow *window;
  GdkDisplay *display;
  Display *xdisplay;
  Window xwindow;
  Atom property;
  Atom type;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  window = gtk_widget_get_window (widget);
  display = gdk_window_get_display (window);
  xdisplay = GDK_DISPLAY_XDISPLAY (display);
  xwindow = GDK_WINDOW_XID (window);

  property = None;

  if (display != NULL)
    property = gdk_x11_get_xatom_by_name_for_display (display, name);

  if (property == None)
    property = gdk_x11_get_xatom_by_name (name);

  g_return_if_fail (property != None);

  type = None;

  if (display != NULL)
    type = gdk_x11_get_xatom_by_name_for_display (display, "UTF8_STRING");

  if (type == None)
    type = gdk_x11_get_xatom_by_name ("UTF8_STRING");

  g_return_if_fail (type != None);

  if (value != NULL)
    XChangeProperty (xdisplay,
                     xwindow,
                     property,
                     type,
                     8,
                     PropModeReplace,
                     (unsigned char *) value,
                     g_utf8_strlen (value, -1));
  else
    XDeleteProperty (xdisplay,
                     xwindow,
                     property);
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

      if (window_data->old_model != NULL)
        g_object_unref (window_data->old_model);

      if (window_data->menus != NULL)
        g_slist_free_full (window_data->menus, g_object_unref);

      g_slice_free (WindowData, window_data);
    }
}

static MenuShellData *
menu_shell_data_new (void)
{
  return g_slice_new0 (MenuShellData);
}

static void
menu_shell_data_free (gpointer data)
{
  if (data != NULL)
    g_slice_free (MenuShellData, data);
}

static MenuShellData *
gtk_menu_shell_get_menu_shell_data (GtkMenuShell *menu_shell)
{
  MenuShellData *menu_shell_data;

  g_return_val_if_fail (GTK_IS_MENU_SHELL (menu_shell), NULL);

  menu_shell_data = g_object_get_qdata (G_OBJECT (menu_shell), menu_shell_data_quark ());

  if (menu_shell_data == NULL)
    {
      menu_shell_data = menu_shell_data_new ();

      g_object_set_qdata_full (G_OBJECT (menu_shell), menu_shell_data_quark (), menu_shell_data, menu_shell_data_free);
    }

  return menu_shell_data;
}

static WindowData *
gtk_window_get_window_data (GtkWindow *window)
{
  WindowData *window_data;

  g_return_val_if_fail (GTK_IS_WINDOW (window), NULL);

  window_data = g_object_get_qdata (G_OBJECT (window), window_data_quark ());

  if (window_data == NULL)
    {
      static guint window_id;

      GDBusConnection *session = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
      gchar *object_path = g_strdup_printf (OBJECT_PATH "/%d", window_id);
      gchar *old_unique_bus_name = gtk_widget_get_x11_property_string (GTK_WIDGET (window), _GTK_UNIQUE_BUS_NAME);
      gchar *old_unity_object_path = gtk_widget_get_x11_property_string (GTK_WIDGET (window), _UNITY_OBJECT_PATH);
      gchar *old_menubar_object_path = gtk_widget_get_x11_property_string (GTK_WIDGET (window), _GTK_MENUBAR_OBJECT_PATH);
      GDBusActionGroup *old_action_group = NULL;
      GDBusMenuModel *old_menu_model = NULL;

      if (old_unique_bus_name != NULL)
        {
          if (old_unity_object_path != NULL)
            old_action_group = g_dbus_action_group_get (session, old_unique_bus_name, old_unity_object_path);

          if (old_menubar_object_path != NULL)
            old_menu_model = g_dbus_menu_model_get (session, old_unique_bus_name, old_menubar_object_path);
        }

      window_data = window_data_new ();
      window_data->window_id = window_id++;
      window_data->menu_model = g_menu_new ();
      window_data->action_group = unity_gtk_action_group_new (G_ACTION_GROUP (old_action_group));

      if (old_menu_model != NULL)
        {
          window_data->old_model = g_object_ref (old_menu_model);
          g_menu_append_section (window_data->menu_model, NULL, G_MENU_MODEL (old_menu_model));
        }

      window_data->menu_model_export_id = g_dbus_connection_export_menu_model (session, old_menubar_object_path != NULL ? old_menubar_object_path : object_path, G_MENU_MODEL (window_data->menu_model), NULL);
      window_data->action_group_export_id = g_dbus_connection_export_action_group (session, old_unity_object_path != NULL ? old_unity_object_path : object_path, G_ACTION_GROUP (window_data->action_group), NULL);

      if (old_unique_bus_name == NULL)
        gtk_widget_set_x11_property_string (GTK_WIDGET (window), _GTK_UNIQUE_BUS_NAME, g_dbus_connection_get_unique_name (session));

      if (old_unity_object_path == NULL)
        gtk_widget_set_x11_property_string (GTK_WIDGET (window), _UNITY_OBJECT_PATH, object_path);

      if (old_menubar_object_path == NULL)
        gtk_widget_set_x11_property_string (GTK_WIDGET (window), _GTK_MENUBAR_OBJECT_PATH, object_path);

      g_object_set_qdata_full (G_OBJECT (window), window_data_quark (), window_data, window_data_free);

      g_free (old_menubar_object_path);
      g_free (old_unity_object_path);
      g_free (old_unique_bus_name);
      g_free (object_path);
    }

  return window_data;
}

static void
gtk_window_disconnect_menu_shell (GtkWindow    *window,
                                  GtkMenuShell *menu_shell)
{
  WindowData *window_data;
  MenuShellData *menu_shell_data;

  g_return_if_fail (GTK_IS_WINDOW (window));
  g_return_if_fail (GTK_IS_MENU_SHELL (menu_shell));

  menu_shell_data = gtk_menu_shell_get_menu_shell_data (menu_shell);

  g_warn_if_fail (window == menu_shell_data->window);

  window_data = gtk_window_get_window_data (menu_shell_data->window);

  if (window_data != NULL)
    {
      GSList *iter;
      guint i = 0;

      if (window_data->old_model != NULL)
        i++;

      for (iter = window_data->menus; iter != NULL; iter = g_slist_next (iter), i++)
        if (UNITY_GTK_MENU_SHELL (iter->data)->menu_shell == menu_shell)
          break;

      if (iter != NULL)
        {
          g_menu_remove (window_data->menu_model, i);

          unity_gtk_action_group_disconnect_shell (window_data->action_group, iter->data);

          g_object_unref (iter->data);

          window_data->menus = g_slist_delete_link (window_data->menus, iter);
        }

      menu_shell_data->window = NULL;
    }
}

static void
gtk_window_connect_menu_shell (GtkWindow    *window,
                               GtkMenuShell *menu_shell)
{
  MenuShellData *menu_shell_data;

  g_return_if_fail (GTK_IS_WINDOW (window));
  g_return_if_fail (GTK_IS_MENU_SHELL (menu_shell));

  menu_shell_data = gtk_menu_shell_get_menu_shell_data (menu_shell);

  if (window != menu_shell_data->window)
    {
      WindowData *window_data;

      if (menu_shell_data->window != NULL)
        gtk_window_disconnect_menu_shell (menu_shell_data->window, menu_shell);

      window_data = gtk_window_get_window_data (window);

      if (window_data != NULL)
        {
          GSList *iter;

          for (iter = window_data->menus; iter != NULL; iter = g_slist_next (iter))
            if (UNITY_GTK_MENU_SHELL (iter->data)->menu_shell == menu_shell)
              break;

          if (iter == NULL)
            {
              UnityGtkMenuShell *shell = unity_gtk_menu_shell_new (menu_shell);

              unity_gtk_action_group_connect_shell (window_data->action_group, shell);

              g_menu_append_section (window_data->menu_model, NULL, G_MENU_MODEL (shell));

              window_data->menus = g_slist_append (window_data->menus, shell);
            }
        }

      menu_shell_data->window = window;
    }
}

static gboolean
gtk_widget_shell_shows_menubar (GtkWidget *widget)
{
  GtkSettings *settings;
  gboolean shell_shows_menubar;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  settings = gtk_widget_get_settings (widget);

  g_return_val_if_fail (GTK_IS_SETTINGS (settings), FALSE);

  g_object_get (settings, "gtk-shell-shows-menubar", &shell_shows_menubar, NULL);

  return shell_shows_menubar;
}

static void
gtk_settings_handle_gtk_shell_shows_menubar (GObject    *object,
                                             GParamSpec *pspec,
                                             gpointer    user_data)
{
  gtk_widget_queue_resize (user_data);
}

static void
hijacked_window_realize (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WINDOW (widget));

  if (pre_hijacked_window_realize != NULL)
    (* pre_hijacked_window_realize) (widget);

#if GTK_MAJOR_VERSION == 3
  if (!GTK_IS_APPLICATION_WINDOW (widget))
#endif
    gtk_window_get_window_data (GTK_WINDOW (widget));
}

static void
hijacked_window_unrealize (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WINDOW (widget));

  if (pre_hijacked_window_unrealize != NULL)
    (* pre_hijacked_window_unrealize) (widget);

  g_object_set_qdata (G_OBJECT (widget), window_data_quark (), NULL);
}

#if GTK_MAJOR_VERSION == 3
static void
hijacked_application_window_realize (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_APPLICATION_WINDOW (widget));

  if (pre_hijacked_application_window_realize != NULL)
    (* pre_hijacked_application_window_realize) (widget);

  gtk_window_get_window_data (GTK_WINDOW (widget));
}
#endif

static void
hijacked_menu_bar_realize (GtkWidget *widget)
{
  GtkWidget *window;
  GtkSettings *settings;
  MenuShellData *menu_shell_data;

  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  if (pre_hijacked_menu_bar_realize != NULL)
    (* pre_hijacked_menu_bar_realize) (widget);

  window = gtk_widget_get_toplevel (widget);

  if (GTK_IS_WINDOW (window))
    gtk_window_connect_menu_shell (GTK_WINDOW (window), GTK_MENU_SHELL (widget));

  settings = gtk_widget_get_settings (widget);
  menu_shell_data = gtk_menu_shell_get_menu_shell_data (GTK_MENU_SHELL (widget));
  menu_shell_data->settings_notify_gtk_shell_shows_menubar_handler_id = g_signal_connect (settings, "notify::gtk-shell-shows-menubar", G_CALLBACK (gtk_settings_handle_gtk_shell_shows_menubar), widget);
}

static void
hijacked_menu_bar_unrealize (GtkWidget *widget)
{
  MenuShellData *menu_shell_data;

  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  if (pre_hijacked_menu_bar_unrealize != NULL)
    (* pre_hijacked_menu_bar_unrealize) (widget);

  menu_shell_data = gtk_menu_shell_get_menu_shell_data (GTK_MENU_SHELL (widget));

  if (menu_shell_data->settings_notify_gtk_shell_shows_menubar_handler_id)
    {
      g_signal_handler_disconnect (gtk_widget_get_settings (widget), menu_shell_data->settings_notify_gtk_shell_shows_menubar_handler_id);
      menu_shell_data->settings_notify_gtk_shell_shows_menubar_handler_id = 0;
    }

  if (menu_shell_data->window != NULL)
    gtk_window_disconnect_menu_shell (menu_shell_data->window, GTK_MENU_SHELL (widget));
}

static void
hijacked_menu_bar_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  GtkAllocation zero = { 0, 0, 0, 0 };
  GdkWindow *window;

  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  if (gtk_widget_shell_shows_menubar (widget))
    {
      /*
       * We manually assign an empty allocation to the menu bar to
       * prevent the container from attempting to draw it at all.
       */
      if (pre_hijacked_widget_size_allocate != NULL)
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
  else if (pre_hijacked_menu_bar_size_allocate != NULL)
    (* pre_hijacked_menu_bar_size_allocate) (widget, allocation);
}

#if GTK_MAJOR_VERSION == 2
static void
hijacked_menu_bar_size_request (GtkWidget      *widget,
                                GtkRequisition *requisition)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  if (pre_hijacked_menu_bar_size_request != NULL)
    (* pre_hijacked_menu_bar_size_request) (widget, requisition);

  if (gtk_widget_shell_shows_menubar (widget))
    {
      requisition->width = 0;
      requisition->height = 0;
    }
}
#elif GTK_MAJOR_VERSION == 3
static void
hijacked_menu_bar_get_preferred_width (GtkWidget *widget,
                                       gint      *minimum_width,
                                       gint      *natural_width)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  if (pre_hijacked_menu_bar_get_preferred_width != NULL)
    (* pre_hijacked_menu_bar_get_preferred_width) (widget, minimum_width, natural_width);

  if (gtk_widget_shell_shows_menubar (widget))
    {
      *minimum_width = 0;
      *natural_width = 0;
    }
}

static void
hijacked_menu_bar_get_preferred_height (GtkWidget *widget,
                                        gint      *minimum_height,
                                        gint      *natural_height)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  if (pre_hijacked_menu_bar_get_preferred_height != NULL)
    (* pre_hijacked_menu_bar_get_preferred_height) (widget, minimum_height, natural_height);

  if (gtk_widget_shell_shows_menubar (widget))
    {
      *minimum_height = 0;
      *natural_height = 0;
    }
}

static void
hijacked_menu_bar_get_preferred_width_for_height (GtkWidget *widget,
                                                  gint       height,
                                                  gint      *minimum_width,
                                                  gint      *natural_width)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  if (pre_hijacked_menu_bar_get_preferred_width_for_height != NULL)
    (* pre_hijacked_menu_bar_get_preferred_width_for_height) (widget, height, minimum_width, natural_width);

  if (gtk_widget_shell_shows_menubar (widget))
    {
      *minimum_width = 0;
      *natural_width = 0;
    }
}

static void
hijacked_menu_bar_get_preferred_height_for_width (GtkWidget *widget,
                                                  gint       width,
                                                  gint      *minimum_height,
                                                  gint      *natural_height)
{
  g_return_if_fail (GTK_IS_MENU_BAR (widget));

  if (pre_hijacked_menu_bar_get_preferred_height_for_width != NULL)
    (* pre_hijacked_menu_bar_get_preferred_height_for_width) (widget, width, minimum_height, natural_height);

  if (gtk_widget_shell_shows_menubar (widget))
    {
      *minimum_height = 0;
      *natural_height = 0;
    }
}
#endif

static void
hijack_window_class_vtable (GType type)
{
  GtkWidgetClass *widget_class = g_type_class_ref (type);
  GType *children;
  guint n;
  guint i;

  if (widget_class->realize == pre_hijacked_window_realize)
    widget_class->realize = hijacked_window_realize;

#if GTK_MAJOR_VERSION == 3
  if (widget_class->realize == pre_hijacked_application_window_realize)
    widget_class->realize = hijacked_application_window_realize;
#endif

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

  /* This fixes lp:1113008. */
  widget_class->hierarchy_changed = NULL;

  if (widget_class->realize == pre_hijacked_menu_bar_realize)
    widget_class->realize = hijacked_menu_bar_realize;

  if (widget_class->unrealize == pre_hijacked_menu_bar_unrealize)
    widget_class->unrealize = hijacked_menu_bar_unrealize;

  if (widget_class->size_allocate == pre_hijacked_menu_bar_size_allocate)
    widget_class->size_allocate = hijacked_menu_bar_size_allocate;

#if GTK_MAJOR_VERSION == 2
  if (widget_class->size_request == pre_hijacked_menu_bar_size_request)
    widget_class->size_request = hijacked_menu_bar_size_request;
#elif GTK_MAJOR_VERSION == 3
  if (widget_class->get_preferred_width == pre_hijacked_menu_bar_get_preferred_width)
    widget_class->get_preferred_width = hijacked_menu_bar_get_preferred_width;

  if (widget_class->get_preferred_height == pre_hijacked_menu_bar_get_preferred_height)
    widget_class->get_preferred_height = hijacked_menu_bar_get_preferred_height;

  if (widget_class->get_preferred_width_for_height == pre_hijacked_menu_bar_get_preferred_width_for_height)
    widget_class->get_preferred_width_for_height = hijacked_menu_bar_get_preferred_width_for_height;

  if (widget_class->get_preferred_height_for_width == pre_hijacked_menu_bar_get_preferred_height_for_width)
    widget_class->get_preferred_height_for_width = hijacked_menu_bar_get_preferred_height_for_width;
#endif

  children = g_type_children (type, &n);

  for (i = 0; i < n; i++)
    hijack_menu_bar_class_vtable (children[i]);

  g_free (children);
}

void
gtk_module_init (void)
{
  const gchar *proxy = g_getenv ("UBUNTU_MENUPROXY");

  if ((proxy == NULL || is_true (proxy)) && !is_blacklisted (g_get_prgname ()))
    {
      GtkWidgetClass *widget_class;

      unity_gtk_menu_shell_set_debug (is_true (g_getenv ("UNITY_GTK_MENU_SHELL_DEBUG")));
      unity_gtk_action_group_set_debug (is_true (g_getenv ("UNITY_GTK_ACTION_GROUP_DEBUG")));

      /* store the base GtkWidget size_allocate vfunc */
      widget_class = g_type_class_ref (GTK_TYPE_WIDGET);
      pre_hijacked_widget_size_allocate = widget_class->size_allocate;

#if GTK_MAJOR_VERSION == 3
      /* store the base GtkApplicationWindow realize vfunc */
      widget_class = g_type_class_ref (GTK_TYPE_APPLICATION_WINDOW);
      pre_hijacked_application_window_realize = widget_class->realize;
#endif

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
#if GTK_MAJOR_VERSION == 2
      pre_hijacked_menu_bar_size_request = widget_class->size_request;
#elif GTK_MAJOR_VERSION == 3
      pre_hijacked_menu_bar_get_preferred_width = widget_class->get_preferred_width;
      pre_hijacked_menu_bar_get_preferred_height = widget_class->get_preferred_height;
      pre_hijacked_menu_bar_get_preferred_width_for_height = widget_class->get_preferred_width_for_height;
      pre_hijacked_menu_bar_get_preferred_height_for_width = widget_class->get_preferred_height_for_width;
#endif
      hijack_menu_bar_class_vtable (GTK_TYPE_MENU_BAR);
    }
}
