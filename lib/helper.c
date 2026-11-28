#include "helper.h"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
	APPMENU_HELPER_CONTENT_UPDATED_SIGNAL,
	APPMENU_HELPER_NUM_SIGNALS
};
static guint appmenu_helper_signals[APPMENU_HELPER_NUM_SIGNALS] = { 0 };

G_DEFINE_ABSTRACT_TYPE(AppmenuHelper, appmenu_helper, G_TYPE_OBJECT)

GMenuModel *appmenu_helper_create_model(GMenuModel *appmenu, GMenuModel *menubar)
{
	GMenu *combined = g_menu_new();

	if (appmenu)
		g_menu_append_section(combined, NULL, appmenu);
	if (menubar)
		g_menu_append_section(combined, NULL, menubar);

	return G_MENU_MODEL(combined);
}

static void copy_action(gpointer key, gpointer value, gpointer user_data)
{
	g_return_if_fail(key != NULL);
	g_return_if_fail(value != NULL);

	g_hash_table_insert((GHashTable *)user_data, key, g_object_ref(value));
}

void appmenu_helper_merge_actions(GHashTable *src, GHashTable *dst)
{
	g_return_if_fail(src != NULL);
	g_return_if_fail(dst != NULL);
	g_hash_table_foreach(src, copy_action, dst);
}

static void appmenu_helper_finalize(GObject *obj)
{
	AppmenuHelper *self = APPMENU_HELPER(obj);

	g_clear_object(&self->menu);
	g_clear_pointer(&self->actions, g_hash_table_unref);
	G_OBJECT_CLASS(appmenu_helper_parent_class)->finalize(obj);
}

static void appmenu_helper_init(AppmenuHelper *self)
{
	self->actions = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);
}

static void appmenu_helper_class_init(AppmenuHelperClass *klass)
{
	G_OBJECT_CLASS(klass)->finalize = appmenu_helper_finalize;
	appmenu_helper_signals[APPMENU_HELPER_CONTENT_UPDATED_SIGNAL] =
	    g_signal_new("content-updated", appmenu_helper_get_type(), G_SIGNAL_RUN_LAST, 0, NULL,
	                 NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}