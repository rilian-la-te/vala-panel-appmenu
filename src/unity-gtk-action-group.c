#include "unity-gtk-action-group.h"
#include <gio/gio.h>

static void unity_gtk_action_group_action_group_init (GActionGroupInterface *iface);

G_DEFINE_TYPE_WITH_CODE (UnityGtkActionGroup,
                         unity_gtk_action_group,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP,
                                                unity_gtk_action_group_action_group_init));

static void
unity_gtk_action_group_dispose (GObject *object)
{
  UnityGtkActionGroup *group;
  GHashTable *actions_by_name;

  g_return_if_fail (UNITY_GTK_IS_ACTION_GROUP (object));

  group = UNITY_GTK_ACTION_GROUP (object);
  actions_by_name = group->actions_by_name;

  if (actions_by_name != NULL)
    {
      group->actions_by_name = NULL;
      g_hash_table_unref (actions_by_name);
    }

  G_OBJECT_CLASS (unity_gtk_action_group_parent_class)->dispose (object);
}

static gchar **
unity_gtk_action_group_list_actions (GActionGroup *action_group)
{
}

static void
unity_gtk_action_group_change_action_state (GActionGroup *action_group,
                                            const gchar  *action_name,
                                            GVariant     *value)
{
}

static void
unity_gtk_action_group_activate_action (GActionGroup *action_group,
                                        const gchar  *action_name,
                                        GVariant     *parameter)
{
}

static gboolean
unity_gtk_action_group_query_action (GActionGroup        *action_group,
                                     const gchar         *action_name,
                                     gboolean            *enabled,
                                     const GVariantType **parameter_type,
                                     const GVariantType **state_type,
                                     GVariant           **state_hint,
                                     GVariant           **state)
{
}

static void
unity_gtk_action_group_class_init (UnityGtkActionGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = unity_gtk_action_group_dispose;
}

static void
unity_gtk_action_group_action_group_init (GActionGroupInterface *iface)
{
  iface->list_actions = unity_gtk_action_group_list_actions;
  iface->change_action_state = unity_gtk_action_group_change_action_state;
  iface->activate_action = unity_gtk_action_group_activate_action;
  iface->query_action = unity_gtk_action_group_query_action;
}

static void
unity_gtk_action_group_init (UnityGtkActionGroup *self)
{
  self->actions_by_name = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);
}
