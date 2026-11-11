/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "exporter-muxer.h"

#include "gtkactiontreepriv.h"
#include "gtkactionkey.h"
#include "definitions.h"

#include <string.h>

struct _DBusMenuExporterMuxer
{
  GObject        parent_instance;
  GtkActionTree *tree;
  GtkActionNode *node;
  const gchar  **prefix_cache;
};

static void muxer_action_group_iface_init (GActionGroupInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (DBusMenuExporterMuxer, dbus_menu_exporter_muxer,
                               G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP,
                                                      muxer_action_group_iface_init))

static GtkActionSource *
find_source_for_name (DBusMenuExporterMuxer *self,
                      const char            *full_name,
                      const char           **out_local)
{
  GHashTableIter iter;
  gpointer value;
  gsize n;

  if (self->node == NULL || self->node->sources == NULL || full_name == NULL)
    return NULL;

  g_hash_table_iter_init (&iter, self->node->sources);
  while (g_hash_table_iter_next (&iter, NULL, &value))
    {
      GtkActionSource *source = value;

      n = strlen (source->prefix);
      if (strncmp (full_name, source->prefix, n) == 0 && full_name[n] == '.')
        {
          if (out_local)
            *out_local = full_name + n + 1;
          return source;
        }
    }

  return NULL;
}

static const char *
find_prefix_for_group (DBusMenuExporterMuxer *self,
                       GActionGroup          *group)
{
  GHashTableIter iter;
  gpointer value;

  if (self->node == NULL || self->node->sources == NULL)
    return NULL;

  g_hash_table_iter_init (&iter, self->node->sources);
  while (g_hash_table_iter_next (&iter, NULL, &value))
    {
      GtkActionSource *source = value;
      if (source->group == group)
        return source->prefix;
    }

  return NULL;
}

#define muxer_disconnect_all(group)                                                        \
  G_STMT_START {                                                                            \
    g_signal_handlers_disconnect_by_func_only ((group), on_action_added);                  \
    g_signal_handlers_disconnect_by_func_only ((group), on_action_removed);                \
    g_signal_handlers_disconnect_by_func_only ((group), on_action_enabled_changed);        \
    g_signal_handlers_disconnect_by_func_only ((group), on_action_state_changed);          \
  } G_STMT_END

static void
on_action_added (GActionGroup          *group,
                 const char            *action_name,
                 DBusMenuExporterMuxer *self)
{
  const char *prefix = find_prefix_for_group (self, group);
  char *full;

  if (prefix == NULL)
    return;

  full = g_strconcat (prefix, ".", action_name, NULL);
  g_signal_emit_by_name (self, "action-added", full);
  g_free (full);
}

static void
on_action_removed (GActionGroup          *group,
                   const char            *action_name,
                   DBusMenuExporterMuxer *self)
{
  const char *prefix = find_prefix_for_group (self, group);
  char *full;

  if (prefix == NULL)
    return;

  full = g_strconcat (prefix, ".", action_name, NULL);
  g_signal_emit_by_name (self, "action-removed", full);
  g_free (full);
}

static void
on_action_enabled_changed (GActionGroup          *group,
                           const char            *action_name,
                           gboolean               enabled,
                           DBusMenuExporterMuxer *self)
{
  const char *prefix = find_prefix_for_group (self, group);
  char *full;

  if (prefix == NULL)
    return;

  full = g_strconcat (prefix, ".", action_name, NULL);
  g_signal_emit_by_name (self, "action-enabled-changed", full, enabled);
  g_free (full);
}

static void
on_action_state_changed (GActionGroup          *group,
                         const char            *action_name,
                         GVariant              *state,
                         DBusMenuExporterMuxer *self)
{
  const char *prefix = find_prefix_for_group (self, group);
  char *full;

  if (prefix == NULL)
    return;

  full = g_strconcat (prefix, ".", action_name, NULL);
  g_signal_emit_by_name (self, "action-state-changed", full, state);
  g_free (full);
}

static void
connect_forwarders (DBusMenuExporterMuxer *self,
                    GActionGroup          *group)
{
  g_signal_connect_object (group, "action-added",
                           G_CALLBACK (on_action_added), self, 0);
  g_signal_connect_object (group, "action-removed",
                           G_CALLBACK (on_action_removed), self, 0);
  g_signal_connect_object (group, "action-enabled-changed",
                           G_CALLBACK (on_action_enabled_changed), self, 0);
  g_signal_connect_object (group, "action-state-changed",
                           G_CALLBACK (on_action_state_changed), self, 0);
}

static gchar **
muxer_list_actions (GActionGroup *group)
{
  DBusMenuExporterMuxer *self = DBUS_MENU_EXPORTER_MUXER (group);

  if (self->node == NULL)
    return NULL;

  return gtk_action_node_list_actions (self->node, TRUE);
}

static gboolean
muxer_has_action (GActionGroup *group,
                  const char   *action_name)
{
  DBusMenuExporterMuxer *self = DBUS_MENU_EXPORTER_MUXER (group);
  const char *local = NULL;
  GtkActionSource *source;

  if (!(source = find_source_for_name (self, action_name, &local)))
    return FALSE;

  return g_action_group_has_action (source->group, local);
}

static gboolean
muxer_query_action (GActionGroup        *group,
                    const char          *action_name,
                    gboolean            *enabled,
                    const GVariantType **parameter_type,
                    const GVariantType **state_type,
                    GVariant           **state_hint,
                    GVariant           **state)
{
  DBusMenuExporterMuxer *self = DBUS_MENU_EXPORTER_MUXER (group);
  const char *local = NULL;
  GtkActionSource *source;

  if (!(source = find_source_for_name (self, action_name, &local)))
    return FALSE;

  return g_action_group_query_action (source->group, local,
                                      enabled, parameter_type, state_type,
                                      state_hint, state);
}

static void
muxer_activate_action (GActionGroup *group,
                       const char   *action_name,
                       GVariant     *parameter)
{
  DBusMenuExporterMuxer *self = DBUS_MENU_EXPORTER_MUXER (group);
  const char *local = NULL;
  GtkActionSource *source;

  if ((source = find_source_for_name (self, action_name, &local)))
    g_action_group_activate_action (source->group, local, parameter);
}

static void
muxer_change_action_state (GActionGroup *group,
                           const char   *action_name,
                           GVariant     *state)
{
  DBusMenuExporterMuxer *self = DBUS_MENU_EXPORTER_MUXER (group);
  const char *local = NULL;
  GtkActionSource *source;

  if ((source = find_source_for_name (self, action_name, &local)))
    g_action_group_change_action_state (source->group, local, state);
}

static void
muxer_action_group_iface_init (GActionGroupInterface *iface)
{
  iface->list_actions        = muxer_list_actions;
  iface->has_action          = muxer_has_action;
  iface->query_action        = muxer_query_action;
  iface->activate_action     = muxer_activate_action;
  iface->change_action_state = muxer_change_action_state;
}

DBusMenuExporterMuxer *
dbus_menu_exporter_muxer_new (void)
{
  return g_object_new (dbus_menu_exporter_muxer_get_type(), NULL);
}

void
dbus_menu_exporter_muxer_insert (DBusMenuExporterMuxer *self,
                                 const gchar           *prefix,
                                 GActionGroup          *action_group)
{
  GtkActionSource *existing;
  char **names;
  guint i;

  g_return_if_fail (DBUS_MENU_IS_EXPORTER_MUXER (self));
  g_return_if_fail (prefix != NULL && strchr (prefix, '.') == NULL);
  g_return_if_fail (G_IS_ACTION_GROUP (action_group));

  if (self->node == NULL)
    return;

  existing = self->node->sources
           ? g_hash_table_lookup (self->node->sources, prefix)
           : NULL;

  if (existing != NULL && existing->group == action_group)
    return;

  if (existing != NULL)
    dbus_menu_exporter_muxer_remove (self, prefix);

  gtk_action_node_insert_group (self->node, prefix, action_group);

  connect_forwarders (self, action_group);

  names = g_action_group_list_actions (action_group);
  for (i = 0; names[i] != NULL; i++)
    {
      char *full = g_strconcat (prefix, ".", names[i], NULL);
      g_signal_emit_by_name (self, "action-added", full);
      g_free (full);
    }
  g_strfreev (names);
}

void
dbus_menu_exporter_muxer_remove (DBusMenuExporterMuxer *self,
                                 const gchar           *prefix)
{
  GtkActionSource *source;
  GActionGroup *group;
  char **names;
  guint i;

  g_return_if_fail (DBUS_MENU_IS_EXPORTER_MUXER (self));
  g_return_if_fail (prefix != NULL);

  if (self->node == NULL || self->node->sources == NULL)
    return;

  source = g_hash_table_lookup (self->node->sources, prefix);
  if (source == NULL)
    return;

  group = g_object_ref (source->group);
  names = g_action_group_list_actions (group);

  muxer_disconnect_all (group);

  gtk_action_node_remove_group (self->node, prefix);

  for (i = 0; names[i] != NULL; i++)
    {
      char *full = g_strconcat (prefix, ".", names[i], NULL);
      g_signal_emit_by_name (self, "action-removed", full);
      g_free (full);
    }
  g_strfreev (names);
  g_object_unref (group);
}

const gchar **
dbus_menu_exporter_muxer_list_prefixes (DBusMenuExporterMuxer *self)
{
  g_return_val_if_fail (DBUS_MENU_IS_EXPORTER_MUXER (self), NULL);

  if (self->node == NULL || self->node->sources == NULL)
    return g_new0 (const gchar *, 1);

  return (const gchar **) g_hash_table_get_keys_as_array (self->node->sources, NULL);
}

GActionGroup *
dbus_menu_exporter_muxer_lookup (DBusMenuExporterMuxer *self,
                                 const gchar           *prefix)
{
  GtkActionSource *source;

  g_return_val_if_fail (DBUS_MENU_IS_EXPORTER_MUXER (self), NULL);
  g_return_val_if_fail (prefix != NULL, NULL);

  source = (self->node && self->node->sources)
         ? g_hash_table_lookup (self->node->sources, prefix)
         : NULL;

  return source ? source->group : NULL;
}

GtkActionNode *
dbus_menu_exporter_muxer_get_node (DBusMenuExporterMuxer *self)
{
  g_return_val_if_fail (DBUS_MENU_IS_EXPORTER_MUXER (self), NULL);
  return self->node;
}

static void
dbus_menu_exporter_muxer_finalize (GObject *object)
{
  DBusMenuExporterMuxer *self = DBUS_MENU_EXPORTER_MUXER (object);

  if (self->node != NULL)
    {
      GtkActionNode *node = self->node;
      self->node = NULL;
      gtk_action_node_remove (node);
    }

  if (self->tree != NULL)
    {
      gtk_action_tree_free (self->tree);
      self->tree = NULL;
    }

  G_OBJECT_CLASS (dbus_menu_exporter_muxer_parent_class)->finalize (object);
}

static void
dbus_menu_exporter_muxer_class_init (DBusMenuExporterMuxerClass *klass)
{
  G_OBJECT_CLASS (klass)->finalize = dbus_menu_exporter_muxer_finalize;
}

static void
dbus_menu_exporter_muxer_init (DBusMenuExporterMuxer *self)
{
  self->tree = gtk_action_tree_new ();
  self->node = gtk_action_tree_add_synthetic (self->tree, self);
}
