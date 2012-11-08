#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "unitygtkmenuparser.h"

#define WINDOW_OBJECT_PATH "/com/canonical/Unity/Gtk/Window"

G_DEFINE_QUARK (window_data, window_data);

struct _WindowData
{
  guint   window_id;
  guint   export_id;
  GMenu  *menu;
  GSList *menu_shells;
};

typedef struct _WindowData WindowData;

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
      if (window_data->export_id)
        {
          GDBusConnection *session;

          session = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

          g_dbus_connection_unexport_menu_model (session, window_data->export_id);
        }

      if (window_data->menu != NULL)
        g_object_unref (window_data->menu);

      g_slice_free (WindowData, data);
    }
}

static void (* pre_hijacked_widget_size_allocate)                    (GtkWidget     *widget,
                                                                      GtkAllocation *allocation);

static void (* pre_hijacked_window_realize)                          (GtkWidget     *widget);

static void (* pre_hijacked_window_unrealize)                        (GtkWidget     *widget);

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

static void (* pre_hijacked_menu_bar_size_allocate)                  (GtkWidget     *widget,
                                                                      GtkAllocation *allocation);

static void (* pre_hijacked_menu_bar_realize)                        (GtkWidget     *widget);

static void (* pre_hijacked_menu_bar_unrealize)                      (GtkWidget     *widget);

static void
hijacked_window_realize (GtkWidget *widget)
{
  WindowData *window_data;

  g_message ("%s (%p)", __func__, widget);

  (* pre_hijacked_window_realize) (widget);

  window_data = g_object_get_qdata (G_OBJECT (widget), window_data_quark ());

  if (window_data == NULL)
    {
      static guint window_id;

      GDBusConnection *session;
      gchar object_path[80];
      GdkX11Window *window;

      window_data = window_data_new ();
      window_data->menu = g_menu_new ();
      window_data->window_id = window_id++;

      session = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
      sprintf (object_path, WINDOW_OBJECT_PATH "/%d", window_data->window_id);
      window_data->export_id = g_dbus_connection_export_menu_model (session, object_path, G_MENU_MODEL (window_data->menu), NULL);

      window = GDK_X11_WINDOW (gtk_widget_get_window (widget));

      gdk_x11_window_set_utf8_property (window, "_GTK_MENUBAR_OBJECT_PATH", object_path);
      gdk_x11_window_set_utf8_property (window, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name (session));

      g_object_set_qdata_full (G_OBJECT (widget), window_data_quark (), window_data, window_data_free);
    }
}

static void
hijacked_window_unrealize (GtkWidget *widget)
{
  g_message ("%s (%p)", __func__, widget);

  window_data_free (g_object_get_qdata (G_OBJECT (widget), window_data_quark ()));

  (* pre_hijacked_window_unrealize) (widget);
}

static void
hijacked_menu_bar_get_preferred_width (GtkWidget *widget,
                                       gint      *minimum_width,
                                       gint      *natural_width)
{
  g_message ("%s (%p, %p, %p)", __func__, widget, minimum_width, natural_width);

  (* pre_hijacked_menu_bar_get_preferred_width) (widget, minimum_width, natural_width);

  *minimum_width = 0;
  *natural_width = 0;
}

static void
hijacked_menu_bar_get_preferred_height (GtkWidget *widget,
                                        gint      *minimum_height,
                                        gint      *natural_height)
{
  g_message ("%s (%p, %p, %p)", __func__, widget, minimum_height, natural_height);

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
  g_message ("%s (%p, %d, %p, %p)", __func__, widget, height, minimum_width, natural_width);

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
  g_message ("%s (%p, %d, %p, %p)", __func__, widget, width, minimum_height, natural_height);

  (* pre_hijacked_menu_bar_get_preferred_height_for_width) (widget, width, minimum_height, natural_height);

  *minimum_height = 0;
  *natural_height = 0;
}

static void
hijacked_menu_bar_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  GtkAllocation zero = { 0, 0, 0, 0 };
  GdkWindow *window;

  g_message ("%s (%p, %p)", __func__, widget, allocation);

  /* We manually assign an empty allocation to the menu bar to
   * prevent the container from attempting to draw it at all.
   */
  (* pre_hijacked_widget_size_allocate) (widget, &zero);

  /* Then we move the GdkWindow belonging to the menu bar outside of
   * the clipping rectangle of the parent window so that we can't
   * see it.
   */
  window = gtk_widget_get_window (widget);

  if (window != NULL)
    gdk_window_move_resize (window, -1, -1, 1, 1);
}

static void
hijacked_menu_bar_realize (GtkWidget *widget)
{
  GtkWidget *window;
  WindowData *window_data;

  g_message ("%s (%p)", __func__, widget);

  (* pre_hijacked_menu_bar_realize) (widget);

  window = gtk_widget_get_toplevel (widget);
  window_data = g_object_get_qdata (G_OBJECT (window), window_data_quark ());

  if (window_data != NULL)
    {
      GSList *iter = g_slist_find (window_data->menu_shells, widget);

      if (iter == NULL)
        {
          UnityGtkMenuParser *parser = unity_gtk_menu_parser_new (GTK_MENU_SHELL (widget));

          g_menu_append_section (window_data->menu, NULL, G_MENU_MODEL (parser));

          window_data->menu_shells = g_slist_append (window_data->menu_shells, widget);
        }
    }
}

static void
hijacked_menu_bar_unrealize (GtkWidget *widget)
{
  GtkWidget *window;
  WindowData *window_data;

  g_message ("%s (%p)", __func__, widget);

  window = gtk_widget_get_toplevel (widget);
  window_data = g_object_get_qdata (G_OBJECT (window), window_data_quark ());

  if (window_data != NULL)
    {
      gint i = g_slist_index (window_data->menu_shells, widget);

      if (i >= 0)
        g_menu_remove (window_data->menu, i);
    }

  (* pre_hijacked_menu_bar_unrealize) (widget);
}

static void
hijack_window_class_vtable (GType type)
{
  GtkWidgetClass *widget_class;
  GType *children;
  guint n, i;

  widget_class = g_type_class_ref (type);

  if (widget_class->realize == pre_hijacked_window_realize)
    widget_class->realize = hijacked_window_realize;

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
  GtkWidgetClass *widget_class;
  GType *children;
  guint n, i;

  widget_class = g_type_class_ref (type);

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

  /* intercept window realize vcalls on GtkWindow */
  widget_class = g_type_class_ref (GTK_TYPE_WINDOW);
  pre_hijacked_window_realize = widget_class->realize;
  pre_hijacked_window_unrealize = widget_class->unrealize;
  hijack_window_class_vtable (GTK_TYPE_WINDOW);

  /* intercept size request and allocate vcalls on GtkMenuBar (for hiding) */
  widget_class = g_type_class_ref (GTK_TYPE_MENU_BAR);
  pre_hijacked_menu_bar_get_preferred_width = widget_class->get_preferred_width;
  pre_hijacked_menu_bar_get_preferred_height = widget_class->get_preferred_height;
  pre_hijacked_menu_bar_get_preferred_width_for_height = widget_class->get_preferred_width_for_height;
  pre_hijacked_menu_bar_get_preferred_height_for_width = widget_class->get_preferred_height_for_width;
  pre_hijacked_menu_bar_size_allocate = widget_class->size_allocate;
  pre_hijacked_menu_bar_realize = widget_class->realize;
  pre_hijacked_menu_bar_unrealize = widget_class->unrealize;
  hijack_menu_bar_class_vtable (GTK_TYPE_MENU_BAR);
}
