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
  GtkMenuItem  *menu_item;
  UnityGtkMenu *submenu;
  guint         submenu_valid : 1;
};

struct _UnityGtkMenuSection
{
  GMenuModel parent_instance;

  /*< private >*/
  UnityGtkMenu *parent_menu;
  guint         shell_offset;
  GPtrArray    *menu_items;
};

struct _UnityGtkMenuSectionClass
{
  GMenuModelClass parent_class;
};

struct _UnityGtkMenuPrivate
{
  GtkMenuShell *menu_shell;
  GPtrArray    *sections;
};

G_DEFINE_TYPE (UnityGtkMenuSection, unity_gtk_menu_section, G_TYPE_MENU_MODEL);
G_DEFINE_TYPE (UnityGtkMenu, unity_gtk_menu, G_TYPE_MENU_MODEL);

enum
{
  MENU_SECTION_PROP_0,
  MENU_SECTION_PROP_PARENT_MENU,
  MENU_SECTION_PROP_SHELL_OFFSET,
  MENU_SECTION_PROP_MENU_ITEMS,
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
  UnityGtkMenuItem *item = data;

  g_return_if_fail (data != NULL);

  if (item->submenu != NULL)
    g_object_unref (item->submenu);

  g_slice_free (UnityGtkMenuItem, item);
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

  if (section->menu_items != NULL)
    {
      g_ptr_array_free (section->menu_items, TRUE);
      section->menu_items = NULL;
    }

  if (section->parent_menu != NULL)
    {
      g_object_unref (section->parent_menu);
      section->parent_menu = NULL;
    }

  G_OBJECT_CLASS (unity_gtk_menu_section_parent_class)->dispose (object);
}

static GPtrArray *
unity_gtk_menu_section_get_menu_items (UnityGtkMenuSection *section)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (section), NULL);

  if (section->menu_items == NULL && section->parent_menu != NULL && section->parent_menu->priv->menu_shell != NULL)
    {
      GtkMenuShell *menu_shell = section->parent_menu->priv->menu_shell;
      GList *start = gtk_container_get_children (GTK_CONTAINER (menu_shell));
      GList *finish;
      GList *iter;
      guint n;
      guint i;

      for (i = 0; start != NULL && i < section->shell_offset; i++)
        start = g_list_next (start);

      finish = start;

      for (n = 0; finish != NULL && !GTK_IS_SEPARATOR_MENU_ITEM (finish->data); n++)
        finish = g_list_next (finish);

      section->menu_items = g_ptr_array_new_full (n, unity_gtk_menu_item_free);

      for (iter = start; iter != finish; iter = g_list_next (iter))
        g_ptr_array_add (section->menu_items, unity_gtk_menu_item_new (iter->data));
    }

  return section->menu_items;
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

    case MENU_SECTION_PROP_MENU_ITEMS:
      g_value_set_pointer (value, unity_gtk_menu_section_get_menu_items (self));
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
          if (self->menu_items != NULL)
            {
              g_ptr_array_free (self->menu_items, TRUE);
              self->menu_items = NULL;
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
          if (self->menu_items != NULL)
            {
              g_ptr_array_free (self->menu_items, TRUE);
              self->menu_items = NULL;
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
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (model), FALSE);

  /* The menu can change at any time. */
  return FALSE;
}

static gint
unity_gtk_menu_section_get_n_items (GMenuModel *model)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (model), 0);

  return unity_gtk_menu_section_get_menu_items (UNITY_GTK_MENU_SECTION (model))->len;
}

static void
unity_gtk_menu_section_get_item_attributes (GMenuModel  *model,
                                            gint         item_index,
                                            GHashTable **attributes)
{
  UnityGtkMenuSection *section;
  GPtrArray *menu_items;
  GHashTable *hash_table;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (model));
  g_return_if_fail (0 <= item_index && item_index < unity_gtk_menu_section_get_n_items (model));
  g_return_if_fail (attributes != NULL);

  section = UNITY_GTK_MENU_SECTION (model);
  menu_items = unity_gtk_menu_section_get_menu_items (section);
  /* XXX: We should free values with g_variant_unref (). */
  hash_table = g_hash_table_new (g_str_hash, g_str_equal);

  if (menu_items != NULL)
    {
      UnityGtkMenuItem *item = g_ptr_array_index (menu_items, item_index);

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
  GPtrArray *menu_items;
  GHashTable *hash_table;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (model));
  g_return_if_fail (0 <= item_index && item_index < unity_gtk_menu_section_get_n_items (model));
  g_return_if_fail (links != NULL);

  section = UNITY_GTK_MENU_SECTION (model);
  menu_items = unity_gtk_menu_section_get_menu_items (section);
  hash_table = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);

  if (menu_items != NULL)
    {
      UnityGtkMenuItem *item = g_ptr_array_index (menu_items, item_index);

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
  menu_section_properties[MENU_SECTION_PROP_MENU_ITEMS] = g_param_spec_pointer ("menu-items",
                                                                                "Menu items",
                                                                                "Menu items",
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
      g_ptr_array_free (priv->sections, TRUE);
      priv->sections = NULL;
    }

  G_OBJECT_CLASS (unity_gtk_menu_parent_class)->dispose (object);
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
                iter = g_list_next (iter);
            }
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
  g_return_val_if_fail (UNITY_GTK_IS_MENU (model), FALSE);

  /* The menu can change at any time. */
  return FALSE;
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
  g_return_if_fail (0 <= item_index && item_index < unity_gtk_menu_get_n_items (model));
  g_return_if_fail (attributes != NULL);

  /* XXX: We should free values with g_variant_unref (). */
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
  g_return_if_fail (0 <= item_index && item_index < unity_gtk_menu_get_n_items (model));
  g_return_if_fail (links != NULL);

  menu = UNITY_GTK_MENU (model);
  sections = unity_gtk_menu_get_sections (menu);
  section = g_ptr_array_index (sections, item_index);
  hash_table = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);

  if (section != NULL)
    g_hash_table_insert (hash_table, G_MENU_LINK_SECTION, section);

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
unity_gtk_menu_model_print (GMenuModel *model,
                            guint       depth)
{
  gchar *indent;
  guint n;
  guint i;

  g_return_if_fail (G_IS_MENU_MODEL (model));

  indent = g_strnfill (2 * depth, ' ');

  n = g_menu_model_get_n_items (model);

  for (i = 0; i < n; i++)
    {
      GVariant *label = g_menu_model_get_item_attribute_value (model, i, G_MENU_ATTRIBUTE_LABEL, G_VARIANT_TYPE_STRING);
      GMenuModel *section = g_menu_model_get_item_link (model, i, G_MENU_LINK_SECTION);
      GMenuModel *submenu = g_menu_model_get_item_link (model, i, G_MENU_LINK_SUBMENU);

      g_print ("%s%s\n", indent, label != NULL ? g_variant_get_string (label, NULL) : "(null)");

      if (section != NULL)
        {
          g_print ("%s section\n", indent);
          unity_gtk_menu_model_print (section, depth + 1);
        }

      if (submenu != NULL)
        {
          g_print ("%s submenu\n", indent);
          unity_gtk_menu_model_print (submenu, depth + 1);
        }
    }

  g_free (indent);
}

void
unity_gtk_menu_print (UnityGtkMenu *menu)
{
  g_return_if_fail (UNITY_GTK_IS_MENU (menu));

  unity_gtk_menu_model_print (G_MENU_MODEL (menu), 0);
}
