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

G_DEFINE_TYPE (UnityGtkMenu, unity_gtk_menu, G_TYPE_MENU);

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

static gboolean
unity_gtk_menu_is_mutable (GMenuModel *model)
{
  g_return_val_if_fail (model != NULL, FALSE);

  /* model->menu_shell can change at any time. */
  return FALSE;
}

static UnityGtkMenuItem *
unity_gtk_menu_item_new (GtkMenuItem *menu_item)
{
  UnityGtkMenuItem *item = g_slice_new0 (UnityGtkMenuItem);

  item->menu_item = menu_item;

  return item;
}

static void
unity_gtk_menu_item_free (UnityGtkMenuItem *menu_item)
{
  g_return_if_fail (menu_item != NULL);

  if (menu_item->submenu != NULL)
    g_object_unref (menu_item->submenu);

  g_slice_free (UnityGtkMenuItem, menu_item);
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
            {
              priv->section = unity_gtk_menu_new (priv->menu_shell);
              priv->section->shell_offset = i + 1;
            }
        }

      priv->section_valid = TRUE;
    }

  return priv->section;
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
}

static GMenuAttributeIter *
unity_gtk_menu_iterate_item_attributes (GMenuModel *model,
                                        gint        item_index)
{
}

static GVariant *
unity_gtk_menu_get_item_attribute_value (GMenuModel         *model,
                                         gint                item_index,
                                         const gchar        *attribute,
                                         const GVariantType *expected_type)
{
}

static void
unity_gtk_menu_get_item_links (GMenuModel  *model,
                               gint         item_index,
                               GHashTable **links)
{
}

static GMenuLinkIter *
unity_gtk_menu_iterate_item_links (GMenuModel *model,
                                   gint        item_index)
{
}

static GMenuModel *
unity_gtk_menu_get_item_link (GMenuModel  *model,
                              gint         item_index,
                              const gchar *link)
{
}

static void
unity_gtk_menu_class_init (UnityGtkMenuClass *klass)
{
  G_OBJECT_CLASS (klass)->dispose                  = unity_gtk_menu_dispose;
  G_MENU_CLASS   (klass)->is_mutable               = unity_gtk_menu_is_mutable;
  G_MENU_CLASS   (klass)->get_n_items              = unity_gtk_menu_get_n_items;
  G_MENU_CLASS   (klass)->get_item_attributes      = unity_gtk_menu_get_item_attributes;
  G_MENU_CLASS   (klass)->iterate_item_attributes  = unity_gtk_menu_iterate_item_attributes;
  G_MENU_CLASS   (klass)->get_item_attribute_value = unity_gtk_menu_get_item_attribute_value;
  G_MENU_CLASS   (klass)->get_item_links           = unity_gtk_menu_get_item_links;
  G_MENU_CLASS   (klass)->iterate_item_links       = unity_gtk_menu_iterate_item_links;
  G_MENU_CLASS   (klass)->get_item_link            = unity_gtk_menu_get_item_link;

  g_type_class_add_private (klass, sizeof (UnityGtkMenuPrivate));
}

static void
unity_gtk_menu_init (UnityGtkMenu *self)
{
}

UnityGtkMenu *
unity_gtk_menu_new (GtkMenuShell *menu_shell)
{
  return g_object_new (UNITY_GTK_TYPE_MENU,
                       "menu-shell", menu_shell,
                       NULL);
}
