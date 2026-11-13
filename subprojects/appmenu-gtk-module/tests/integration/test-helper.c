/* SPDX-License-Identifier: LGPL-3.0-or-later */
#include "test-helper.h"

int g_new_fires;

/* ============================================================ */
/* Callbacks                                                     */

static void on_simple_activate(GSimpleAction *a, GVariant *p, gpointer u)
{
	(void)a;
	(void)p;
	(*(int *)u)++;
}

static void checkbox_change_state(GSimpleAction *a, GVariant *v, gpointer u)
{
	(void)u;
	g_simple_action_set_state(a, v);
}

static void string_change_state(GSimpleAction *a, GVariant *v, gpointer u)
{
	(void)u;
	g_simple_action_set_state(a, v);
}

/* ============================================================ */
/* Dump                                                          */

static void dump_actions(GActionGroup *group, const char *label)
{
	if (!group)
	{
		g_printerr("--- %s actions: (null) ---\n", label);
		return;
	}
	g_printerr("--- %s actions ---\n", label);
	gchar **names = g_action_group_list_actions(group);
	for (guint i = 0; names && names[i]; i++)
	{
		GVariant *st = g_action_group_get_action_state(group, names[i]);
		gchar *s     = st ? g_variant_print(st, TRUE) : g_strdup("(stateless)");
		g_printerr("  %s = %s\n", names[i], s);
		g_free(s);
		if (st)
			g_variant_unref(st);
	}
	g_strfreev(names);
}

void dump_e2e(const char *tag, E2E *e)
{
	g_printerr("=== %s FAILED ===\n", tag);

	GString *errors = g_string_new(NULL);
	(void)compare_models(e->source, e->imported_model, errors, 0);
	if (errors->len)
		g_printerr("Models differ:\n%s\n", errors->str);
	g_string_free(errors, TRUE);

	g_printerr("--- source ---\n");
	g_menu_markup_print_to_console(e->source);
	g_printerr("--- parsed ---\n");
	g_menu_markup_print_to_console(e->imported_model);

	dump_actions(G_ACTION_GROUP(e->actions), "source");
	dump_actions(G_ACTION_GROUP(e->unity_actions), "unity-bridge");
	dump_actions(e->imported_actions, "imported");
}

/* Force-load every submenu/section in a lazy model. */
static void force_load(GMenuModel *m)
{
	int n = g_menu_model_get_n_items(m);
	for (int i = 0; i < n; i++)
	{
		GMenuModel *sub = g_menu_model_get_item_link(m, i, G_MENU_LINK_SUBMENU);
		if (sub)
		{
			force_load(sub);
			g_object_unref(sub);
		}
		GMenuModel *sec = g_menu_model_get_item_link(m, i, G_MENU_LINK_SECTION);
		if (sec)
		{
			force_load(sec);
			g_object_unref(sec);
		}
	}
}

void pump_events(void)
{
	while (gtk_events_pending())
		gtk_main_iteration();
}

void settle(guint ms)
{
	gint64 deadline = g_get_monotonic_time() + (gint64)ms * G_TIME_SPAN_MILLISECOND;
	do
	{
		pump_events();
		g_usleep(5000);
	} while (g_get_monotonic_time() < deadline);
}

void load_imported(E2E *e)
{
	for (int pass = 0; pass < 3; pass++)
	{
		force_load(e->imported_model);
		settle(80);
	}
	GMenuModel *m = NULL;
	g_object_get(e->importer, "model", &m, NULL);
	if (m && m != e->imported_model)
	{
		g_object_unref(e->imported_model);
		e->imported_model = m;
	}
	else if (m)
	{
		g_object_unref(m);
	}
	force_load(e->imported_model);
	settle(50);
}

void assert_models(E2E *e, const char *tag)
{
	load_imported(e);
	force_load(e->source);
	pump_events();

	GString *errors = g_string_new(NULL);
	gboolean ok     = compare_models(e->source, e->imported_model, errors, 0);
	if (!ok)
	{
		g_printerr("%s FAILED:\n", tag);
		g_printerr("Models differ:\n%s\n", errors->str);
		g_printerr("--- source ---\n");
		g_menu_markup_print_to_console(e->source);
		g_printerr("--- parsed ---\n");
		g_menu_markup_print_to_console(e->imported_model);
		dump_actions(G_ACTION_GROUP(e->actions), "source");
		dump_actions(G_ACTION_GROUP(e->unity_actions), "unity-bridge");
		dump_actions(e->imported_actions, "imported");
	}
	g_assert_true(ok);
	g_string_free(errors, TRUE);
}

/* ============================================================ */
/* Lookup action by label (recursive through sections only)     */

static char *action_for_label(GMenuModel *m, const char *label)
{
	int n = g_menu_model_get_n_items(m);
	for (int i = 0; i < n; i++)
	{
		gchar *l = NULL;
		gboolean has =
		    g_menu_model_get_item_attribute(m, i, G_MENU_ATTRIBUTE_LABEL, "s", &l);
		gboolean match = has && g_strcmp0(l, label) == 0;
		g_free(l);
		if (match)
		{
			char *a = NULL;
			if (g_menu_model_get_item_attribute(m, i, G_MENU_ATTRIBUTE_ACTION, "s", &a))
				return a;
			g_free(a);
		}
		GMenuModel *sec = g_menu_model_get_item_link(m, i, G_MENU_LINK_SECTION);
		if (sec)
		{
			char *a = action_for_label(sec, label);
			g_object_unref(sec);
			if (a)
				return a;
		}
	}
	return NULL;
}

static char *unprefixed(const char *ref)
{
	const char *dot = strchr(ref, '.');
	return g_strdup(dot ? dot + 1 : ref);
}

char *imported_action_for_label(GMenuModel *m, const char *label)
{
	char *ref = action_for_label(m, label);
	if (!ref)
		return NULL;
	char *a = unprefixed(ref);
	g_free(ref);
	return a;
}

gboolean radio_state_is(GActionGroup *g, const char *action, const char *expected)
{
	GVariant *st = g_action_group_get_action_state(g, action);
	if (!st)
		return FALSE;
	gboolean ok = g_variant_is_of_type(st, G_VARIANT_TYPE_STRING) &&
	              g_strcmp0(g_variant_get_string(st, NULL), expected) == 0;
	g_variant_unref(st);
	return ok;
}

/* ============================================================ */
/* Source menu + actions                                         */

static void build_menu(E2E *e)
{
	g_new_fires = 0;

	e->root     = g_menu_new();
	e->section1 = g_menu_new();
	g_menu_append(e->section1, "New", "app.new");
	g_menu_append(e->section1, "Open", "app.open");
	GMenuItem *check = g_menu_item_new("Auto-save", "app.autosave");
	g_menu_append_item(e->section1, check);
	g_object_unref(check);
	g_menu_append_section(e->root, NULL, G_MENU_MODEL(e->section1));

	e->section2      = g_menu_new();
	GMenuItem *small = g_menu_item_new("Small", "app.size");
	g_menu_item_set_attribute(small, G_MENU_ATTRIBUTE_TARGET, "s", "small");
	g_menu_append_item(e->section2, small);
	g_object_unref(small);

	GMenuItem *large = g_menu_item_new("Large", "app.size");
	g_menu_item_set_attribute(large, G_MENU_ATTRIBUTE_TARGET, "s", "large");
	g_menu_append_item(e->section2, large);
	g_object_unref(large);

	GMenu *sub = g_menu_new();
	g_menu_append(sub, "Sub A", "app.sub_a");
	g_menu_append(sub, "Sub B", "app.sub_b");
	GMenuItem *recent = g_menu_item_new_submenu("Recent", G_MENU_MODEL(sub));
	g_menu_append_item(e->section2, recent);
	g_object_unref(recent);
	g_object_unref(sub);
	g_menu_append_section(e->root, NULL, G_MENU_MODEL(e->section2));

	e->source  = G_MENU_MODEL(e->root);
	e->actions = g_simple_action_group_new();

	GSimpleAction *a;
	a = g_simple_action_new("new", NULL);
	g_signal_connect(a, "activate", G_CALLBACK(on_simple_activate), &g_new_fires);
	g_action_map_add_action(G_ACTION_MAP(e->actions), G_ACTION(a));
	g_object_unref(a);

	a = g_simple_action_new("open", NULL);
	g_action_map_add_action(G_ACTION_MAP(e->actions), G_ACTION(a));
	g_object_unref(a);

	a = g_simple_action_new("save", NULL);
	g_action_map_add_action(G_ACTION_MAP(e->actions), G_ACTION(a));
	g_object_unref(a);

	a = g_simple_action_new("sub_a", NULL);
	g_action_map_add_action(G_ACTION_MAP(e->actions), G_ACTION(a));
	g_object_unref(a);

	a = g_simple_action_new("sub_b", NULL);
	g_action_map_add_action(G_ACTION_MAP(e->actions), G_ACTION(a));
	g_object_unref(a);

	a = g_simple_action_new_stateful("autosave", NULL, g_variant_new_boolean(FALSE));
	g_signal_connect(a, "change-state", G_CALLBACK(checkbox_change_state), NULL);
	g_action_map_add_action(G_ACTION_MAP(e->actions), G_ACTION(a));
	g_object_unref(a);

	a = g_simple_action_new_stateful("size",
	                                 G_VARIANT_TYPE_STRING,
	                                 g_variant_new_string("small"));
	g_signal_connect(a, "change-state", G_CALLBACK(string_change_state), NULL);
	g_action_map_add_action(G_ACTION_MAP(e->actions), G_ACTION(a));
	g_object_unref(a);
}

/* ============================================================ */
/* Setup / teardown                                              */

void e2e_setup(E2E *e)
{
	memset(e, 0, sizeof(*e));
	build_menu(e);

	e->menubar = gtk_menu_bar_new();
	gtk_widget_insert_action_group(e->menubar, "app", G_ACTION_GROUP(e->actions));
	gtk_menu_shell_bind_model(GTK_MENU_SHELL(e->menubar), e->source, NULL, TRUE);

	e->shell         = unity_gtk_menu_shell_new(GTK_MENU_SHELL(e->menubar));
	e->unity_actions = unity_gtk_action_group_new(NULL);
	unity_gtk_action_group_connect_shell(e->unity_actions, e->shell);
	settle(80);

	gchar *bus_name =
	    g_strdup_printf("org.ValaPanel.Test.E2E%u", g_random_int_range(10000, 99999));
	gchar *object_path =
	    g_strdup_printf("/org/ValaPanel/Test/E2E%u", g_random_int_range(10000, 99999));

	e->exporter = dbus_menu_exporter_new(G_MENU_MODEL(e->shell),
	                                     "unity",
	                                     G_ACTION_GROUP(e->unity_actions),
	                                     g_strdup(bus_name),
	                                     g_strdup(object_path));
	settle(120);

	e->importer = dbus_menu_importer_new(bus_name, object_path);
	settle(300);

	g_object_get(e->importer,
	             "model",
	             &e->imported_model,
	             "action-group",
	             &e->imported_actions,
	             NULL);

	g_free(bus_name);
	g_free(object_path);

	load_imported(e);
}

void e2e_teardown(E2E *e)
{
	if (e->imported_actions)
		g_object_unref(e->imported_actions);
	if (e->imported_model)
		g_object_unref(e->imported_model);
	if (e->importer)
		g_object_unref(e->importer);
	if (e->exporter)
		g_object_unref(e->exporter);
	if (e->unity_actions)
		g_object_unref(e->unity_actions);
	if (e->shell)
		g_object_unref(e->shell);
	if (e->menubar)
		gtk_widget_destroy(e->menubar);
	if (e->section2)
		g_object_unref(e->section2);
	if (e->section1)
		g_object_unref(e->section1);
	if (e->root)
		g_object_unref(e->root);
	if (e->actions)
		g_object_unref(e->actions);
	settle(500);
}