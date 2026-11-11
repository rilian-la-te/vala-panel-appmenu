/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"

static GMenu *g_dynamic_submenu;
static gboolean g_dynamic_loaded;

static void load_dynamic_submenu(void)
{
	if (g_dynamic_loaded)
		return;
	g_dynamic_loaded = TRUE;
	g_menu_append(g_dynamic_submenu, "Recent 1", "app.recent1");
	g_menu_append(g_dynamic_submenu, "Recent 2", "app.recent2");
}

static void on_load_recent_change_state(GSimpleAction *action, GVariant *value, gpointer user_data)
{
	(void)user_data;
	g_simple_action_set_state(action, value);
	if (g_variant_get_boolean(value))
		load_dynamic_submenu();
}

G_GNUC_INTERNAL void test_func(void)
{
	GMenu *root    = g_menu_new();
	GMenu *section = g_menu_new();

	g_dynamic_submenu = g_menu_new();
	g_dynamic_loaded  = FALSE;

	GMenuItem *recent = g_menu_item_new_submenu("Recent", G_MENU_MODEL(g_dynamic_submenu));
	g_menu_item_set_attribute(recent, G_MENU_ATTRIBUTE_SUBMENU_ACTION, "s", "app.load-recent");
	g_menu_append_item(section, recent);
	g_object_unref(recent);

	g_menu_append(section, "Quit", "app.quit");
	g_menu_append_section(root, NULL, G_MENU_MODEL(section));
	g_object_unref(section);

	GSimpleAction *load_action = g_simple_action_new_stateful("load-recent",
	                                                          G_VARIANT_TYPE_BOOLEAN,
	                                                          g_variant_new_boolean(FALSE));
	g_signal_connect(load_action,
	                 "change-state",
	                 G_CALLBACK(on_load_recent_change_state),
	                 NULL);

	GSimpleActionGroup *trigger_group = g_simple_action_group_new();
	g_action_map_add_action(G_ACTION_MAP(trigger_group), G_ACTION(load_action));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root), trigger_group);
	GString *errors = g_string_new(NULL);

	gboolean ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("submenu-action/pre-request comparison", &f, errors);
	g_assert_true(ok);
	g_string_set_size(errors, 0);

	g_action_group_change_action_state(G_ACTION_GROUP(trigger_group),
	                                   "load-recent",
	                                   g_variant_new_boolean(TRUE));
	test_wait();

	g_assert_true(g_dynamic_loaded);

	ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("submenu-action/post-request comparison", &f, errors);
	g_assert_true(ok);

	g_action_group_change_action_state(G_ACTION_GROUP(trigger_group),
	                                   "load-recent",
	                                   g_variant_new_boolean(FALSE));

	test_wait();
	g_string_free(errors, TRUE);
	g_object_unref(trigger_group);
	g_object_unref(load_action);
	fixture_teardown(&f);
	g_clear_object(&g_dynamic_submenu);
}