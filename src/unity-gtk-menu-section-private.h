#ifndef __UNITY_GTK_MENU_SECTION_H__
#define __UNITY_GTK_MENU_SECTION_H__

#include <glib-object.h>

typedef struct _UnityGtkMenuSection      UnityGtkMenuSection;
typedef struct _UnityGtkMenuSectionClass UnityGtkMenuSectionClass;

#include "unity-gtk-menu-shell-private.h"

#define UNITY_GTK_TYPE_MENU_SECTION            (unity_gtk_menu_section_get_type ())
#define UNITY_GTK_MENU_SECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNITY_GTK_TYPE_MENU_SECTION, UnityGtkMenuSection))
#define UNITY_GTK_IS_MENU_SECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNITY_GTK_TYPE_MENU_SECTION))
#define UNITY_GTK_MENU_SECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNITY_GTK_TYPE_MENU_SECTION, UnityGtkMenuSectionClass))
#define UNITY_GTK_IS_MENU_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNITY_GTK_TYPE_MENU_SECTION))
#define UNITY_GTK_MENU_SECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNITY_GTK_TYPE_MENU_SECTION, UnityGtkMenuSectionClass))

struct _UnityGtkMenuSection
{
  GMenuModel parent_instance;

  /*< private >*/
  UnityGtkMenuShell *parent_shell;
  guint              section_index;
};

struct _UnityGtkMenuSectionClass
{
  GMenuModelClass parent_class;
};

GType                 unity_gtk_menu_section_get_type       (void)                               G_GNUC_INTERNAL;

UnityGtkMenuSection * unity_gtk_menu_section_new            (UnityGtkMenuShell   *parent_shell,
                                                             guint                section_index) G_GNUC_INTERNAL;

GSequenceIter *       unity_gtk_menu_section_get_begin_iter (UnityGtkMenuSection *section)       G_GNUC_INTERNAL;

GSequenceIter *       unity_gtk_menu_section_get_end_iter   (UnityGtkMenuSection *section)       G_GNUC_INTERNAL;

GSequenceIter *       unity_gtk_menu_section_get_iter       (UnityGtkMenuSection *section,
                                                             guint                index)         G_GNUC_INTERNAL;

void                  unity_gtk_menu_section_print          (UnityGtkMenuSection *section,
                                                             guint                depth)         G_GNUC_INTERNAL;

#endif /* __UNITY_GTK_MENU_SECTION_H__ */
