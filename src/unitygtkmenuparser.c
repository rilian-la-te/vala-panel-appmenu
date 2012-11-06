/*
 * Copyright © 2011 Canonical Limited
 *
 * All rights reserved.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include "unitygtkmenuparser.h"

/**
 * The goal is to be aware of all GtkMenuBar instances and the following
 * things about them:
 *
 *   - the toplevel GdkWindow that they are inside of (in order to set
 *     the appropriate X properties)
 *
 *   - the GtkMenuItem contents (recursively)
 *
 * We accomplish this by way of several mechanisms.
 *
 * The primary mechanism to accomplish this is that we intercept all
 * GtkMenuShell vcalls to the 'insert' function by hijacking the class
 * structure and patching in our own function.  This requires that each
 * GtkMenuBar must first contain at least one item in order to be
 * noticed, but this is a pretty reasonable assumption to make.
 *
 * The 'insert' call gives us the GtkMenuShell and the GtkMenuItem, so
 * we can begin to monitor both.
 *
 * For each GtkMenuShell we make a GMenuModel implementation to reflect
 * the contents of that menu.  We monitor for new additions by way of
 * the 'insert' hack, and we notice removes and changes in properties of
 * the items by monitoring the GObject "notify" signal on each
 * GtkMenuItem.
 *
 * Note that the "parent" property of a GtkWidget is *always* unset
 * before the widget is destroyed so this is a reliable method of
 * safely detecting removes.
 *
 * For the menu shell we use the qdata destroy notify to discover the
 * its destruction.  This is because at the time the first item is added
 * it is quite likely that the shell has no parent (and possibly even a
 * floating reference count).
 *
 * Finally, for GtkMenuBar itself, we monitor realize/unrealize in order
 * to be able to discover the appropriate GdkWindow to attach the X
 * properties to.
 *
 * One thing that is very important is that we may not hold any strong
 * references on any Gtk widgetry.  Widget lifecycle must continue to be
 * exactly the same as if we were not here, otherwise we could easily
 * get into situations where we introduce reference cycles.
 *
 * There are three approaches under consideration here:
 *
 *   - first, we monitor every single GtkMenuShell that we know about,
 *     from the first insertion.  Lots of overhead here, but simple.
 *
 *   - second, we monitor only GtkMenuBar instances.  When we first
 *     discover a GtkMenuItem being added to a GtkMenuBar then we
 *     recurse over the contents of that item.
 *
 *   - third, like the second, we only care about GtkMenuBar, but we are
 *     more lazy.  We don't do the recursion until the consumer of the
 *     model requests it.
 *
 * I think I like #3.  It seems clever.  I like clever.
 **/

typedef GMenuModelClass UnityGtkMenuParserClass;
struct _UnityGtkMenuParser
{
  GMenuModel parent_instance;

  GSimpleActionGroup *actions;
  GtkMenuShell *menu_shell;
  gulong        handler_id;
  GSequence    *items;
};

/* UnityGtkMenuParser can have a few internal states:
 *
 *   - when first created, menu_shell is set but items is %NULL.  This
 *     is the "lazy" mode where nothing is queried yet.
 *
 *   - on first query, items is initialised from menu_shell
 *
 *   - the model can possibly remain after the menu_shell which it
 *     represents is no longer part of the tree (because of external
 *     references).  In this case, menu_shell is set to %NULL and items
 *     is replaced with an empty array
 *
 *   - if menu_shell is already %NULL for the initialisation then items
 *     is initialised as an empty array
 *
 * Of course we only need to monitor for changes when items is non-%NULL
 * (since otherwise nobody is watching us anyway).
 */

typedef struct
{
  UnityGtkMenuParser *parent;
  GtkMenuItem *menu_item;
  gulong       handler_id;
  UnityGtkMenuParser *submenu;
  GAction *action;
  gulong action_handler_id;
} UnityGtkMenuItem;

G_DEFINE_TYPE (UnityGtkMenuParser, unity_gtk_menu_parser, G_TYPE_MENU_MODEL);

static void
unity_gtk_menu_item_notify (GObject    *object,
                            GParamSpec *pspec,
                            gpointer    user_data)
{
  static const gchar *interned_submenu;
  static const gchar *interned_parent;
  static const gchar *interned_active;
  GSequenceIter *iter = user_data;
  gboolean removed = FALSE;
  UnityGtkMenuParser *parser;
  UnityGtkMenuItem *item;
  gint position;

  if G_UNLIKELY (interned_submenu == NULL)
    interned_submenu = g_intern_static_string ("submenu");

  if G_UNLIKELY (interned_parent == NULL)
    interned_parent = g_intern_static_string ("parent");

  if G_UNLIKELY (interned_active == NULL)
    interned_active = g_intern_static_string ("active");

  item = g_sequence_get (iter);
  position = g_sequence_iter_get_position (iter);
  parser = item->parent;
  g_assert (UNITY_GTK_IS_MENU_PARSER (parser));

  g_print ("Change %s\n", pspec->name);

  if (pspec->name == interned_active && GTK_IS_CHECK_MENU_ITEM (object))
    {
      const GVariantType *state_type;
      gboolean active;
      GVariant *state;

      active = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (object));


      g_assert (item->action != NULL);
      state_type = g_action_get_state_type (item->action);
      g_assert (state_type != NULL);

      if (g_variant_type_equal (state_type, G_VARIANT_TYPE_STRING))
        state = g_variant_new_string (active ? "1" : "");
      else
        state = g_variant_new_boolean (active);

      g_simple_action_set_state (G_SIMPLE_ACTION (item->action), state);
      g_print (">> %s -> %s\n", g_action_get_name (item->action), g_variant_print (state, FALSE));
      return;
    }

  if (pspec->name == interned_parent)
    {
      GtkWidget *parent;

      parent = gtk_widget_get_parent (GTK_WIDGET (object));

      if (parent != (gpointer) parser->menu_shell)
        g_sequence_remove (iter);

      removed = TRUE;
    }

  else if (pspec->name == interned_submenu)
    {
      GtkWidget *submenu;

      submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (object));

      /* out with the old */
      if (item->submenu != NULL)
        {
          unity_gtk_menu_parser_destroy (item->submenu);
          g_object_unref (item->submenu);
          item->submenu = NULL;
        }

      /* in with the new */
      if (submenu != NULL)
        {
          g_assert (GTK_IS_MENU (submenu));
          item->submenu = unity_gtk_menu_parser_new (GTK_MENU_SHELL (submenu));
          item->submenu->actions = g_object_ref (parser->actions);
        }
    }

  g_menu_model_items_changed (G_MENU_MODEL (parser), position, 1, !removed);
}

static void
unity_gtk_menu_item_free (gpointer data)
{
  UnityGtkMenuItem *item = data;

  g_signal_handler_disconnect (item->menu_item, item->handler_id);
  g_object_unref (item->menu_item);

  if (item->submenu != NULL)
    g_object_unref (item->submenu);

  if (item->action)
    {
      g_simple_action_group_remove (item->parent->actions, g_action_get_name (item->action));
      g_signal_handler_disconnect (item->action, item->action_handler_id);
      g_object_unref (item->action);
    }

  g_slice_free (UnityGtkMenuItem, item);
}

static gchar *
unity_gtk_menu_item_find_action_name (GActionGroup *action_group,
                                      const gchar  *ideal)
{
  gint i;

  if (ideal != NULL)
    {
      if (!g_action_group_has_action (action_group, ideal))
        return g_strdup (ideal);
    }
  else
    ideal = "unnamed";

  for (i = 0; i < 1000000; i++)
    {
      gchar *candidate;

      candidate = g_strdup_printf ("%s (%d)", ideal, i);
      if (!g_action_group_has_action (action_group, candidate))
        return candidate;
      g_free (candidate);
    }

  g_assert_not_reached ();
}

static GAction *
unity_gtk_menu_item_create_action (GtkMenuItem        *menu_item,
                                   GSimpleActionGroup *action_group)
{
  GSimpleAction *action;
  GtkAction *gtk_action;
  const gchar *ideal;
  GVariant *state;
  gchar *name;

  gtk_action = gtk_activatable_get_related_action (GTK_ACTIVATABLE (menu_item));
  if (gtk_action != NULL)
    ideal = gtk_action_get_name (gtk_action);
  else
    ideal = gtk_menu_item_get_label (menu_item);

  if (GTK_IS_CHECK_MENU_ITEM (menu_item))
    {
      gboolean active;

      active = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menu_item));

      if (GTK_IS_RADIO_MENU_ITEM (menu_item))
        state = g_variant_new_string (active ? "1" : "");
      else
        state = g_variant_new_boolean (active);
    }
  else
    state = NULL;

  name = unity_gtk_menu_item_find_action_name (G_ACTION_GROUP (action_group), ideal);

  if (state != NULL)
    action = g_simple_action_new_stateful (name, NULL, state);

  else
    action = g_simple_action_new (name, NULL);

  g_free (name);

  return G_ACTION (action);
}

static void
unity_gtk_menu_item_activate (GSimpleAction *action,
                              GVariant      *parameter,
                              gpointer       user_data)
{
  UnityGtkMenuItem *item = user_data;

  gtk_menu_item_activate (item->menu_item);
}

static GSequenceIter *
unity_gtk_menu_item_insert (GSequenceIter      *iter,
                            UnityGtkMenuParser *parent,
                            GtkMenuItem        *menu_item)
{
  GtkWidget *submenu;
  UnityGtkMenuItem *item;

  item = g_slice_new (UnityGtkMenuItem);
  iter = g_sequence_insert_before (iter, item);
  item->menu_item = g_object_ref (menu_item);
  item->parent = parent;
  item->handler_id = g_signal_connect (menu_item, "notify", G_CALLBACK (unity_gtk_menu_item_notify), iter);

  g_assert (gtk_widget_get_parent (GTK_WIDGET (menu_item)) == GTK_WIDGET (parent->menu_shell));

  if (!GTK_IS_SEPARATOR_MENU_ITEM (menu_item))
    {
      submenu = gtk_menu_item_get_submenu (menu_item);

      if (submenu != NULL)
        {
          g_assert (GTK_IS_MENU (submenu));
          item->submenu = unity_gtk_menu_parser_new (GTK_MENU_SHELL (submenu));
          item->submenu->actions = g_object_ref (parent->actions);
          item->action = NULL;
        }
      else
        {
          item->action = unity_gtk_menu_item_create_action (menu_item, parent->actions);
          item->action_handler_id =
            g_signal_connect (item->action, "activate", G_CALLBACK (unity_gtk_menu_item_activate), item);
          g_simple_action_group_insert (parent->actions, item->action);
          item->submenu = NULL;
        }
    }
  else
    {
      item->action = NULL;
      item->submenu = NULL;
    }

  return iter;
}

static void
unity_gtk_menu_parser_populate (GtkWidget *widget,
                                gpointer   user_data)
{
  UnityGtkMenuParser *parser = user_data;
  GtkMenuItem *menu_item;

  menu_item = GTK_MENU_ITEM (widget);
  g_assert (menu_item != NULL);

  unity_gtk_menu_item_insert (g_sequence_get_end_iter (parser->items), parser, menu_item);
}

static void
unity_gtk_menu_parser_inserted (GtkMenuShell *menu_shell,
                                GtkWidget    *child,
                                gint          position,
                                gpointer      user_data)
{
  UnityGtkMenuParser *parser = user_data;

  if (parser != NULL && parser->items != NULL)
    {
      GSequenceIter *iter;

      iter = g_sequence_get_iter_at_pos (parser->items, position);
      iter = unity_gtk_menu_item_insert (iter, parser, GTK_MENU_ITEM (child));
      g_menu_model_items_changed (G_MENU_MODEL (parser), position, 0, 1);
    }
}

static void
unity_gtk_menu_parser_ensure_alive (UnityGtkMenuParser *parser)
{
  if (parser->items != NULL)
    return;

  if (parser->actions == NULL)
    parser->actions = g_simple_action_group_new ();

  parser->items = g_sequence_new (unity_gtk_menu_item_free);

  if (parser->menu_shell != NULL)
    {
      gtk_container_foreach (GTK_CONTAINER (parser->menu_shell), unity_gtk_menu_parser_populate, parser);
      parser->handler_id = g_signal_connect (parser->menu_shell, "insert",
                                             G_CALLBACK (unity_gtk_menu_parser_inserted), parser);
    }
}

static gboolean
unity_gtk_menu_parser_is_mutable (GMenuModel *model)
{
  return TRUE;
}

static gint
unity_gtk_menu_parser_get_n_items (GMenuModel *model)
{
  UnityGtkMenuParser *parser = (UnityGtkMenuParser *) model;

  unity_gtk_menu_parser_ensure_alive (parser);

  return g_sequence_get_length (parser->items);
}

static void
unity_gtk_menu_parser_get_item_attributes (GMenuModel  *model,
                                           gint         item_index,
                                           GHashTable **quark_table,
                                           GHashTable **string_table,
                                           GVariant   **dictionary)
{
  UnityGtkMenuParser *parser = (UnityGtkMenuParser *) model;
  GSequenceIter *iter;
  UnityGtkMenuItem *item;

  iter = g_sequence_get_iter_at_pos (parser->items, item_index);
  item = g_sequence_get (iter);

  *quark_table = g_hash_table_new (NULL, NULL);

  if (GTK_IS_SEPARATOR_MENU_ITEM (item->menu_item))
    {
      g_hash_table_insert (*quark_table, GINT_TO_POINTER(g_quark_from_static_string ("type")), g_variant_ref_sink (g_variant_new_string ("separator")));
    }
  else if (!gtk_widget_get_visible (GTK_WIDGET (item->menu_item)))
    {
      g_hash_table_insert (*quark_table, GINT_TO_POINTER(g_quark_from_static_string ("type")), g_variant_ref_sink (g_variant_new_string ("placeholder")));

    }
  else
    {
      const gchar *label;
      label = gtk_menu_item_get_label (item->menu_item);
      if (label)
        g_hash_table_insert (*quark_table, GINT_TO_POINTER (G_MENU_ATTRIBUTE_LABEL), g_variant_ref_sink (g_variant_new_string (label)));
      if (item->action)
        g_hash_table_insert (*quark_table, GINT_TO_POINTER (G_MENU_ATTRIBUTE_ACTION), g_variant_ref_sink (g_variant_new_string (g_action_get_name (item->action))));
    }
}

static void
unity_gtk_menu_parser_get_item_links (GMenuModel  *model,
                                      gint         item_index,
                                      GHashTable **quark_table,
                                      GHashTable **string_table)
{
  UnityGtkMenuParser *parser = (UnityGtkMenuParser *) model;
  UnityGtkMenuItem *item;
  GSequenceIter *iter;

  iter = g_sequence_get_iter_at_pos (parser->items, item_index);
  item = g_sequence_get (iter);

  *quark_table = g_hash_table_new_full (NULL, NULL, NULL, g_object_unref);

  if (item->submenu)
    g_hash_table_insert (*quark_table, GINT_TO_POINTER (G_MENU_LINK_SUBMENU), g_object_ref (item->submenu));
}

static void
unity_gtk_menu_parser_finalize (GObject *object)
{
  UnityGtkMenuParser *parser = (UnityGtkMenuParser *) object;

  if (parser->items)
    g_sequence_free (parser->items);

  if (parser->actions)
    g_object_unref (parser->actions);

  if (parser->menu_shell)
    {
      g_signal_handler_disconnect (parser->menu_shell, parser->handler_id);
      g_object_unref (parser->menu_shell);
    }

  G_OBJECT_CLASS (unity_gtk_menu_parser_parent_class)
    ->finalize (object);
}

static void
unity_gtk_menu_parser_init (UnityGtkMenuParser *parser)
{
}

static void
unity_gtk_menu_parser_class_init (UnityGtkMenuParserClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = unity_gtk_menu_parser_finalize;

  class->is_mutable = unity_gtk_menu_parser_is_mutable;
  class->get_n_items = unity_gtk_menu_parser_get_n_items;
  class->get_item_attributes = unity_gtk_menu_parser_get_item_attributes;
  class->get_item_links = unity_gtk_menu_parser_get_item_links;
}

/**
 * unity_gtk_menu_parser_new:
 * @menu_shell: a #GtkMenuShell
 *
 * Creates a #GMenuModel that describes the contents of a #GtkMenuShell.
 *
 * In effect, this is done by "parsing" the #GtkMenuShell and
 * (recursively) describing it in the form of a #GMenuModel (which is
 * what this function returns).
 *
 * The model holds a strong reference on @menu_shell.  You can have this
 * reference dropped by calling unity_gtk_menu_parser_destroy().
 *
 * Returns: a #UnityGtkMenuParser describing @menu_shell
 **/
UnityGtkMenuParser *
unity_gtk_menu_parser_new (GtkMenuShell *menu_shell)
{
  UnityGtkMenuParser *parser;

  g_return_val_if_fail (GTK_IS_MENU_SHELL (menu_shell), NULL);

  parser = g_object_new (UNITY_GTK_TYPE_MENU_PARSER, NULL);
  parser->menu_shell = g_object_ref (menu_shell);
  parser->items = NULL;

  return parser;
}

/**
 * unity_gtk_menu_parser_destroy:
 * @parser: a #UnityGtkMenuParser
 *
 * Requests that @parser drop its reference on the #GtkMenuShell that it
 * was created with.
 *
 * A signal is emitted to indicate that all items have been removed from
 * the model.  All future queries will indicate an empty menu.
 *
 * It is safe (but meaningless) to call this function multiple times.
 **/
void
unity_gtk_menu_parser_destroy (UnityGtkMenuParser *parser)
{
  if (parser->items)
    {
      gint n_items;

      n_items = g_sequence_get_length (parser->items);

      if (n_items > 0)
        {
          g_sequence_remove_range (g_sequence_get_begin_iter (parser->items),
                                   g_sequence_get_end_iter (parser->items));
          g_menu_model_items_changed (G_MENU_MODEL (parser), 0, n_items, 0);
        }
    }

  if (parser->handler_id)
    {
      g_signal_handler_disconnect (parser->menu_shell, parser->handler_id);
      parser->handler_id = 0;
    }

  if (parser->menu_shell)
    {
      g_object_unref (parser->menu_shell);
      parser->menu_shell = NULL;
    }
}

GActionGroup *
unity_gtk_menu_parser_get_action_group (UnityGtkMenuParser *parser)
{
  if (parser->actions == NULL)
    parser->actions = g_simple_action_group_new ();

  return G_ACTION_GROUP (parser->actions);
}
