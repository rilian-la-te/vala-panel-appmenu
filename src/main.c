#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "unitygtkmenuparser.h"

G_DEFINE_QUARK(UnityGtkModuleMenuModel, unity_gtk_module_menu_model)

static void (* widget_class_size_allocate)                          (GtkWidget     *widget,
                                                                     GtkAllocation *allocation);

static void (* pre_hijacked_window_realize)                         (GtkWidget     *widget);

static void (* pre_hijacked_window_unrealize)                       (GtkWidget     *widget);

static void (* pre_hijacked_menubar_get_preferred_width)            (GtkWidget     *widget,
                                                                     gint          *minimum_width,
                                                                     gint          *natural_width);

static void (* pre_hijacked_menubar_get_preferred_height)           (GtkWidget     *widget,
                                                                     gint          *minimum_height,
                                                                     gint          *natural_height);

static void (* pre_hijacked_menubar_get_preferred_width_for_height) (GtkWidget     *widget,
                                                                     gint           height,
                                                                     gint          *minimum_width,
                                                                     gint          *natural_width);

static void (* pre_hijacked_menubar_get_preferred_height_for_width) (GtkWidget     *widget,
                                                                     gint           width,
                                                                     gint          *minimum_height,
                                                                     gint          *natural_height);

static void (* pre_hijacked_menubar_size_allocate)                  (GtkWidget     *widget,
                                                                     GtkAllocation *allocation);

static void (* pre_hijacked_menubar_realize)                        (GtkWidget     *widget);

static void (* pre_hijacked_menubar_unrealize)                      (GtkWidget     *widget);

static void
hijacked_window_realize (GtkWidget *widget)
{
  g_print ("%s\n", __func__);

  static gint id;

  GMenu *menu;
  GMenu *submenu;
  GdkX11Window *window;
  gchar object_path[80];
  GDBusConnection *session;

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

  g_object_set_qdata (G_OBJECT (widget), unity_gtk_module_menu_model_quark (), menu);
}

static void
hijacked_window_unrealize (GtkWidget *widget)
{
  g_print ("%s\n", __func__);

  (* pre_hijacked_window_unrealize) (widget);
}

static void
hijacked_menubar_get_preferred_width (GtkWidget *widget,
                                      gint      *minimum_width,
                                      gint      *natural_width)
{
  g_print ("%s\n", __func__);

  *minimum_width = 0;
  *natural_width = 0;
}

static void
hijacked_menubar_get_preferred_height (GtkWidget *widget,
                                       gint      *minimum_height,
                                       gint      *natural_height)
{
  g_print ("%s\n", __func__);

  *minimum_height = 0;
  *natural_height = 0;
}

static void
hijacked_menubar_get_preferred_width_for_height (GtkWidget *widget,
                                                 gint       height,
                                                 gint      *minimum_width,
                                                 gint      *natural_width)
{
  g_print ("%s\n", __func__);

  *minimum_width = 0;
  *natural_width = 0;
}

static void
hijacked_menubar_get_preferred_height_for_width (GtkWidget *widget,
                                                 gint       width,
                                                 gint      *minimum_height,
                                                 gint      *natural_height)
{
  g_print ("%s\n", __func__);

  *minimum_height = 0;
  *natural_height = 0;
}

static void
hijacked_menubar_size_allocate (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  g_print ("%s\n", __func__);

  GtkAllocation zero = { 0, 0, 0, 0 };
  GdkWindow *window;

  /* We manually assign an empty allocation to the menubar to
   * prevent the container from attempting to draw it at all.
   */
  (* widget_class_size_allocate) (widget, &zero);

  /* Then we move the GdkWindow belonging to the menubar outside of
   * the clipping rectangle of the parent window so that we can't
   * see it.
   */
  window = gtk_widget_get_window (widget);

  if (window != NULL)
    gdk_window_move_resize (window, -1, -1, 1, 1);
}

static void
hijacked_menubar_realize (GtkWidget *widget)
{
  g_print ("%s\n", __func__);

  GMenu *menu;
  GtkWidget *window;
  UnityGtkMenuParser *parser;

  (* pre_hijacked_menubar_realize) (widget);

  window = gtk_widget_get_toplevel (widget);
  menu = g_object_get_qdata (G_OBJECT (window), unity_gtk_module_menu_model_quark ());

  g_assert (menu != NULL);

  parser = unity_gtk_menu_parser_new (GTK_MENU_SHELL (widget));
  g_menu_append_section (menu, NULL, G_MENU_MODEL (parser));
}

static void
hijacked_menubar_unrealize (GtkWidget *widget)
{
  g_print ("%s\n", __func__);

  GMenu *menu;
  GtkWidget *window;
  guint n, i;

  window = gtk_widget_get_toplevel (widget);
  menu = g_object_get_qdata (G_OBJECT (window), unity_gtk_module_menu_model_quark ());

  g_assert (menu != NULL);

  n = g_menu_model_get_n_items (G_MENU_MODEL (menu));

  for (i = 2; i < n; i++)
    g_menu_remove (menu, 2);

  (* pre_hijacked_menubar_unrealize) (widget);
}

static void
patch_window_class_vtable (GType type)
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
    patch_window_class_vtable (children[i]);
  g_free (children);
}

static void
patch_menubar_class_vtable (GType type)
{
  GtkWidgetClass *widget_class;
  GType *children;
  guint n, i;

  widget_class = g_type_class_ref (type);

  if (widget_class->get_preferred_height == pre_hijacked_menubar_get_preferred_height)
    widget_class->get_preferred_height = hijacked_menubar_get_preferred_height;

  if (widget_class->get_preferred_width_for_height == pre_hijacked_menubar_get_preferred_width_for_height)
    widget_class->get_preferred_width_for_height = hijacked_menubar_get_preferred_width_for_height;

  if (widget_class->get_preferred_width == pre_hijacked_menubar_get_preferred_width)
    widget_class->get_preferred_width = hijacked_menubar_get_preferred_width;

  if (widget_class->get_preferred_height_for_width == pre_hijacked_menubar_get_preferred_height_for_width)
    widget_class->get_preferred_height_for_width = hijacked_menubar_get_preferred_height_for_width;

  if (widget_class->size_allocate == pre_hijacked_menubar_size_allocate)
    widget_class->size_allocate = hijacked_menubar_size_allocate;

  if (widget_class->realize == pre_hijacked_menubar_realize)
    widget_class->realize = hijacked_menubar_realize;

  if (widget_class->unrealize == pre_hijacked_menubar_unrealize)
    widget_class->unrealize = hijacked_menubar_unrealize;

  children = g_type_children (type, &n);
  for (i = 0; i < n; i++)
    patch_menubar_class_vtable (children[i]);
  g_free (children);
}

void
gtk_module_init (void)
{
  GtkWidgetClass *widget_class;

  /* store the base GtkWidget size_allocate vfunc */
  widget_class = g_type_class_ref (GTK_TYPE_WIDGET);
  widget_class_size_allocate = widget_class->size_allocate;

  /* intercept window realize vcalls on GtkWindow */
  widget_class = g_type_class_ref (GTK_TYPE_WINDOW);
  pre_hijacked_window_realize = widget_class->realize;
  pre_hijacked_window_unrealize = widget_class->unrealize;
  patch_window_class_vtable (GTK_TYPE_WINDOW);

  /* intercept size request and allocate vcalls on GtkMenuBar (for hiding) */
  widget_class = g_type_class_ref (GTK_TYPE_MENU_BAR);
  pre_hijacked_menubar_get_preferred_width = widget_class->get_preferred_width;
  pre_hijacked_menubar_get_preferred_height = widget_class->get_preferred_height;
  pre_hijacked_menubar_get_preferred_width_for_height = widget_class->get_preferred_width_for_height;
  pre_hijacked_menubar_get_preferred_height_for_width = widget_class->get_preferred_height_for_width;
  pre_hijacked_menubar_size_allocate = widget_class->size_allocate;
  pre_hijacked_menubar_realize = widget_class->realize;
  pre_hijacked_menubar_unrealize = widget_class->unrealize;
  patch_menubar_class_vtable (GTK_TYPE_MENU_BAR);
}
