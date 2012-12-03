#ifndef __UNITY_GTK_ACTION_GROUP_H__
#define __UNITY_GTK_ACTION_GROUP_H__

#include <glib-object.h>

typedef struct _UnityGtkActionGroup      UnityGtkActionGroup;
typedef struct _UnityGtkActionGroupClass UnityGtkActionGroupClass;

#define UNITY_GTK_TYPE_ACTION_GROUP            (unity_gtk_action_group_get_type ())
#define UNITY_GTK_ACTION_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroup))
#define UNITY_GTK_IS_ACTION_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_GTK_TYPE_ACTION_GROUP))
#define UNITY_GTK_ACTION_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroupClass))
#define UNITY_GTK_IS_ACTION_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_GTK_TYPE_ACTION_GROUP))
#define UNITY_GTK_ACTION_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_GTK_TYPE_ACTION_GROUP, UnityGtkActionGroupClass))

struct _UnityGtkActionGroup
{
  GObject parent_instance;

  /*< private >*/
  GHashTable *actions_by_name;
};

struct _UnityGtkActionGroupClass
{
  GObjectClass parent_class;
};

GType unity_gtk_action_group_get_type (void) G_GNUC_INTERNAL;

#endif /* __UNITY_GTK_ACTION_GROUP_H__ */
