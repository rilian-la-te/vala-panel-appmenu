/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "appmenu-gtk-parser.h"
#include "comparator.h"
#include <debug.h>
#include <gtk/gtk.h>

typedef struct {
	GMenuModel         *source;
	GtkWidget          *menubar;
	UnityGtkMenuShell  *shell;
	UnityGtkActionGroup *action_group;
} Fixture;

static void
fixture_setup(Fixture *f, GMenuModel *source)
{
	f->source       = source;
	f->menubar      = gtk_menu_bar_new();
	gtk_menu_shell_bind_model(GTK_MENU_SHELL(f->menubar), source, NULL, TRUE);
	f->shell        = unity_gtk_menu_shell_new(GTK_MENU_SHELL(f->menubar));
	f->action_group = unity_gtk_action_group_new(NULL);
	unity_gtk_action_group_connect_shell(f->action_group, f->shell);
}

static void
fixture_teardown(Fixture *f)
{
	g_object_unref(f->shell);
	g_object_unref(f->action_group);
	gtk_widget_destroy(f->menubar);
	g_object_unref(f->source);
}

static void
pump_events(void)
{
	while (gtk_events_pending())
		gtk_main_iteration();
}

static gboolean
compare_and_check_actions(Fixture *f, GString *errors)
{
#ifdef NO_LAZY
	(void)g_menu_model_get_n_items(G_MENU_MODEL(f->shell));
#endif

	gboolean ok = compare_models(f->source, G_MENU_MODEL(f->shell), errors, 0);

	gboolean has_actions = FALSE;
	check_any_actions(G_MENU_MODEL(f->shell), &has_actions);
	if (!has_actions)
	{
		g_string_append(errors,
		                "parsed model contains no 'unity.*' action attributes; "
		                "the action group was not wired\n");
		ok = FALSE;
	}
	return ok;
}

static void
dump_and_report(const char *tag, Fixture *f, GString *errors)
{
	g_printerr("%s failed:\n%s\n", tag, errors->str);
	g_printerr("--- source ---\n");
	g_menu_markup_print_to_console(f->source);
	g_printerr("--- parsed ---\n");
	g_menu_markup_print_to_console(G_MENU_MODEL(f->shell));
}

/* ------------------------------------------------------------------ */
/* 1. Model is modified on the fly and compared twice.                */

static void
test_dynamic_modification(void)
{
	GMenu *root     = g_menu_new();
	GMenu *section1 = g_menu_new();
	g_menu_append(section1, "New", "app.new");
	g_menu_append_section(root, NULL, G_MENU_MODEL(section1));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root));

	GString *errors = g_string_new(NULL);

	/* --- First pass: initial model. --- */
	gboolean ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("dynamic/first comparison", &f, errors);
	g_assert_true(ok);
	g_string_set_size(errors, 0);

	/* --- Mutate: extend the existing section and add a new one. --- */
	g_menu_append(section1, "Open", "app.open");
	g_menu_append(section1, "Save", "app.save");

	GMenu *section2 = g_menu_new();
	g_menu_append(section2, "Quit", "app.quit");
	g_menu_append_section(root, NULL, G_MENU_MODEL(section2));
	g_object_unref(section2);

	pump_events();

	/* --- Second pass: both source and parsed model must reflect the
	 * new items. --- */
	ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("dynamic/second comparison", &f, errors);
	g_assert_true(ok);

	g_string_free(errors, TRUE);
	fixture_teardown(&f);
	g_object_unref(section1);
}

/* ------------------------------------------------------------------ */
/* 2. submenu-action: submenu is populated on change-state(TRUE).      */

static GMenu   *g_dynamic_submenu;
static gboolean g_dynamic_loaded;

static void
load_dynamic_submenu(void)
{
	if (g_dynamic_loaded)
		return;
	g_dynamic_loaded = TRUE;
	g_menu_append(g_dynamic_submenu, "Recent 1", "app.recent1");
	g_menu_append(g_dynamic_submenu, "Recent 2", "app.recent2");
}

/* submenu-action MUST be a stateful boolean action: GTK calls
 * g_action_group_change_action_state() on it with a boolean value
 * (TRUE) right before it opens the submenu. The handler's job is to
 * populate the submenu at that moment. We let the default GSimpleAction
 * handler update the state; we only need to react to the request. */
static void
on_load_recent_change_state(GSimpleAction *action, GVariant *value, gpointer user_data)
{
	(void)action; (void)user_data;
	g_simple_action_set_state (action, value);
	if (g_variant_get_boolean(value))
		load_dynamic_submenu();
}

static void
test_submenu_action(void)
{
	GMenu *root    = g_menu_new();
	GMenu *section = g_menu_new();

	g_dynamic_submenu = g_menu_new();
	g_dynamic_loaded  = FALSE;

	/* Item with submenu-action and an empty submenu. */
	GMenuItem *recent = g_menu_item_new_submenu("Recent",
	                                            G_MENU_MODEL(g_dynamic_submenu));
	g_menu_item_set_attribute(recent,
	                          G_MENU_ATTRIBUTE_SUBMENU_ACTION,
	                          "s", "app.load-recent");
	g_menu_append_item(section, recent);
	g_object_unref(recent);

	/* Regular item so the parser has at least one "unity.*" action. */
	g_menu_append(section, "Quit", "app.quit");

	g_menu_append_section(root, NULL, G_MENU_MODEL(section));
	g_object_unref(section);

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root));

	/* Stateful boolean action — this is what GTK requires for
	 * submenu-action. Initial state is FALSE. */
	GSimpleAction *load_action = g_simple_action_new_stateful(
		"load-recent", G_VARIANT_TYPE_BOOLEAN, g_variant_new_boolean(FALSE));
	g_signal_connect(load_action, "change-state",
	                 G_CALLBACK(on_load_recent_change_state), NULL);

	GSimpleActionGroup *app_actions = g_simple_action_group_new();
	g_action_map_add_action(G_ACTION_MAP(app_actions), G_ACTION(load_action));
	gtk_widget_insert_action_group(f.menubar, "app", G_ACTION_GROUP(app_actions));

	GString *errors = g_string_new(NULL);

	/* --- Before the request: both submenus are empty. --- */
	gboolean ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("submenu-action/pre-request comparison", &f, errors);
	g_assert_true(ok);
	g_string_set_size(errors, 0);

	/* --- Simulate what GTK does when the submenu is about to open:
	 * change submenu-action state to TRUE. The handler populates the
	 * source submenu; the binding propagates the new items to the
	 * GtkMenuShell; the parser picks them up. --- */
	g_action_group_change_action_state(G_ACTION_GROUP(app_actions),
	                                   "load-recent",
	                                   g_variant_new_boolean(TRUE));
	pump_events();

	g_assert_true(g_dynamic_loaded);
	{
		GVariant *state = g_action_group_get_action_state(
			G_ACTION_GROUP(app_actions), "load-recent");
		g_assert_nonnull(state);
		g_assert_true(g_variant_get_boolean(state));
		g_variant_unref(state);
	}

	/* --- After the request: both sides show the loaded submenu. --- */
	ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("submenu-action/post-request comparison", &f, errors);
	g_assert_true(ok);

	g_string_free(errors, TRUE);
	gtk_widget_insert_action_group(f.menubar, "app", NULL);
	g_object_unref(app_actions);
	g_object_unref(load_action);
	fixture_teardown(&f);
	g_clear_object(&g_dynamic_submenu);
}

static void
size_state_change(GSimpleAction *action, GVariant *value, gpointer user_data)
{
	(void)action; (void)user_data;
	g_simple_action_set_state (action, value);
}

static void
test_radio_selection_change(void)
{
	GMenu *root    = g_menu_new();
	GMenu *section = g_menu_new();

	GMenuItem *small = g_menu_item_new("Small", "app.size");
	g_menu_item_set_attribute(small, G_MENU_ATTRIBUTE_TARGET, "s", "small");
	g_menu_append_item(section, small);
	g_object_unref(small);

	GMenuItem *large = g_menu_item_new("Large", "app.size");
	g_menu_item_set_attribute(large, G_MENU_ATTRIBUTE_TARGET, "s", "large");
	g_menu_append_item(section, large);
	g_object_unref(large);

	g_menu_append_section(root, NULL, G_MENU_MODEL(section));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root));

	GSimpleAction *load_action = g_simple_action_new_stateful(
		"size", G_VARIANT_TYPE_STRING, g_variant_new_string("small"));
	g_signal_connect(load_action, "change-state",
	                 G_CALLBACK(size_state_change), NULL);

	GSimpleActionGroup *app_actions = g_simple_action_group_new();
	g_action_map_add_action(G_ACTION_MAP(app_actions), G_ACTION(load_action));
	gtk_widget_insert_action_group(f.menubar, "app", G_ACTION_GROUP(app_actions));

	pump_events();

	GString *errors = g_string_new(NULL);
	gboolean ok = compare_and_check_actions(&f, errors);
	if (!ok)
		dump_and_report("radio-selection-change", &f, errors);
	g_assert_true(ok);

	gtk_widget_insert_action_group(f.menubar, "app", NULL);
	g_object_unref(app_actions);
	g_object_unref(load_action);
	g_string_free(errors, TRUE);
	fixture_teardown(&f);
	g_object_unref(section);
}

/* ------------------------------------------------------------------ */

int
main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	if (!gtk_init_check(&argc, &argv))
	{
		g_printerr("Skipping: no display available\n");
		return 77; /* Meson: skipped */
	}

	g_test_add_func("/parser/dynamic-modification", test_dynamic_modification);
	g_test_add_func("/parser/radio-selection-change", test_radio_selection_change);
	g_test_add_func("/parser/submenu-action",       test_submenu_action);

	return g_test_run();
}
