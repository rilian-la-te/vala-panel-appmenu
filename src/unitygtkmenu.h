#ifndef __UNITY_GTK_MENU_H__
#define __UNITY_GTK_MENU_H__

#include <gtk/gtk.h>

#define UNITY_GTK_TYPE_MENU                    (unity_gtk_menu_get_type ())
#define UNITY_GTK_MENU(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_GTK_TYPE_MENU, UnityGtkMenu))
#define UNITY_GTK_IS_MENU(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_GTK_TYPE_MENU))
#define UNITY_GTK_MENU_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_GTK_TYPE_MENU, UnityGtkMenuClass))
#define UNITY_GTK_IS_MENU_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_GTK_TYPE_MENU))
#define UNITY_GTK_MENU_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_GTK_TYPE_MENU, UnityGtkMenuClass))
#define UNITY_GTK_TYPE_ACTION_GROUP            (unity_gtk_action_group_get_type ())
#define UNITY_GTK_ACTION_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroup))
#define UNITY_GTK_IS_ACTION_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_GTK_TYPE_ACTION_GROUP))
#define UNITY_GTK_ACTION_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroupClass))
#define UNITY_GTK_IS_ACTION_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_GTK_TYPE_ACTION_GROUP))
#define UNITY_GTK_ACTION_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroupClass))

typedef struct _UnityGtkMenu               UnityGtkMenu;
typedef struct _UnityGtkMenuClass          UnityGtkMenuClass;
typedef struct _UnityGtkMenuPrivate        UnityGtkMenuPrivate;
typedef struct _UnityGtkActionGroup        UnityGtkActionGroup;
typedef struct _UnityGtkActionGroupClass   UnityGtkActionGroupClass;
typedef struct _UnityGtkActionGroupPrivate UnityGtkActionGroupPrivate;

struct _UnityGtkMenu
{
  GMenuModel parent_instance;

  /*< private >*/
  UnityGtkMenuPrivate *priv;
};

struct _UnityGtkMenuClass
{
  GMenuModelClass parent_class;
};

struct _UnityGtkActionGroup
{
  GObject parent_instance;

  /*< private >*/
  UnityGtkActionGroupPrivate *priv;
};

struct _UnityGtkActionGroupClass
{
  GObjectClass parent_class;
};

GType                 unity_gtk_menu_get_type         (void)                       G_GNUC_INTERNAL;

UnityGtkMenu *        unity_gtk_menu_new              (GtkMenuShell        *shell) G_GNUC_INTERNAL;

void                  unity_gtk_menu_set_action_group (UnityGtkMenu        *menu,
                                                       UnityGtkActionGroup *group,
                                                       const gchar         *prefix) G_GNUC_INTERNAL;

gboolean              unity_gtk_menu_is_valid         (UnityGtkMenu        *menu)  G_GNUC_INTERNAL;

void                  unity_gtk_menu_print            (UnityGtkMenu        *menu,
                                                       guint                depth) G_GNUC_INTERNAL;

void                  g_menu_model_print              (GMenuModel          *model,
                                                       guint                depth) G_GNUC_INTERNAL;

UnityGtkActionGroup * unity_gtk_action_group_new      (void)                       G_GNUC_INTERNAL;

#endif /* __UNITY_GTK_MENU_H__ */
