/*
 * Copyright © 2011 Canonical Limited
 *
 * All rights reserved.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef __MENU_SOURCE_H__
#define __MENU_SOURCE_H__

#include <gdk/gdk.h>

#define MENU_SOURCE_TYPE (menu_source_get_type())
#define MENU_SOURCE(inst) (G_TYPE_CHECK_INSTANCE_CAST((inst), MENU_SOURCE_TYPE, MenuSource))
#define IS_MENU_SOURCE(inst) (G_TYPE_CHECK_INSTANCE_TYPE((inst), MENU_SOURCE_TYPE))

typedef struct _MenuSource MenuSource;

GType menu_source_get_type(void) G_GNUC_CONST;
MenuSource *menu_source_get_for_screen(GdkScreen *screen);
GMenuModel *menu_source_get_menu(MenuSource *menu_source);

#endif /* __MENU_SOURCE_H__ */
