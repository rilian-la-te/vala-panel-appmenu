// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "definitions.h"
#include <gio/gio.h>

G_BEGIN_DECLS

typedef enum  {
	APPMENU_MODEL_TYPE_NONE,
	APPMENU_MODEL_TYPE_STUB,
	APPMENU_MODEL_TYPE_DESKTOP,
	APPMENU_MODEL_TYPE_MENUMODEL,
	APPMENU_MODEL_TYPE_DBUSMENU
} AppmenuModelType;

struct _AppmenuHelperClass
{
	GObjectClass parent_class;
	gpointer padding[10];
};

struct _AppmenuHelper
{
	GObject parent_instance;
	GMenuModel *menu;
	GHashTable *actions;
};

G_DECLARE_ANY_TYPE(AppmenuHelper, appmenu_helper, APPMENU, HELPER, GObject)

GMenuModel *appmenu_helper_create_model(GMenuModel *appmenu, GMenuModel *menubar);
void appmenu_helper_merge_actions(GHashTable *src, GHashTable *dst);

G_END_DECLS