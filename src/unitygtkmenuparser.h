/*
 * Copyright © 2011 Canonical Limited
 *
 * All rights reserved.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef __UNITY_GTK_MENU_PARSER_H__
#define __UNITY_GTK_MENU_PARSER_H__

#include <gtk/gtk.h>

#define UNITY_GTK_TYPE_MENU_PARSER                           (unity_gtk_menu_parser_get_type ())
#define UNITY_GTK_MENU_PARSER(inst)                          (G_TYPE_CHECK_INSTANCE_CAST ((inst),                    \
                                                              UNITY_GTK_TYPE_MENU_PARSER, UnityGtkMenuParser))
#define UNITY_GTK_IS_MENU_PARSER(inst)                       (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                    \
                                                              UNITY_GTK_TYPE_MENU_PARSER))

typedef struct _UnityGtkMenuParser                           UnityGtkMenuParser;

GType                   unity_gtk_menu_parser_get_type                  (void) G_GNUC_INTERNAL;
UnityGtkMenuParser *    unity_gtk_menu_parser_new                       (GtkMenuShell *menu_shell) G_GNUC_INTERNAL;
void                    unity_gtk_menu_parser_destroy                   (UnityGtkMenuParser *parser) G_GNUC_INTERNAL;
GActionGroup *          unity_gtk_menu_parser_get_action_group          (UnityGtkMenuParser *parser) G_GNUC_INTERNAL;

#endif /* __UNITY_GTK_MENU_PARSER_H__ */
