#ifndef __UNITY_GTK_MENU_SHELL_H__
#define __UNITY_GTK_MENU_SHELL_H__

typedef struct _UnityGtkMenuShell      UnityGtkMenuShell;
typedef struct _UnityGtkMenuShellClass UnityGtkMenuShellClass;

#include <gtk/gtk.h>
#include "unity-gtk-menu-item.h"

#define UNITY_GTK_TYPE_MENU_SHELL            (unity_gtk_menu_shell_get_type ())
#define UNITY_GTK_MENU_SHELL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_GTK_TYPE_MENU_SHELL, UnityGtkMenuShell))
#define UNITY_GTK_IS_MENU_SHELL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_GTK_TYPE_MENU_SHELL))
#define UNITY_GTK_MENU_SHELL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_GTK_TYPE_MENU_SHELL, UnityGtkMenuShellClass))
#define UNITY_GTK_IS_MENU_SHELL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_GTK_TYPE_MENU_SHELL))
#define UNITY_GTK_MENU_SHELL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_GTK_TYPE_MENU_SHELL, UnityGtkMenuShellClass))

struct _UnityGtkMenuShell
{
  GMenuModel parent_instance;

  /*< private >*/
  GtkMenuShell *menu_shell;
  gulong        menu_shell_insert_handler_id;
  GPtrArray    *items;
  GPtrArray    *sections;
  GSequence    *visible_indices;
  GSequence    *separator_indices;
};

struct _UnityGtkMenuShellClass
{
  GMenuModelClass parent_class;
};

GType               unity_gtk_menu_shell_get_type              (void)                          G_GNUC_INTERNAL;

UnityGtkMenuShell * unity_gtk_menu_shell_new                   (GtkMenuShell      *menu_shell) G_GNUC_INTERNAL;

UnityGtkMenuItem *  unity_gtk_menu_shell_get_item              (UnityGtkMenuShell *shell,
                                                                guint              index)      G_GNUC_INTERNAL;

GSequence *         unity_gtk_menu_shell_get_visible_indices   (UnityGtkMenuShell *shell)      G_GNUC_INTERNAL;

GSequence *         unity_gtk_menu_shell_get_separator_indices (UnityGtkMenuShell *shell)      G_GNUC_INTERNAL;

void                unity_gtk_menu_shell_handle_item_notify    (UnityGtkMenuShell *shell,
                                                                UnityGtkMenuItem  *item,
                                                                GParamSpec        *pspec)      G_GNUC_INTERNAL;

#endif /* __UNITY_GTK_MENU_SHELL_H__ */
