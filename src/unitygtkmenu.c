#include "unitygtkmenu.h"

#define UNITY_GTK_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), UNITY_GTK_TYPE_MENU, UnityGtkMenuPrivate))

typedef struct _UnityGtkMenuItem UnityGtkMenuItem;

struct _UnityGtkMenuItem
{
  GtkMenuItem  *menu_item;
  UnityGtkMenu *submenu;
};

struct _UnityGtkMenuPrivate
{
  GtkMenuShell *menu_shell;
  guint         shell_offset;
  GPtrArray    *menu_items;
  UnityGtkMenu *section;
  guint         section_valid : 1;
};

enum
{
  PROP_0,
  PROP_MENU_SHELL,
  PROP_SHELL_OFFSET,
  PROP_MENU_ITEMS,
  PROP_SECTION,
  N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL };

G_DEFINE_TYPE (UnityGtkMenu, unity_gtk_menu, G_TYPE_MENU_MODEL);

static UnityGtkMenuItem *
unity_gtk_menu_item_new (GtkMenuItem *menu_item)
{
  UnityGtkMenuItem *item = g_slice_new0 (UnityGtkMenuItem);

  item->menu_item = menu_item;

  return item;
}

static void
unity_gtk_menu_item_free (gpointer data)
{
  UnityGtkMenuItem *menu_item = data;

  g_return_if_fail (menu_item != NULL);

  if (menu_item->submenu != NULL)
    g_object_unref (menu_item->submenu);

  g_slice_free (UnityGtkMenuItem, menu_item);
}

static void
unity_gtk_menu_dispose (GObject *object)
{
  UnityGtkMenu *menu;
  UnityGtkMenuPrivate *priv;

  g_warn_if_fail (object != NULL);

  menu = UNITY_GTK_MENU (object);
  priv = menu->priv;

  if (priv->section != NULL)
    {
      g_object_unref (priv->section);
      priv->section = NULL;
    }

  if (priv->menu_items != NULL)
    {
      g_ptr_array_free (priv->menu_items, TRUE);
      priv->menu_items = NULL;
    }

  G_OBJECT_CLASS (unity_gtk_menu_parent_class)->dispose (object);
}

static GPtrArray *
unity_gtk_menu_get_menu_items (UnityGtkMenu *menu)
{
  UnityGtkMenuPrivate *priv;

  g_return_val_if_fail (menu != NULL, NULL);

  priv = menu->priv;

  if (priv->menu_shell == NULL)
    {
      if (priv->menu_items == NULL)
        priv->menu_items = g_ptr_array_new ();
      else
        g_ptr_array_set_size (priv->menu_items, 0);
    }

  if (priv->menu_items == NULL)
    {
      GList *start = gtk_container_get_children (GTK_CONTAINER (priv->menu_shell));
      GList *finish;
      GList *iter;
      guint n;
      guint i;

      for (i = 0; i < priv->shell_offset; i++)
        start = g_list_next (start);

      finish = start;

      for (n = 0; finish != NULL && !GTK_IS_SEPARATOR_MENU_ITEM (finish->data); n++)
        finish = g_list_next (finish);

      priv->menu_items = g_ptr_array_new_full (n, unity_gtk_menu_item_free);

      for (iter = start; iter != finish; iter = g_list_next (iter))
        g_ptr_array_add (priv->menu_items, unity_gtk_menu_item_new (iter->data));
    }

  return priv->menu_items;
}

static UnityGtkMenu *
unity_gtk_menu_new_with_offset (GtkMenuShell *menu_shell,
                                guint         shell_offset)
{
  return g_object_new (UNITY_GTK_TYPE_MENU,
                       "menu-shell", menu_shell,
                       "shell-offset", shell_offset,
                       NULL);
}

static UnityGtkMenu *
unity_gtk_menu_get_section (UnityGtkMenu *menu)
{
  UnityGtkMenuPrivate *priv;

  g_return_val_if_fail (menu != NULL, NULL);

  priv = menu->priv;

  if (!priv->section_valid)
    {
      if (priv->section != NULL)
        {
          g_object_unref (priv->section);
          priv->section = NULL;
        }

      if (priv->menu_shell != NULL)
        {
          GList *start = gtk_container_get_children (GTK_CONTAINER (priv->menu_shell));
          guint i;

          for (i = 0; i < priv->shell_offset; i++)
            start = g_list_next (start);

          while (start != NULL && !GTK_IS_SEPARATOR_MENU_ITEM (start->data))
            {
              start = g_list_next (start);
              i++;
            }

          if (start != NULL)
            priv->section = unity_gtk_menu_new_with_offset (priv->menu_shell, i + 1);
        }

      priv->section_valid = TRUE;
    }

  return priv->section;
}

static void
unity_gtk_menu_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  UnityGtkMenu *self;
  UnityGtkMenuPrivate *priv;

  g_return_if_fail (object != NULL);
  g_return_if_fail (value != NULL);
  g_return_if_fail (pspec != NULL);

  self = UNITY_GTK_MENU (object);
  priv = self->priv;

  switch (property_id)
    {
    case PROP_MENU_SHELL:
      g_value_take_object (value, priv->menu_shell);
      break;

    case PROP_SHELL_OFFSET:
      g_value_set_uint (value, priv->shell_offset);
      break;

    case PROP_MENU_ITEMS:
      g_value_set_pointer (value, unity_gtk_menu_get_menu_items (self));
      break;

    case PROP_SECTION:
      g_value_set_object (value, unity_gtk_menu_get_section (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
unity_gtk_menu_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  UnityGtkMenu *self;
  UnityGtkMenuPrivate *priv;

  g_return_if_fail (object != NULL);
  g_return_if_fail (value != NULL);
  g_return_if_fail (pspec != NULL);

  self = UNITY_GTK_MENU (object);
  priv = self->priv;

  switch (property_id)
    {
    case PROP_MENU_SHELL:
      priv->menu_shell = g_value_get_object (value);
      break;

    case PROP_SHELL_OFFSET:
      priv->shell_offset = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
unity_gtk_menu_is_mutable (GMenuModel *model)
{
  g_return_val_if_fail (model != NULL, FALSE);

  /* model->menu_shell can change at any time. */
  return FALSE;
}

static gint
unity_gtk_menu_get_n_items (GMenuModel *model)
{
  UnityGtkMenu *menu = UNITY_GTK_MENU (model);

  g_return_val_if_fail (model != NULL, 0);

  return unity_gtk_menu_get_menu_items (menu)->len + (unity_gtk_menu_get_section (menu) != NULL ? 1 : 0);
}

static void
unity_gtk_menu_get_item_attributes (GMenuModel  *model,
                                    gint         item_index,
                                    GHashTable **attributes)
{
  UnityGtkMenu *menu;
  GPtrArray *menu_items;
  GHashTable *hash_table;

  g_return_if_fail (model != NULL);
  g_return_if_fail (attributes != NULL);
  g_return_if_fail (0 <= item_index && item_index < unity_gtk_menu_get_n_items (model));

  menu = UNITY_GTK_MENU (model);
  menu_items = unity_gtk_menu_get_menu_items (menu);
  hash_table = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify) g_variant_unref);

  if (item_index < menu_items->len)
    {
      UnityGtkMenuItem *item = g_ptr_array_index (menu_items, item_index);

      if (item != NULL)
        {
          GtkMenuItem *menu_item = item->menu_item;

          if (menu_item != NULL)
            {
              const gchar *label = gtk_menu_item_get_label (menu_item);

              g_hash_table_insert (hash_table, G_MENU_ATTRIBUTE_LABEL, g_variant_new_string (label));
            }
          else
            g_assert_not_reached ();
        }
      else
        g_assert_not_reached ();
    }

  *attributes = hash_table;
}

static void
unity_gtk_menu_get_item_links (GMenuModel  *model,
                               gint         item_index,
                               GHashTable **links)
{
  UnityGtkMenu *menu;
  GPtrArray *menu_items;
  GHashTable *hash_table;

  g_return_if_fail (model != NULL);
  g_return_if_fail (links != NULL);
  g_return_if_fail (0 <= item_index && item_index < unity_gtk_menu_get_n_items (model));

  menu = UNITY_GTK_MENU (model);
  menu_items = unity_gtk_menu_get_menu_items (menu);
  hash_table = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);

  if (item_index < menu_items->len)
    {
      UnityGtkMenuItem *menu_item = g_ptr_array_index (menu_items, item_index);

      if (menu_item != NULL)
        {
          UnityGtkMenu *submenu = menu_item->submenu;

          if (submenu != NULL)
            g_hash_table_insert (hash_table, G_MENU_LINK_SUBMENU, g_object_ref (submenu));
        }
      else
        g_assert_not_reached ();
    }
  else
    {
      UnityGtkMenu *section = unity_gtk_menu_get_section (menu);

      if (section != NULL)
        g_hash_table_insert (hash_table, G_MENU_LINK_SECTION, g_object_ref (section));
    }

  *links = hash_table;
}

static void
unity_gtk_menu_class_init (UnityGtkMenuClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GMenuModelClass *menu_model_class = G_MENU_MODEL_CLASS (klass);

  object_class->dispose = unity_gtk_menu_dispose;
  object_class->get_property = unity_gtk_menu_get_property;
  object_class->set_property = unity_gtk_menu_set_property;
  menu_model_class->is_mutable = unity_gtk_menu_is_mutable;
  menu_model_class->get_n_items = unity_gtk_menu_get_n_items;
  menu_model_class->get_item_attributes = unity_gtk_menu_get_item_attributes;
  menu_model_class->get_item_links = unity_gtk_menu_get_item_links;

  properties[PROP_MENU_SHELL] = g_param_spec_object ("menu-shell",
                                                     "Menu shell",
                                                     "Menu shell",
                                                     GTK_TYPE_MENU_SHELL,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_SHELL_OFFSET] = g_param_spec_uint ("shell-offset",
                                                     "Menu shell offset",
                                                     "Index of first GtkMenuItem",
                                                     0,
                                                     G_MAXUINT,
                                                     0,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY);
  properties[PROP_MENU_ITEMS] = g_param_spec_pointer ("menu-items",
                                                      "Menu items",
                                                      "Menu items",
                                                      G_PARAM_READABLE);
  properties[PROP_SECTION] = g_param_spec_object ("section",
                                                  "Menu section",
                                                  "Menu section",
                                                  UNITY_GTK_TYPE_MENU,
                                                  G_PARAM_READABLE);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);

  g_type_class_add_private (klass, sizeof (UnityGtkMenuPrivate));
}

static void
unity_gtk_menu_init (UnityGtkMenu *self)
{
  self->priv = UNITY_GTK_MENU_GET_PRIVATE (self);
}

UnityGtkMenu *
unity_gtk_menu_new (GtkMenuShell *menu_shell)
{
  return unity_gtk_menu_new_with_offset (menu_shell, 0);
}
