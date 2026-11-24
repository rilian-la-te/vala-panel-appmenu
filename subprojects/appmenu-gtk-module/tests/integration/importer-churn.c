/* SPDX-License-Identifier: LGPL-3.0-or-later */
/*
 * importer-churn: put the importer onto a widget, then do nasty things to the
 * source model (insert/remove items and whole sections, rapid add+remove with
 * no settle in between, remove-all/refill, submenu churn). The point is to
 * hammer the translator's add/remove paths so any lifetime bug surfaces as a
 * crash / ASan-UBSan report. Final-state correctness is asserted only at
 * quiescent points; the real pass condition is "no crash".
 */
#include "test-helper.h"

const char *test_path = "/e2e/importer/churn";

/* Put the importer onto a widget: bind the imported (DBus-derived) model and
 * its action group to a GtkMenu shell, exactly like a panel applet does. The
 * lazy tracker then subscribes to and walks every item under it. */
static GtkWidget *attach_importer(E2E *e)
{
	GtkWidget *menu = gtk_menu_new();
	gtk_widget_insert_action_group(menu, "app", e->imported_actions);
	gtk_menu_shell_bind_model(GTK_MENU_SHELL(menu), e->imported_model, NULL, FALSE);
	return menu;
}

static void detach_importer(E2E *e, GtkWidget *menu)
{
	/* Tear down in the same order GTK's own menu-shell dispose uses: drop the
	 * model / importer FIRST (this frees the vendored exporter items and closes
	 * their submenu openers while the "app" provider is still installed on the
	 * widget), and only THEN remove the action group. Removing the provider
	 * before the openers close leaves each change_state() with a NULL route, so
	 * nothing sinks its floating boolean and it leaks. */
	gtk_menu_shell_bind_model(GTK_MENU_SHELL(menu), NULL, NULL, FALSE);

	if (e->imported_model)
	{
		g_object_unref(e->imported_model);
		e->imported_model = NULL;
	}
	if (e->importer)
	{
		g_object_unref(e->importer);
		e->importer = NULL;
	}

	gtk_widget_insert_action_group(menu, "app", NULL);
	gtk_widget_destroy(menu);
	pump_events();
}

/* Re-point a bound menu at the (possibly replaced) imported model without
 * tearing the widget down. */
static void rebind_importer(E2E *e, GtkWidget *menu)
{
	load_imported(e); /* may swap e->imported_model if the root changed */
	gtk_menu_shell_bind_model(GTK_MENU_SHELL(menu), e->imported_model, NULL, FALSE);
	settle(80);
}

static guint n_items(GMenu *m)
{
	return (guint)g_menu_model_get_n_items(G_MENU_MODEL(m));
}

void test_func(void)
{
	E2E e;
	e2e_setup(&e);
	assert_models(&e, "churn/initial");

	GtkWidget *menu = attach_importer(&e);
	settle(80);

	/* ---- 1. Insert at front / middle / end of an existing section ---- */
	g_menu_prepend(e.section1, "Front", "app.open");
	g_menu_insert(e.section1, 2, "Middle", "app.save");
	g_menu_append(e.section1, "End", "app.new");
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-inserts");

	/* ---- 2. Remove them back out (last, then front) ---- */
	g_menu_remove(e.section1, n_items(e.section1) - 1); /* "End"   */
	g_menu_remove(e.section1, 0);                       /* "Front" */
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-removes");

	/* ---- 3. Rapid add+remove with NO settle in between: updates coalesce
	 * mid-flight and may reference items that vanished before delivery. ---- */
	for (int i = 0; i < 25; i++)
	{
		g_autofree char *nm = g_strdup_printf("Rapid-%d", i);
		g_menu_append(e.section1, nm, "app.new");
		g_menu_remove(e.section1, n_items(e.section1) - 1);
	}
	pump_events(); /* deliberately no settle: deliver whatever is queued */
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-rapid");

	/* ---- 4. Empty a section completely, then refill it. We deliberately do
	 * NOT assert at the instantaneous empty state: by design the importer cannot
	 * hold an entirely empty section (layout_parse_items / finalize_section prune
	 * and compact them), so an empty source section legitimately does not
	 * round-trip as a section. The churn through empty is still exercised; we
	 * only compare once it is non-empty again (refill). ---- */
	while (n_items(e.section2) > 0)
		g_menu_remove(e.section2, 0);
	settle(100); /* churns through the empty-section code path; no exact-match assert */

	/* Try to force MULTIPLE empty sections at once. By design the importer must
	 * collapse these to at most one — this is a no-crash probe, not an exact-
	 * round-trip check: we only require that the pipeline keeps running and the
	 * quiescent state still has section1 intact (>= 1 non-empty section). */
	GMenu *empty_a = g_menu_new();
	GMenu *empty_b = g_menu_new();
	g_menu_append_section(e.root, NULL, G_MENU_MODEL(empty_a));
	g_menu_append_section(e.root, NULL, G_MENU_MODEL(empty_b));
	rebind_importer(&e, menu);
	CHECK(&e, "churn/multi-empty-alive", g_menu_model_get_n_items(e.imported_model) >= 1);
	g_menu_remove(e.root, n_items(e.root) - 1);
	g_menu_remove(e.root, n_items(e.root) - 1);
	g_object_unref(empty_a);
	g_object_unref(empty_b);
	settle(100);

	g_menu_append(e.section2, "Small", "app.size"); /* rebuild section2 */
	g_menu_append(e.section2, "Large", "app.size");
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-refill");

	/* ---- 5. Whole-section churn at the root: add a section, then remove it,
	 * while the importer widget stays bound the whole time. ---- */
	GMenu *extra = g_menu_new();
	g_menu_append(extra, "Extra", "app.open");
	g_menu_append_section(e.root, NULL, G_MENU_MODEL(extra));
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-add-section");

	g_menu_remove(e.root, n_items(e.root) - 1);
	g_object_unref(extra);
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-remove-section");

	/* ---- 6. Submenu churn through a freshly attached submenu link: append an
	 * item that owns a nested model, mutate the nested model, then remove it.
	 * The chain source → exporter → importer is exercised while bound. ---- */
	GMenu *live = g_menu_new();
	g_menu_append(live, "Live A", "app.sub_a");
	GMenuItem *recent_item = g_menu_item_new_submenu("Recent2", G_MENU_MODEL(live));
	g_menu_append_item(e.section1, recent_item);
	g_object_unref(recent_item);
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-add-submenu");

	/* mutate the nested submenu contents while it is referenced through the chain */
	g_menu_append(live, "Live B", "app.sub_b");
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-submenu-mutate");

	/* remove the submenu item; `live` stays owned by us for teardown */
	g_menu_remove(e.section1, n_items(e.section1) - 1);
	rebind_importer(&e, menu);
	assert_models(&e, "churn/after-remove-submenu");

	/* ---- 7. Detach the importer widget, then tear down cleanly. ---- */
	detach_importer(&e, menu);
	g_object_unref(live);

	e2e_teardown(&e);
}
