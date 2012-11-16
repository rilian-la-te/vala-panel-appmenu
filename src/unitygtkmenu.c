#include "unitygtkmenu.h"

#define UNITY_GTK_TYPE_MENU_SECTION            (unity_gtk_menu_section_get_type ())
#define UNITY_GTK_MENU_SECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_GTK_TYPE_MENU_SECTION, UnityGtkMenuSection))
#define UNITY_GTK_IS_MENU_SECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_GTK_TYPE_MENU_SECTION))
#define UNITY_GTK_MENU_SECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_GTK_TYPE_MENU_SECTION, UnityGtkMenuSectionClass))
#define UNITY_GTK_IS_MENU_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_GTK_TYPE_MENU_SECTION))
#define UNITY_GTK_MENU_SECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_GTK_TYPE_MENU_SECTION, UnityGtkMenuSectionClass))
#define UNITY_GTK_MENU_GET_PRIVATE(obj)        (G_TYPE_INSTANCE_GET_PRIVATE ((obj), UNITY_GTK_TYPE_MENU, UnityGtkMenuPrivate))

typedef struct _UnityGtkMenuItem         UnityGtkMenuItem;
typedef struct _UnityGtkMenuSection      UnityGtkMenuSection;
typedef struct _UnityGtkMenuSectionClass UnityGtkMenuSectionClass;

struct _UnityGtkMenuItem
{
  GtkMenuItem         *menu_item;
  UnityGtkMenuSection *parent_section;
  UnityGtkMenu        *submenu;
  guint                submenu_valid : 1;
  gulong               menu_item_notify_handler_id;
};

/* The last item may be a separator. */
struct _UnityGtkMenuSection
{
  GMenuModel parent_instance;

  /*< private >*/
  UnityGtkMenu *parent_menu;
  guint         shell_offset;
  GPtrArray    *items;
};

struct _UnityGtkMenuSectionClass
{
  GMenuModelClass parent_class;
};

struct _UnityGtkMenuPrivate
{
  GtkMenuShell *menu_shell;
  GPtrArray    *sections;
  gulong        menu_shell_insert_handler_id;
};

G_DEFINE_TYPE (UnityGtkMenuSection, unity_gtk_menu_section, G_TYPE_MENU_MODEL);
G_DEFINE_TYPE (UnityGtkMenu, unity_gtk_menu, G_TYPE_MENU_MODEL);

enum
{
  MENU_SECTION_PROP_0,
  MENU_SECTION_PROP_PARENT_MENU,
  MENU_SECTION_PROP_SHELL_OFFSET,
  MENU_SECTION_PROP_ITEMS,
  MENU_SECTION_N_PROPERTIES
};

enum
{
  MENU_PROP_0,
  MENU_PROP_MENU_SHELL,
  MENU_PROP_SECTIONS,
  MENU_N_PROPERTIES
};

static GParamSpec *menu_section_properties[MENU_SECTION_N_PROPERTIES] = { NULL };
static GParamSpec *menu_properties[MENU_N_PROPERTIES] = { NULL };

static void unity_gtk_menu_remove_item (UnityGtkMenu     *menu,
                                        UnityGtkMenuItem *item);

static void
unity_gtk_menu_item_handle_notify (GObject    *gobject,
                                   GParamSpec *pspec,
                                   gpointer    user_data)
{
  static const gchar *parent_name;

  UnityGtkMenuItem *item = user_data;
  GtkMenuItem *menu_item = item->menu_item;

  g_warn_if_fail (gobject == G_OBJECT (menu_item));

  if (parent_name == NULL)
    parent_name = g_intern_static_string ("parent");

  if (g_param_spec_get_name (pspec) == parent_name)
    {
      if (gtk_widget_get_parent (GTK_WIDGET (menu_item)) == NULL)
        {
          g_assert (item->parent_section != NULL);
          g_assert (item->parent_section->parent_menu != NULL);

          unity_gtk_menu_remove_item (item->parent_section->parent_menu, item);
        }
    }
}

static void
unity_gtk_menu_item_set_menu_item (UnityGtkMenuItem *item,
                                   GtkMenuItem      *menu_item)
{
  g_return_if_fail (item != NULL);

  if (menu_item != item->menu_item)
    {
      if (item->menu_item_notify_handler_id)
        {
          g_assert (item->menu_item != NULL);

          g_signal_handler_disconnect (item->menu_item, item->menu_item_notify_handler_id);
          item->menu_item_notify_handler_id = 0;
        }

      item->menu_item = menu_item;

      if (menu_item != NULL)
        item->menu_item_notify_handler_id = g_signal_connect (menu_item, "notify", G_CALLBACK (unity_gtk_menu_item_handle_notify), item);
    }
}

static UnityGtkMenuItem *
unity_gtk_menu_item_new (GtkMenuItem         *menu_item,
                         UnityGtkMenuSection *parent_section)
{
  UnityGtkMenuItem *item = g_slice_new0 (UnityGtkMenuItem);

  unity_gtk_menu_item_set_menu_item (item, menu_item);

  if (parent_section != NULL)
    item->parent_section = g_object_ref (parent_section);

  return item;
}

static void
unity_gtk_menu_item_free (gpointer data)
{
  if (data != NULL)
    {
      UnityGtkMenuItem *item = data;

      unity_gtk_menu_item_set_menu_item (item, NULL);

      if (item->submenu != NULL)
        g_object_unref (item->submenu);

      if (item->parent_section != NULL)
        g_object_unref (item->parent_section);

      g_slice_free (UnityGtkMenuItem, item);
    }
}

static UnityGtkMenu *
unity_gtk_menu_item_get_submenu (UnityGtkMenuItem *item)
{
  g_return_val_if_fail (item != NULL, NULL);

  if (!item->submenu_valid)
    {
      if (item->submenu != NULL)
        {
          g_object_unref (item->submenu);
          item->submenu = NULL;
        }

      if (item->menu_item != NULL)
        {
          GtkWidget *submenu = gtk_menu_item_get_submenu (item->menu_item);

          if (submenu != NULL)
            item->submenu = unity_gtk_menu_new (GTK_MENU_SHELL (submenu));
        }

      item->submenu_valid = TRUE;
    }

  return item->submenu;
}

static void
unity_gtk_menu_section_dispose (GObject *object)
{
  UnityGtkMenuSection *section;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (object));

  section = UNITY_GTK_MENU_SECTION (object);

  if (section->items != NULL)
    {
      g_ptr_array_free (section->items, TRUE);
      section->items = NULL;
    }

  if (section->parent_menu != NULL)
    {
      g_object_unref (section->parent_menu);
      section->parent_menu = NULL;
    }

  G_OBJECT_CLASS (unity_gtk_menu_section_parent_class)->dispose (object);
}

static GPtrArray *
unity_gtk_menu_section_get_items (UnityGtkMenuSection *section)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (section), NULL);

  if (section->items == NULL && section->parent_menu != NULL && section->parent_menu->priv->menu_shell != NULL)
    {
      GtkMenuShell *menu_shell = section->parent_menu->priv->menu_shell;
      GList *start = g_list_nth (gtk_container_get_children (GTK_CONTAINER (menu_shell)), section->shell_offset);
      GList *finish = start;
      GList *iter;
      guint n;

      for (n = 0; finish != NULL && !GTK_IS_SEPARATOR_MENU_ITEM (finish->data); n++)
        finish = g_list_next (finish);

      /* Store the separator too. */
      if (finish != NULL)
        {
          finish = g_list_next (finish);
          n++;
        }

      section->items = g_ptr_array_new_full (n, unity_gtk_menu_item_free);

      for (iter = start; iter != finish; iter = g_list_next (iter))
        g_ptr_array_add (section->items, unity_gtk_menu_item_new (iter->data, section));
    }

  return section->items;
}

static void
unity_gtk_menu_section_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  UnityGtkMenuSection *self;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (object));

  self = UNITY_GTK_MENU_SECTION (object);

  switch (property_id)
    {
    case MENU_SECTION_PROP_PARENT_MENU:
      g_value_set_object (value, self->parent_menu);
      break;

    case MENU_SECTION_PROP_SHELL_OFFSET:
      g_value_set_uint (value, self->shell_offset);
      break;

    case MENU_SECTION_PROP_ITEMS:
      g_value_set_pointer (value, unity_gtk_menu_section_get_items (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
unity_gtk_menu_section_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  UnityGtkMenuSection *self;
  UnityGtkMenu *parent_menu;
  guint shell_offset;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (object));

  self = UNITY_GTK_MENU_SECTION (object);

  switch (property_id)
    {
    case MENU_SECTION_PROP_PARENT_MENU:
      parent_menu = g_value_get_object (value);

      if (parent_menu != self->parent_menu)
        {
          if (self->items != NULL)
            {
              g_ptr_array_free (self->items, TRUE);
              self->items = NULL;
            }

          if (self->parent_menu != NULL)
            g_object_unref (self->parent_menu);

          self->parent_menu = g_object_ref (parent_menu);
        }

      break;

    case MENU_SECTION_PROP_SHELL_OFFSET:
      shell_offset = g_value_get_uint (value);

      if (shell_offset != self->shell_offset)
        {
          if (self->items != NULL)
            {
              g_ptr_array_free (self->items, TRUE);
              self->items = NULL;
            }

          self->shell_offset = shell_offset;
        }

      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
unity_gtk_menu_section_is_mutable (GMenuModel *model)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (model), TRUE);

  /* The menu can change at any time. */
  return TRUE;
}

static gint
unity_gtk_menu_section_get_n_items (GMenuModel *model)
{
  GPtrArray *items;

  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (model), 0);

  items = unity_gtk_menu_section_get_items (UNITY_GTK_MENU_SECTION (model));

  if (items->len > 0)
    {
      UnityGtkMenuItem *item = g_ptr_array_index (items, items->len - 1);

      if (item != NULL && GTK_IS_SEPARATOR_MENU_ITEM (item->menu_item))
        return items->len - 1;
      else
        return items->len;
    }

  return 0;
}

static void
unity_gtk_menu_section_get_item_attributes (GMenuModel  *model,
                                            gint         item_index,
                                            GHashTable **attributes)
{
  UnityGtkMenuSection *section;
  GPtrArray *items;
  GHashTable *hash_table;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (model));
  g_return_if_fail (0 <= item_index && item_index < g_menu_model_get_n_items (model));
  g_return_if_fail (attributes != NULL);

  section = UNITY_GTK_MENU_SECTION (model);
  items = unity_gtk_menu_section_get_items (section);
  hash_table = g_hash_table_new (g_str_hash, g_str_equal);

  if (items != NULL)
    {
      UnityGtkMenuItem *item = g_ptr_array_index (items, item_index);

      if (item != NULL)
        {
          GtkMenuItem *menu_item = item->menu_item;

          if (menu_item != NULL)
            {
              const gchar *label = gtk_menu_item_get_label (menu_item);

              if (label != NULL)
                g_hash_table_insert (hash_table, G_MENU_ATTRIBUTE_LABEL, g_variant_new_string (label));
            }
        }
      else
        /* We shouldn't have any NULLs. */
        g_assert_not_reached ();
    }

  *attributes = hash_table;
}

static void
unity_gtk_menu_section_get_item_links (GMenuModel  *model,
                                       gint         item_index,
                                       GHashTable **links)
{
  UnityGtkMenuSection *section;
  GPtrArray *items;
  GHashTable *hash_table;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (model));
  g_return_if_fail (0 <= item_index && item_index < g_menu_model_get_n_items (model));
  g_return_if_fail (links != NULL);

  section = UNITY_GTK_MENU_SECTION (model);
  items = unity_gtk_menu_section_get_items (section);
  hash_table = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);

  if (items != NULL)
    {
      UnityGtkMenuItem *item = g_ptr_array_index (items, item_index);

      if (item != NULL)
        {
          UnityGtkMenu *submenu = unity_gtk_menu_item_get_submenu (item);

          if (submenu != NULL)
            g_hash_table_insert (hash_table, G_MENU_LINK_SUBMENU, g_object_ref (submenu));
        }
      else
        /* We shouldn't have any NULLs. */
        g_assert_not_reached ();
    }

  *links = hash_table;
}

static void
unity_gtk_menu_section_class_init (UnityGtkMenuSectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GMenuModelClass *menu_model_class = G_MENU_MODEL_CLASS (klass);

  object_class->dispose = unity_gtk_menu_section_dispose;
  object_class->get_property = unity_gtk_menu_section_get_property;
  object_class->set_property = unity_gtk_menu_section_set_property;
  menu_model_class->is_mutable = unity_gtk_menu_section_is_mutable;
  menu_model_class->get_n_items = unity_gtk_menu_section_get_n_items;
  menu_model_class->get_item_attributes = unity_gtk_menu_section_get_item_attributes;
  menu_model_class->get_item_links = unity_gtk_menu_section_get_item_links;

  menu_section_properties[MENU_SECTION_PROP_PARENT_MENU] = g_param_spec_object ("parent-menu",
                                                                                "Parent menu",
                                                                                "Parent menu",
                                                                                UNITY_GTK_TYPE_MENU,
                                                                                G_PARAM_READWRITE);
  menu_section_properties[MENU_SECTION_PROP_SHELL_OFFSET] = g_param_spec_uint ("shell-offset",
                                                                               "Shell offset",
                                                                               "Shell offset",
                                                                               0,
                                                                               G_MAXUINT,
                                                                               0,
                                                                               G_PARAM_READWRITE);
  menu_section_properties[MENU_SECTION_PROP_ITEMS] = g_param_spec_pointer ("items",
                                                                           "Items",
                                                                           "Items",
                                                                           G_PARAM_READABLE);

  g_object_class_install_properties (object_class, MENU_SECTION_N_PROPERTIES, menu_section_properties);
}

static void
unity_gtk_menu_section_init (UnityGtkMenuSection *self)
{
}

static UnityGtkMenuSection *
unity_gtk_menu_section_new (UnityGtkMenu *parent_menu,
                            guint         shell_offset)
{
  return g_object_new (UNITY_GTK_TYPE_MENU_SECTION,
                       "parent-menu", parent_menu,
                       "shell-offset", shell_offset,
                       NULL);
}

static void
unity_gtk_menu_dispose (GObject *object)
{
  UnityGtkMenu *menu;
  UnityGtkMenuPrivate *priv;

  g_return_if_fail (UNITY_GTK_IS_MENU (object));

  menu = UNITY_GTK_MENU (object);
  priv = menu->priv;

  if (priv->sections != NULL)
    {
      if (priv->menu_shell_insert_handler_id)
        {
          g_assert (priv->menu_shell != NULL);

          g_signal_handler_disconnect (priv->menu_shell, priv->menu_shell_insert_handler_id);
          priv->menu_shell_insert_handler_id = 0;
        }

      g_ptr_array_free (priv->sections, TRUE);
      priv->sections = NULL;
    }

  G_OBJECT_CLASS (unity_gtk_menu_parent_class)->dispose (object);
}

static void
g_ptr_array_insert (GPtrArray *ptr_array,
                    gpointer   data,
                    gint       index)
{
  guint i;

  g_return_if_fail (ptr_array != NULL);
  g_warn_if_fail (-1 <= index && index <= ptr_array->len);

  if (index < 0 || index > ptr_array->len)
    index = ptr_array->len;

  g_ptr_array_add (ptr_array, NULL);

  for (i = ptr_array->len - 1; i > index; i--)
    ptr_array->pdata[i] = g_ptr_array_index (ptr_array, i - 1);

  ptr_array->pdata[index] = data;
}

static void
unity_gtk_menu_handle_insert (GtkMenuShell *menu_shell,
                              GtkWidget    *child,
                              gint          position,
                              gpointer      user_data)
{
  UnityGtkMenu *menu = user_data;
  UnityGtkMenuPrivate *priv = menu->priv;
  GPtrArray *sections = priv->sections;
  UnityGtkMenuSection *section;
  guint section_index;
  guint i;

  g_return_if_fail (sections != NULL);

  if (position < 0)
    position = g_list_length (gtk_container_get_children (GTK_CONTAINER (menu_shell))) - 1;

  /* Find the index of the modified section. */
  for (section_index = 0; section_index + 1 < sections->len; section_index++)
    {
      section = g_ptr_array_index (sections, section_index + 1);

      if (section->shell_offset > position)
        break;
    }

  /* Shift shell offsets up by 1. */
  for (i = section_index + 1; i < sections->len; i++)
    {
      section = g_ptr_array_index (sections, i);
      section->shell_offset++;
    }

  section = g_ptr_array_index (sections, section_index);

  if (GTK_IS_SEPARATOR_MENU_ITEM (child))
    {
      UnityGtkMenuSection *new_section = unity_gtk_menu_section_new (menu, position + 1);

      g_ptr_array_insert (sections, new_section, section_index + 1);

      if (section->items != NULL)
        {
          guint size = position - section->shell_offset;
          guint new_size = section->items->len - size;

          new_section->items = g_ptr_array_new_full (new_size, unity_gtk_menu_item_free);

          /* Move menu items to the new section. */
          for (i = 0; i < new_size; i++)
            {
              UnityGtkMenuItem *item = g_ptr_array_index (section->items, size + i);

              item->parent_section = new_section;
              g_ptr_array_add (new_section->items, item);
              section->items->pdata[size + i] = NULL;
            }

          g_ptr_array_set_size (section->items, size + 1);
          section->items->pdata[size] = unity_gtk_menu_item_new (GTK_MENU_ITEM (child), section);

          if (new_size)
            g_menu_model_items_changed (G_MENU_MODEL (section), size, new_size - 1, 0);
        }

      g_menu_model_items_changed (G_MENU_MODEL (menu), section_index + 1, 0, 1);
    }
  else if (section->items != NULL)
    {
      UnityGtkMenuItem *item = unity_gtk_menu_item_new (GTK_MENU_ITEM (child), section);
      guint index = position - section->shell_offset;

      g_ptr_array_insert (section->items, item, index);

      g_menu_model_items_changed (G_MENU_MODEL (section), index, 0, 1);
    }
}

static void
unity_gtk_menu_remove_item (UnityGtkMenu     *menu,
                            UnityGtkMenuItem *item)
{
  GPtrArray *sections;
  UnityGtkMenuSection *section;
  GtkMenuItem *menu_item;

  g_return_if_fail (UNITY_GTK_IS_MENU (menu));
  g_return_if_fail (item != NULL);

  sections = menu->priv->sections;
  section = item->parent_section;
  menu_item = item->menu_item;

  /* Not really sure how to handle this. */
  g_assert (section != NULL);

  if (sections != NULL)
    {
      guint section_index;
      guint i;

      for (section_index = 0; section_index < sections->len; section_index++)
        if (g_ptr_array_index (sections, section_index) == section)
          break;

      /* The item's parent section should be in the menu. */
      g_assert (section_index < sections->len);

      /* Shift shell offsets down by 1. */
      for (i = section_index + 1; i < sections->len; i++)
        UNITY_GTK_MENU_SECTION (g_ptr_array_index (sections, i))->shell_offset--;

      /* Merge sections if removing a separator. */
      if (GTK_IS_SEPARATOR_MENU_ITEM (menu_item))
        {
          UnityGtkMenuSection *next_section;

          /* There should be a section after a separator. */
          g_assert (section_index + 1 < sections->len);

          next_section = g_ptr_array_index (sections, section_index + 1);

          if (section->items != NULL || next_section->items != NULL)
            {
              unity_gtk_menu_section_get_items (section);
              unity_gtk_menu_section_get_items (next_section);
            }

          /* Both sections should have items or not, simultaneously. */
          g_assert ((section->items == NULL) == (next_section->items == NULL));

          if (section->items != NULL && next_section->items != NULL)
            {
              guint separator_index = section->items->len - 1;
              guint next_section_length = g_menu_model_get_n_items (G_MENU_MODEL (next_section));

              /* The separator should be at the end. */
              g_assert (g_ptr_array_index (section->items, separator_index) == item);

              g_ptr_array_set_size (section->items, separator_index + next_section->items->len);
              unity_gtk_menu_item_free (g_ptr_array_index (section->items, separator_index));

              for (i = 0; i < next_section->items->len; i++)
                {
                  section->items->pdata[separator_index + i] = g_ptr_array_index (next_section->items, i);
                  next_section->items->pdata[i] = NULL;
                }

              g_menu_model_items_changed (G_MENU_MODEL (section), separator_index, 0, next_section_length);
            }

          g_ptr_array_remove_index (sections, section_index + 1);

          g_menu_model_items_changed (G_MENU_MODEL (menu), section_index + 1, 1, 0);
        }
    }

  if (section->items != NULL && !GTK_IS_SEPARATOR_MENU_ITEM (menu_item))
    {
      guint i;

      for (i = 0; i < section->items->len; i++)
        if (g_ptr_array_index (section->items, i) == item)
          break;

      /* The item should be in its section. */
      g_assert (i < section->items->len);

      g_ptr_array_remove_index (section->items, i);

      g_menu_model_items_changed (G_MENU_MODEL (section), i, 1, 0);
    }
}

static GPtrArray *
unity_gtk_menu_get_sections (UnityGtkMenu *menu)
{
  UnityGtkMenuPrivate *priv;

  g_return_val_if_fail (UNITY_GTK_IS_MENU (menu), NULL);

  priv = menu->priv;

  if (priv->sections == NULL)
    {
      GtkMenuShell *menu_shell = priv->menu_shell;

      priv->sections = g_ptr_array_new_with_free_func (g_object_unref);

      if (menu_shell != NULL)
        {
          GList *iter = gtk_container_get_children (GTK_CONTAINER (menu_shell));
          guint i;

          for (i = 0; iter != NULL; i++)
            {
              g_ptr_array_add (priv->sections, unity_gtk_menu_section_new (menu, i));

              for (; iter != NULL && !GTK_IS_SEPARATOR_MENU_ITEM (iter->data); i++)
                iter = g_list_next (iter);

              if (iter != NULL)
                {
                  iter = g_list_next (iter);

                  /* We have a trailing separator. */
                  if (iter == NULL)
                    g_ptr_array_add (priv->sections, unity_gtk_menu_section_new (menu, i + 1));
                }
            }

          priv->menu_shell_insert_handler_id = g_signal_connect (menu_shell, "insert", G_CALLBACK (unity_gtk_menu_handle_insert), menu);
        }
    }

  return priv->sections;
}

static void
unity_gtk_menu_get_property (GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  UnityGtkMenu *self;
  UnityGtkMenuPrivate *priv;

  g_return_if_fail (UNITY_GTK_IS_MENU (object));

  self = UNITY_GTK_MENU (object);
  priv = self->priv;

  switch (property_id)
    {
    case MENU_PROP_MENU_SHELL:
      g_value_set_pointer (value, priv->menu_shell);
      break;

    case MENU_PROP_SECTIONS:
      g_value_set_pointer (value, unity_gtk_menu_get_sections (self));
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
  GtkMenuShell *menu_shell;

  g_return_if_fail (UNITY_GTK_IS_MENU (object));

  self = UNITY_GTK_MENU (object);
  priv = self->priv;

  switch (property_id)
    {
    case MENU_PROP_MENU_SHELL:
      menu_shell = g_value_get_pointer (value);

      if (menu_shell != priv->menu_shell)
        {
          if (priv->sections != NULL)
            {
              if (priv->menu_shell_insert_handler_id)
                {
                  g_assert (priv->menu_shell != NULL);

                  g_signal_handler_disconnect (priv->menu_shell, priv->menu_shell_insert_handler_id);
                  priv->menu_shell_insert_handler_id = 0;
                }

              g_ptr_array_free (priv->sections, TRUE);
              priv->sections = NULL;
            }

          priv->menu_shell = menu_shell;
        }

      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
unity_gtk_menu_is_mutable (GMenuModel *model)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU (model), TRUE);

  /* The menu can change at any time. */
  return TRUE;
}

static gint
unity_gtk_menu_get_n_items (GMenuModel *model)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU (model), 0);

  return unity_gtk_menu_get_sections (UNITY_GTK_MENU (model))->len;
}

static void
unity_gtk_menu_get_item_attributes (GMenuModel  *model,
                                    gint         item_index,
                                    GHashTable **attributes)
{
  g_return_if_fail (UNITY_GTK_IS_MENU (model));
  g_return_if_fail (0 <= item_index && item_index < g_menu_model_get_n_items (model));
  g_return_if_fail (attributes != NULL);

  *attributes = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
unity_gtk_menu_get_item_links (GMenuModel  *model,
                               gint         item_index,
                               GHashTable **links)
{
  UnityGtkMenu *menu;
  GPtrArray *sections;
  UnityGtkMenuSection *section;
  GHashTable *hash_table;

  g_return_if_fail (UNITY_GTK_IS_MENU (model));
  g_return_if_fail (0 <= item_index && item_index < g_menu_model_get_n_items (model));
  g_return_if_fail (links != NULL);

  menu = UNITY_GTK_MENU (model);
  sections = unity_gtk_menu_get_sections (menu);
  section = g_ptr_array_index (sections, item_index);
  hash_table = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);

  if (section != NULL)
    g_hash_table_insert (hash_table, G_MENU_LINK_SECTION, g_object_ref (section));

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

  menu_properties[MENU_PROP_MENU_SHELL] = g_param_spec_pointer ("menu-shell",
                                                                "Menu shell",
                                                                "Menu shell",
                                                                G_PARAM_READWRITE);
  menu_properties[MENU_PROP_SECTIONS] = g_param_spec_pointer ("sections",
                                                              "Sections",
                                                              "Sections",
                                                              G_PARAM_READABLE);

  g_object_class_install_properties (object_class, MENU_N_PROPERTIES, menu_properties);

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
  return g_object_new (UNITY_GTK_TYPE_MENU,
                       "menu-shell", menu_shell,
                       NULL);
}

static void
unity_gtk_menu_item_check (UnityGtkMenuItem *item)
{
  guint i;

  g_return_if_fail (item != NULL);
  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (item->parent_section));
  g_return_if_fail (item->menu_item == NULL || GTK_IS_MENU_ITEM (item->menu_item));
  g_return_if_fail (!!item->menu_item_notify_handler_id == GTK_IS_MENU_ITEM (item->menu_item));
  g_return_if_fail (!item->submenu_valid || item->submenu == NULL || UNITY_GTK_IS_MENU (item->submenu));
  g_return_if_fail (item->parent_section->items != NULL);

  /* A menu item must be contained in its parent section. */
  for (i = 0; i < item->parent_section->items->len; i++)
    if (g_ptr_array_index (item->parent_section->items, i) == item)
      break;

  g_return_if_fail (i < item->parent_section->items->len);

  /* Separators must appear at the end of their parent sections. */
  g_return_if_fail (!GTK_IS_SEPARATOR_MENU_ITEM (item->menu_item) || g_ptr_array_index (item->parent_section->items, item->parent_section->items->len - 1) == item);

  /* The menu item should match what's in the shell. */
  if (item->menu_item != NULL)
    {
      GList *iter;

      g_return_if_fail (UNITY_GTK_IS_MENU (item->parent_section->parent_menu));
      g_return_if_fail (GTK_IS_MENU_SHELL (item->parent_section->parent_menu->priv->menu_shell));

      iter = g_list_nth (gtk_container_get_children (GTK_CONTAINER (item->parent_section->parent_menu->priv->menu_shell)), item->parent_section->shell_offset);

      if (item->parent_section->items->len > 0)
        {
          for (i = 0; i < item->parent_section->items->len; i++)
            {
              if (g_ptr_array_index (item->parent_section->items, i) == item)
                {
                  g_return_if_fail (iter != NULL);
                  g_return_if_fail (item->menu_item == iter->data);

                  break;
                }

              iter = g_list_next (iter);
            }

          g_return_if_fail (i < item->parent_section->items->len);
        }
    }

  if (item->submenu_valid && UNITY_GTK_IS_MENU (item->submenu))
    unity_gtk_menu_check (item->submenu);
}

static void
unity_gtk_menu_section_check (UnityGtkMenuSection *section)
{
  guint i;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (section));
  g_return_if_fail (UNITY_GTK_IS_MENU (section->parent_menu));

  /* A section must be contained in its parent menu. */
  for (i = 0; i < section->parent_menu->priv->sections->len; i++)
    if (g_ptr_array_index (section->parent_menu->priv->sections, i) == section)
      break;

  g_return_if_fail (i < section->parent_menu->priv->sections->len);

  if (section->items != NULL)
    {
      for (i = 0; i < section->items->len; i++)
        {
          UnityGtkMenuItem *item = g_ptr_array_index (section->items, i);

          g_return_if_fail (item != NULL);

          /* This section is the parent of all of its items. */
          g_return_if_fail (item->parent_section == section);

          /* Separators can only exist at the end of a section. */
          if (i < section->items->len - 1)
            g_return_if_fail (!GTK_IS_SEPARATOR_MENU_ITEM (item->menu_item));
        }

      for (i = 0; i < section->items->len; i++)
        unity_gtk_menu_item_check (g_ptr_array_index (section->items, i));
    }
}

void
unity_gtk_menu_check (UnityGtkMenu *menu)
{
  UnityGtkMenuPrivate *priv;

  g_return_if_fail (UNITY_GTK_IS_MENU (menu));

  priv = menu->priv;

  g_return_if_fail (priv->menu_shell == NULL || GTK_IS_MENU_SHELL (priv->menu_shell));
  g_return_if_fail (priv->sections == NULL || GTK_IS_MENU_SHELL (priv->menu_shell));
  g_return_if_fail (!priv->menu_shell_insert_handler_id || priv->sections != NULL);

  if (priv->sections != NULL)
    {
      guint i;

      for (i = 0; i < priv->sections->len; i++)
        {
          UnityGtkMenuSection *section = g_ptr_array_index (priv->sections, i);

          g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (section));

          /* This menu is the parent of all of its sections. */
          g_return_if_fail (section->parent_menu == menu);

          if (section->items != NULL)
            {
              if (i + 1 < priv->sections->len)
                {
                  UnityGtkMenuSection *next_section = g_ptr_array_index (priv->sections, i + 1);
                  UnityGtkMenuItem *item;
                  GtkMenuItem *menu_item;

                  /* The shell offsets should be correct. */
                  g_return_if_fail (section->shell_offset + section->items->len == next_section->shell_offset);

                  /* We should have at least one item. */
                  g_return_if_fail (section->items->len > 0);

                  item = g_ptr_array_index (section->items, section->items->len - 1);
                  g_return_if_fail (item != NULL);
                  menu_item = item->menu_item;

                  /* The last item should be a separator. */
                  g_return_if_fail (menu_item == NULL || GTK_IS_SEPARATOR_MENU_ITEM (menu_item));
                }
              else if (section->items->len > 0)
                {
                  UnityGtkMenuItem *item = g_ptr_array_index (section->items, section->items->len - 1);

                  /* The last item should not be a separator. */
                  g_return_if_fail (!GTK_IS_SEPARATOR_MENU_ITEM (item->menu_item));
                }
            }
        }

      for (i = 0; i < priv->sections->len; i++)
        unity_gtk_menu_section_check (g_ptr_array_index (priv->sections, i));
    }
}

static void
unity_gtk_menu_item_print (UnityGtkMenuItem *item,
                           guint             depth)
{
  gchar *indent;
  GtkMenuItem *menu_item;
  const gchar *label;
  UnityGtkMenu *submenu;

  g_return_if_fail (item != NULL);

  indent = g_strnfill (2 + 3 * depth, ' ');
  menu_item = item->menu_item;
  label = menu_item != NULL ? gtk_menu_item_get_label (menu_item) : NULL;
  submenu = item->submenu;

  if (label != NULL && label[0] != '\0')
    g_print ("%s(%s *) %p \"%s\"\n", indent, G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (menu_item)), item, label);
  else
    g_print ("%s(%s *) %p\n", indent, G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (menu_item)), item);

  if (submenu != NULL)
    unity_gtk_menu_print (submenu, depth + 1);

  g_free (indent);
}

static void
unity_gtk_menu_section_print (UnityGtkMenuSection *section,
                              guint                depth)
{
  gchar *indent;
  GPtrArray *items;
  guint i;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (section));

  indent = g_strnfill (1 + 3 * depth, ' ');
  items = unity_gtk_menu_section_get_items (section);

  g_print ("%s(%s *) %p\n", indent, G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (section)), section);

  for (i = 0; i < items->len; i++)
    unity_gtk_menu_item_print (g_ptr_array_index (items, i), depth);

  g_free (indent);
}

void
unity_gtk_menu_print (UnityGtkMenu *menu,
                      guint         depth)
{
  gchar *indent;
  GPtrArray *sections;
  guint i;

  g_return_if_fail (UNITY_GTK_IS_MENU (menu));

  indent = g_strnfill (3 * depth, ' ');
  sections = unity_gtk_menu_get_sections (menu);

  g_print ("%s(%s *) %p\n", indent, G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (menu)), menu);

  for (i = 0; i < sections->len; i++)
    unity_gtk_menu_section_print (g_ptr_array_index (sections, i), depth);

  g_free (indent);
}

void
g_menu_model_print (GMenuModel *model,
                    guint       depth)
{
  gchar *indent;
  guint i;

  g_return_if_fail (G_IS_MENU_MODEL (model));

  indent = g_strnfill (3 * depth, ' ');

  g_print ("%s(%s *) %p\n", indent, G_OBJECT_CLASS_NAME (G_OBJECT_GET_CLASS (model)), model);

  for (i = 0; i < g_menu_model_get_n_items (model); i++)
    {
      GVariant *variant = g_menu_model_get_item_attribute_value (model, i, G_MENU_ATTRIBUTE_LABEL, G_VARIANT_TYPE_STRING);
      GMenuModel *section = g_menu_model_get_item_link (model, i, G_MENU_LINK_SECTION);
      GMenuModel *submenu = g_menu_model_get_item_link (model, i, G_MENU_LINK_SUBMENU);
      const gchar *label = NULL;

      if (variant != NULL)
        label = g_variant_get_string (variant, NULL);

      if (label != NULL)
        g_print ("%s %d: %s\n", indent, i, label);
      else
        g_print ("%s %d\n", indent, i);

      if (section != NULL)
        {
          g_print ("%s  section\n", indent);
          g_menu_model_print (section, depth + 1);
        }

      if (submenu != NULL)
        {
          g_print ("%s  submenu\n", indent);
          g_menu_model_print (submenu, depth + 1);
        }
    }

  g_free (indent);
}
