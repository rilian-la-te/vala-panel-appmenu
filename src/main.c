#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "unitygtkmenuparser.h"

G_DEFINE_QUARK(window_menu, window_menu)

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
  static gint id;

  GMenu *menu;
  GMenu *submenu;
  GdkX11Window *window;
  gchar object_path[80];
  GDBusConnection *session;

  g_message ("%s (%p)", __func__, widget);

  (* pre_hijacked_window_realize) (widget);

  menu = g_menu_new ();

  submenu = g_menu_new ();
  g_menu_append (submenu, "World", NULL);
  g_menu_append_submenu (menu, "Hello", G_MENU_MODEL (submenu));

  submenu = g_menu_new ();
  g_menu_append (submenu, "World", NULL);
  g_menu_append_submenu (menu, "Goodbye", G_MENU_MODEL (submenu));

  session = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
  sprintf (object_path, "/com/canonical/Unity/GtkModule/Window/%d", id++);

  g_dbus_connection_export_menu_model (session, object_path, G_MENU_MODEL (menu), NULL);

  window = GDK_X11_WINDOW (gtk_widget_get_window (widget));

  gdk_x11_window_set_utf8_property (window,
                                    "_GTK_UNIQUE_BUS_NAME",
                                    g_dbus_connection_get_unique_name (session));
  gdk_x11_window_set_utf8_property (window,
                                    "_GTK_MENUBAR_OBJECT_PATH",
                                    object_path);

  g_object_set_qdata (G_OBJECT (widget), window_menu_quark (), menu);
}

static void
hijacked_window_unrealize (GtkWidget *widget)
{
  g_message ("%s (%p)", __func__, widget);

  (* pre_hijacked_window_unrealize) (widget);
}

static void
hijacked_menu_bar_get_preferred_width (GtkWidget *widget,
                                       gint      *minimum_width,
                                       gint      *natural_width)
{
  g_message ("%s (%p, %p, %p)", __func__, widget, minimum_width, natural_width);

  *minimum_width = 0;
  *natural_width = 0;
}

static void
hijacked_menu_bar_get_preferred_height (GtkWidget *widget,
                                        gint      *minimum_height,
                                        gint      *natural_height)
{
  g_message ("%s (%p, %p, %p)", __func__, widget, minimum_height, natural_height);

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
  GMenu *menu;
  GtkWidget *window;
  UnityGtkMenuParser *parser;

  g_message ("%s (%p)", __func__, widget);

  (* pre_hijacked_menu_bar_realize) (widget);

  window = gtk_widget_get_toplevel (widget);
  menu = g_object_get_qdata (G_OBJECT (window), window_menu_quark ());

  g_assert (menu != NULL);

  parser = unity_gtk_menu_parser_new (GTK_MENU_SHELL (widget));
  g_menu_append_section (menu, NULL, G_MENU_MODEL (parser));
}

static void
hijacked_menu_bar_unrealize (GtkWidget *widget)
{
  GMenu *menu;
  GtkWidget *window;
  guint n, i;

  g_message ("%s (%p)", __func__, widget);

  window = gtk_widget_get_toplevel (widget);
  menu = g_object_get_qdata (G_OBJECT (window), window_menu_quark ());

  g_assert (menu != NULL);

  n = g_menu_model_get_n_items (G_MENU_MODEL (menu));

  for (i = 2; i < n; i++)
    g_menu_remove (menu, 2);

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
