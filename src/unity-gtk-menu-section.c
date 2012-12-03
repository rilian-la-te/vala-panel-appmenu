#include "unity-gtk-menu-section-private.h"

G_DEFINE_TYPE (UnityGtkMenuSection, unity_gtk_menu_section, G_TYPE_MENU_MODEL);

enum
{
  MENU_SECTION_PROP_0,
  MENU_SECTION_PROP_PARENT_SHELL,
  MENU_SECTION_PROP_SECTION_INDEX,
  MENU_SECTION_N_PROPERTIES
};

static GParamSpec *menu_section_properties[MENU_SECTION_N_PROPERTIES] = { NULL };

static void
unity_gtk_menu_section_set_parent_shell (UnityGtkMenuSection *section,
                                         UnityGtkMenuShell   *parent_shell)
{
  UnityGtkMenuShell *old_parent_shell;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (section));

  old_parent_shell = section->parent_shell;

  if (parent_shell != old_parent_shell)
    {
      if (old_parent_shell != NULL)
        {
          section->parent_shell = NULL;
          g_object_unref (old_parent_shell);
        }

      if (parent_shell != NULL)
        section->parent_shell = g_object_ref (parent_shell);
    }
}

static void
unity_gtk_menu_section_dispose (GObject *object)
{
  UnityGtkMenuSection *section;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (object));

  section = UNITY_GTK_MENU_SECTION (object);

  unity_gtk_menu_section_set_parent_shell (section, NULL);

  G_OBJECT_CLASS (unity_gtk_menu_section_parent_class)->dispose (object);
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
    case MENU_SECTION_PROP_PARENT_SHELL:
      g_value_set_object (value, self->parent_shell);
      break;

    case MENU_SECTION_PROP_SECTION_INDEX:
      g_value_set_uint (value, self->section_index);
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

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (object));

  self = UNITY_GTK_MENU_SECTION (object);

  switch (property_id)
    {
    case MENU_SECTION_PROP_PARENT_SHELL:
      unity_gtk_menu_section_set_parent_shell (self, g_value_get_object (value));
      break;

    case MENU_SECTION_PROP_SECTION_INDEX:
      self->section_index = g_value_get_uint (value);
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

  return TRUE;
}

static gint
unity_gtk_menu_section_get_n_items (GMenuModel *model)
{
  UnityGtkMenuSection *section;
  GSequenceIter *begin;
  GSequenceIter *end;

  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (model), 0);

  section = UNITY_GTK_MENU_SECTION (model);
  begin = unity_gtk_menu_section_get_begin_iter (section);
  end = unity_gtk_menu_section_get_end_iter (section);

  g_return_val_if_fail (begin != NULL && end != NULL, 0);

  return g_sequence_iter_get_position (end) - g_sequence_iter_get_position (begin);
}

static void
unity_gtk_menu_section_get_item_attributes (GMenuModel  *model,
                                            gint         item_index,
                                            GHashTable **attributes)
{
  UnityGtkMenuSection *section;
  UnityGtkMenuShell *parent_shell;
  UnityGtkMenuItem *item;
  GSequenceIter *iter;
  guint index;
  const gchar *label;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (model));
  g_return_if_fail (attributes != NULL);

  section = UNITY_GTK_MENU_SECTION (model);
  parent_shell = section->parent_shell;

  g_return_if_fail (parent_shell != NULL);

  iter = unity_gtk_menu_section_get_iter (section, item_index);
  index = g_sequence_get_uint (iter);
  item = unity_gtk_menu_shell_get_item (parent_shell, index);
  label = unity_gtk_menu_item_get_label (item);

  *attributes = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, (GDestroyNotify) g_variant_unref);

  if (label != NULL)
    g_hash_table_insert (*attributes, G_MENU_ATTRIBUTE_LABEL, g_variant_ref_sink (g_variant_new_string (label)));
}

static void
unity_gtk_menu_section_get_item_links (GMenuModel  *model,
                                       gint         item_index,
                                       GHashTable **links)
{
  UnityGtkMenuSection *section;
  UnityGtkMenuShell *parent_shell;
  UnityGtkMenuItem *item;
  GSequenceIter *iter;
  guint index;
  UnityGtkMenuShell *child_shell;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (model));
  g_return_if_fail (links != NULL);

  section = UNITY_GTK_MENU_SECTION (model);
  parent_shell = section->parent_shell;

  g_return_if_fail (parent_shell != NULL);

  iter = unity_gtk_menu_section_get_iter (section, item_index);
  index = g_sequence_get_uint (iter);
  item = unity_gtk_menu_shell_get_item (parent_shell, index);
  child_shell = unity_gtk_menu_item_get_child_shell (item);

  *links = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);

  if (child_shell != NULL)
    g_hash_table_insert (*links, G_MENU_LINK_SUBMENU, g_object_ref (child_shell));
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

  menu_section_properties[MENU_SECTION_PROP_PARENT_SHELL] = g_param_spec_object ("parent-shell",
                                                                                 "Parent shell",
                                                                                 "Parent shell",
                                                                                 UNITY_GTK_TYPE_MENU_SHELL,
                                                                                 G_PARAM_READWRITE);
  menu_section_properties[MENU_SECTION_PROP_SECTION_INDEX] = g_param_spec_uint ("section-index",
                                                                                "Section index",
                                                                                "Section index",
                                                                                0,
                                                                                G_MAXUINT,
                                                                                0,
                                                                                G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, MENU_SECTION_N_PROPERTIES, menu_section_properties);
}

static void
unity_gtk_menu_section_init (UnityGtkMenuSection *self)
{
}

UnityGtkMenuSection *
unity_gtk_menu_section_new (UnityGtkMenuShell *parent_shell,
                            guint              section_index)
{
  return g_object_new (UNITY_GTK_TYPE_MENU_SECTION,
                       "parent-shell", parent_shell,
                       "section-index", section_index,
                       NULL);
}

GSequenceIter *
unity_gtk_menu_section_get_begin_iter (UnityGtkMenuSection *section)
{
  UnityGtkMenuShell *parent_shell;
  GSequence *separator_indices;
  GSequence *visible_indices;
  GSequenceIter *separator_iter;
  GSequenceIter *visible_iter;

  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (section), NULL);

  parent_shell = section->parent_shell;

  g_return_val_if_fail (parent_shell != NULL, NULL);

  separator_indices = unity_gtk_menu_shell_get_separator_indices (parent_shell);
  visible_indices = unity_gtk_menu_shell_get_visible_indices (parent_shell);

  if (section->section_index > 0)
    separator_iter = g_sequence_get_iter_at_pos (separator_indices, section->section_index - 1);
  else
    separator_iter = NULL;

  if (separator_iter != NULL)
    {
      guint separator_index = g_sequence_get_uint (separator_iter);
      visible_iter = g_sequence_lookup_uint (visible_indices, separator_index);
      visible_iter = g_sequence_iter_next (visible_iter);
    }
  else
    visible_iter = g_sequence_get_begin_iter (visible_indices);

  return visible_iter;
}

GSequenceIter *
unity_gtk_menu_section_get_end_iter (UnityGtkMenuSection *section)
{
  UnityGtkMenuShell *parent_shell;
  GSequence *separator_indices;
  GSequence *visible_indices;
  GSequenceIter *separator_iter;
  GSequenceIter *visible_iter;

  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (section), NULL);

  parent_shell = section->parent_shell;

  g_return_val_if_fail (parent_shell != NULL, NULL);

  separator_indices = unity_gtk_menu_shell_get_separator_indices (parent_shell);
  visible_indices = unity_gtk_menu_shell_get_visible_indices (parent_shell);
  separator_iter = g_sequence_get_iter_at_pos (separator_indices, section->section_index);

  if (g_sequence_iter_is_end (separator_iter))
    separator_iter = NULL;

  if (separator_iter != NULL)
    {
      guint separator_index = g_sequence_get_uint (separator_iter);
      visible_iter = g_sequence_lookup_uint (visible_indices, separator_index);
    }
  else
    visible_iter = g_sequence_get_end_iter (visible_indices);

  return visible_iter;
}

GSequenceIter *
unity_gtk_menu_section_get_iter (UnityGtkMenuSection *section,
                                 guint                index)
{
  g_return_val_if_fail (UNITY_GTK_IS_MENU_SECTION (section), NULL);

  return g_sequence_iter_move (unity_gtk_menu_section_get_begin_iter (section), index);
}

void
unity_gtk_menu_section_print (UnityGtkMenuSection *section,
                              guint                depth)
{
  GSequenceIter *begin;
  GSequenceIter *end;
  gchar *indent;

  g_return_if_fail (UNITY_GTK_IS_MENU_SECTION (section));

  indent = g_strnfill (4 * depth, ' ');
  begin = unity_gtk_menu_section_get_begin_iter (section);
  end = unity_gtk_menu_section_get_end_iter (section);

  g_print ("%s(UnityGtkMenuSection *) %p\n", indent, section);
  g_print ("%s  parent_shell = %p\n", indent, section->parent_shell);
  g_print ("%s  section_index = %u\n", indent, section->section_index);

  if (begin != NULL)
    g_print ("%s  begin_iter = %d\n", indent, g_sequence_iter_get_position (begin));

  if (end != NULL)
    g_print ("%s  end_iter = %d\n", indent, g_sequence_iter_get_position (end));

  g_free (indent);
}
