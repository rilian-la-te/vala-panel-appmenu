#ifndef __UNITY_GTK_ACTION_H__
#define __UNITY_GTK_ACTION_H__

#include <glib-object.h>

typedef struct _UnityGtkAction      UnityGtkAction;
typedef struct _UnityGtkActionClass UnityGtkActionClass;

#include "unity-gtk-menu-item.h"

#define UNITY_GTK_TYPE_ACTION            (unity_gtk_action_get_type ())
#define UNITY_GTK_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_GTK_TYPE_ACTION, UnityGtkAction))
#define UNITY_GTK_IS_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_GTK_TYPE_ACTION))
#define UNITY_GTK_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_GTK_TYPE_ACTION, UnityGtkActionClass))
#define UNITY_GTK_IS_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_GTK_TYPE_ACTION))
#define UNITY_GTK_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_GTK_TYPE_ACTION, UnityGtkActionClass))

struct _UnityGtkAction
{
  GObject parent_instance;

  /*< private >*/
  gchar            *name;
  UnityGtkMenuItem *item;
  GHashTable       *items_by_name;
};

struct _UnityGtkActionClass
{
  GObjectClass parent_class;
};

GType unity_gtk_action_get_type (void) G_GNUC_INTERNAL;

#endif /* __UNITY_GTK_ACTION_H__ */
