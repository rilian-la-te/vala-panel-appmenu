#include "unity-gtk-action-group.h"
#include "unity-gtk-action.h"
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
  UnityGtkActionGroup *group;
  GHashTable *actions_by_name;
  GHashTableIter iter;
  gchar **names;
  gpointer key;
  guint n;
  guint i;

  g_return_val_if_fail (UNITY_GTK_IS_ACTION_GROUP (action_group), NULL);

  group = UNITY_GTK_ACTION_GROUP (action_group);
  actions_by_name = group->actions_by_name;

  g_return_val_if_fail (actions_by_name != NULL, NULL);

  n = g_hash_table_size (actions_by_name);
  names = g_malloc_n (n + 1, sizeof (gchar *));

  g_hash_table_iter_init (&iter, actions_by_name);
  for (i = 0; i < n && g_hash_table_iter_next (&iter, &key, NULL); i++)
    names[i] = g_strdup (key);

  names[i] = NULL;

  return names;
}

static void
unity_gtk_action_group_really_change_action_state (GActionGroup *action_group,
                                                   const gchar  *action_name,
                                                   GVariant     *value)
{
  UnityGtkActionGroup *group;
  GHashTable *actions_by_name;
  UnityGtkAction *action;

  g_return_if_fail (UNITY_GTK_IS_ACTION_GROUP (action_group));

  group = UNITY_GTK_ACTION_GROUP (action_group);
  actions_by_name = group->actions_by_name;

  g_return_if_fail (actions_by_name != NULL);

  action = g_hash_table_lookup (actions_by_name, action_name);

  g_return_if_fail (action != NULL);

  if (action->items_by_name != NULL)
    {
      const gchar *name;
      UnityGtkMenuItem *item;

      g_return_if_fail (value != NULL && g_variant_is_of_type (value, G_VARIANT_TYPE_STRING));

      name = g_variant_get_string (value, NULL);
      item = g_hash_table_lookup (action->items_by_name, name);

      if (item != NULL && unity_gtk_menu_item_is_check (item))
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item->menu_item), TRUE);
    }
  else if (action->item != NULL && unity_gtk_menu_item_is_check (action->item))
    {
      g_return_if_fail (value != NULL && g_variant_is_of_type (value, G_VARIANT_TYPE_BOOLEAN));

      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (action->item->menu_item), g_variant_get_boolean (value));
    }
  else
    g_warn_if_fail (value == NULL);
}

static void
unity_gtk_action_group_change_action_state (GActionGroup *action_group,
                                            const gchar  *action_name,
                                            GVariant     *value)
{
  g_variant_ref_sink (value);
  unity_gtk_action_group_really_change_action_state (action_group, action_name, value);
  g_variant_unref (value);
}

static void
unity_gtk_action_group_activate_action (GActionGroup *action_group,
                                        const gchar  *action_name,
                                        GVariant     *parameter)
{
  UnityGtkActionGroup *group;
  GHashTable *actions_by_name;
  UnityGtkAction *action;

  g_return_if_fail (UNITY_GTK_IS_ACTION_GROUP (action_group));

  group = UNITY_GTK_ACTION_GROUP (action_group);
  actions_by_name = group->actions_by_name;

  g_return_if_fail (actions_by_name != NULL);

  action = g_hash_table_lookup (actions_by_name, action_name);

  g_return_if_fail (action != NULL);

  if (action->items_by_name != NULL)
    {
      const gchar *name;
      UnityGtkMenuItem *item;

      g_return_if_fail (parameter != NULL && g_variant_is_of_type (parameter, G_VARIANT_TYPE_STRING));

      name = g_variant_get_string (parameter, NULL);
      item = g_hash_table_lookup (action->items_by_name, name);

      if (item != NULL && item->menu_item != NULL)
        gtk_menu_item_activate (item->menu_item);
    }
  else if (action->item != NULL)
    {
      g_warn_if_fail (parameter == NULL);

      if (action->item->menu_item != NULL)
        gtk_menu_item_activate (action->item->menu_item);
    }
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
  UnityGtkActionGroup *group;
  GHashTable *actions_by_name;
  UnityGtkAction *action;

  g_return_val_if_fail (UNITY_GTK_IS_ACTION_GROUP (action_group), FALSE);

  group = UNITY_GTK_ACTION_GROUP (action_group);
  actions_by_name = group->actions_by_name;

  g_return_val_if_fail (actions_by_name != NULL, FALSE);

  action = g_hash_table_lookup (actions_by_name, action_name);

  if (action != NULL)
    {
      if (enabled != NULL)
        {
          if (action->items_by_name != NULL)
            {
              GHashTableIter iter;
              gpointer value;

              *enabled = FALSE;

              g_hash_table_iter_init (&iter, action->items_by_name);
              while (!*enabled && g_hash_table_iter_next (&iter, NULL, &value))
                *enabled = unity_gtk_menu_item_is_sensitive (value);
            }
          else
            *enabled = action->item != NULL && unity_gtk_menu_item_is_sensitive (action->item);
        }

      if (parameter_type != NULL)
        {
          if (action->items_by_name != NULL)
            *parameter_type = G_VARIANT_TYPE_STRING;
          else
            *parameter_type = NULL;
        }

      if (state_type != NULL)
        {
          if (action->items_by_name != NULL)
            *state_type = G_VARIANT_TYPE_STRING;
          else if (action->item != NULL && unity_gtk_menu_item_is_check (action->item))
            *state_type = G_VARIANT_TYPE_BOOLEAN;
          else
            *state_type = NULL;
        }

      if (state_hint != NULL)
        {
          if (action->items_by_name != NULL)
            {
              GVariantBuilder builder;
              GHashTableIter iter;
              gpointer key;

              g_variant_builder_init (&builder, G_VARIANT_TYPE_ARRAY);

              g_hash_table_iter_init (&iter, action->items_by_name);
              while (g_hash_table_iter_next (&iter, &key, NULL))
                g_variant_builder_add (&builder, "s", key);

              *state_hint = g_variant_ref_sink (g_variant_builder_end (&builder));
            }
          else if (action->item != NULL && unity_gtk_menu_item_is_check (action->item))
            {
              GVariantBuilder builder;

              g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);

              g_variant_builder_add (&builder, "b", FALSE);
              g_variant_builder_add (&builder, "b", TRUE);

              *state_hint = g_variant_ref_sink (g_variant_builder_end (&builder));
            }
          else
            *state_hint = NULL;
        }

      if (state != NULL)
        {
          if (action->items_by_name != NULL)
            {
              GHashTableIter iter;
              gpointer key;
              gpointer value;

              *state = NULL;

              g_hash_table_iter_init (&iter, action->items_by_name);
              while (*state == NULL && g_hash_table_iter_next (&iter, &key, &value))
                if (unity_gtk_menu_item_is_active (value))
                  *state = g_variant_ref_sink (g_variant_new_string (key));
            }
          else if (action->item != NULL && unity_gtk_menu_item_is_check (action->item))
            *state = g_variant_ref_sink (g_variant_new_boolean (unity_gtk_menu_item_is_active (action->item)));
          else
            *state = NULL;
        }
    }

  return action != NULL;
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

UnityGtkActionGroup *
unity_gtk_action_group_new (void)
{
  return g_object_new (UNITY_GTK_TYPE_ACTION_GROUP,
                       NULL);
}

void
unity_gtk_action_group_connect_shell (UnityGtkActionGroup *group,
                                      UnityGtkMenuShell   *shell)
{
}

void
unity_gtk_action_group_disconnect_shell (UnityGtkActionGroup *group,
                                         UnityGtkMenuShell   *shell)
{
}
