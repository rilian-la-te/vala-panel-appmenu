/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <string.h>

#include "comparator.h"

/* Locate the item with the given label.  Section-wise / positional
 * equality between the two models must not be assumed, so descend
 * through both SECTION and SUBMENU links. */
static GMenuModel *find_item_section(GMenuModel *model, const char *label,
                                     int *out_index)
{
	int n = g_menu_model_get_n_items(model);
	for (int i = 0; i < n; i++) {
		char *item_label = NULL;
		gboolean match =
		    g_menu_model_get_item_attribute(model, i,
		                                    G_MENU_ATTRIBUTE_LABEL, "s",
		                                    &item_label)
		    && g_strcmp0(item_label, label) == 0;
		g_free(item_label);
		if (match) {
			*out_index = i;
			return g_object_ref(model);
		}
		GMenuModel *sub =
		    g_menu_model_get_item_link(model, i, G_MENU_LINK_SECTION);
		if (sub) {
			GMenuModel *found = find_item_section(sub, label, out_index);
			g_object_unref(sub);
			if (found)
				return found;
		}
		sub = g_menu_model_get_item_link(model, i, G_MENU_LINK_SUBMENU);
		if (sub) {
			GMenuModel *found = find_item_section(sub, label, out_index);
			g_object_unref(sub);
			if (found)
				return found;
		}
	}
	return NULL;
}

/* Models carry the export prefix ("app.my-action", "dbusmenu.id-15"),
 * while the importer's action group stores bare names ("my-action",
 * "id-15").  Cut at the first dot. */
static char *bare_action_name(const char *name)
{
	const char *dot = strchr(name, '.');
	return g_strdup(dot ? dot + 1 : name);
}

static char *extract_bare_action(GMenuModel *model, const char *label)
{
	int idx;
	GMenuModel *sec = find_item_section(model, label, &idx);
	if (!sec)
		return NULL;
	GVariant *v = g_menu_model_get_item_attribute_value(
	    sec, idx, G_MENU_ATTRIBUTE_ACTION, NULL);
	g_object_unref(sec);
	if (!v)
		return NULL;
	g_assert_true(g_variant_is_of_type(v, G_VARIANT_TYPE_STRING));
	char *full = g_variant_dup_string(v, NULL);
	g_variant_unref(v);
	char *bare = bare_action_name(full);
	g_free(full);
	return bare;
}

static void dump_models(const char *tag, GMenuModel *source, GMenuModel *target)
{
	g_printerr("\n===== %s =====\n", tag);
	g_printerr("--- source ---\n");
	g_menu_markup_print_to_console(source);
	g_printerr("--- target ---\n");
	if (target)
		g_menu_markup_print_to_console(target);
	else
		g_printerr("(NULL)\n");
	g_printerr("===== end %s =====\n\n", tag);
}

G_GNUC_INTERNAL void test_func(void)
{
	GMenu *root    = g_menu_new();
	GMenu *section = g_menu_new();

	GMenuItem *item = g_menu_item_new("Item", "app.my-action");
	g_menu_item_set_attribute(item, G_MENU_ATTRIBUTE_HIDDEN_WHEN, "s",
	                          "action-missing");
	g_menu_append_item(section, item);
	g_object_unref(item);
	g_menu_append_section(root, NULL, G_MENU_MODEL(section));

	GSimpleActionGroup *action_group = g_simple_action_group_new();

	/* Put the action into the group FIRST so the item is not missing. */
	GSimpleAction *my_action = g_simple_action_new("my-action", NULL);
	g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(my_action));

	Fixture f;
	fixture_setup(&f, G_MENU_MODEL(root), action_group);

	GActionGroup *importer_group = NULL;
	g_object_get(f.importer, "action-group", &importer_group, NULL);
	g_assert_nonnull(importer_group);

	char *name;
	int idx;
	GMenuModel *sec;

	/* ===== Stage 1: action present =====
	 * The item must be in the target model, and the action extracted from
	 * the second model (prefixed, so strip at the first dot) must exist in
	 * the importer's action group.  hidden-when is NOT checked here — the
	 * tracker/importer is free to drop it while the action is live. */
	dump_models("stage 1: action present", f.source, f.target);

	sec = find_item_section(f.target, "Item", &idx);
	if (!sec)
		dump_models("stage 1: item not found", f.source, f.target);
	g_assert_nonnull(sec);
	g_object_unref(sec);

	name = extract_bare_action(f.target, "Item");
	g_assert_nonnull(name);
	g_assert_true(g_action_group_has_action(importer_group, name));
	g_free(name);

	/* ===== Stage 2: action removed =====
	 * Two acceptable outcomes:
	 *
	 *   (a) the item is gone from the target model (GtkMenuTracker dropped
	 *       it before it ever reached D-Bus), or
	 *   (b) the item is still there with hidden-when=action-missing, and
	 *       the action it now references is missing in the importer's
	 *       group.
	 *
	 * In both cases the source model keeps the item, and its action no
	 * longer resolves in the source action group. */
	g_action_map_remove_action(G_ACTION_MAP(action_group), "my-action");
	test_wait();
	dump_models("stage 2: action missing", f.source, f.target);

	name = extract_bare_action(f.source, "Item");
	g_assert_nonnull(name);
	g_assert_false(g_action_group_has_action(G_ACTION_GROUP(action_group),
	                                         name));
	g_free(name);

	sec = find_item_section(f.target, "Item", &idx);
	if (sec) {
		GVariant *hw = g_menu_model_get_item_attribute_value(
		    sec, idx, G_MENU_ATTRIBUTE_HIDDEN_WHEN, NULL);
		g_assert_nonnull(hw);
		g_assert_true(g_variant_is_of_type(hw, G_VARIANT_TYPE_STRING));
		g_assert_cmpstr(g_variant_get_string(hw, NULL), ==, "action-missing");
		g_variant_unref(hw);
		g_object_unref(sec);

		name = extract_bare_action(f.target, "Item");
		g_assert_nonnull(name);
		g_assert_false(g_action_group_has_action(importer_group, name));
		g_free(name);
	}
	/* else (a): item absent — already "not visible". */

	/* ===== Stage 3: action re-added =====
	 * Item must reappear, and the freshly extracted bare action name must
	 * resolve in the importer's group again.  hidden-when is NOT checked
	 * here either. */
	g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(my_action));
	test_wait();
	dump_models("stage 3: action present again", f.source, f.target);

	sec = find_item_section(f.target, "Item", &idx);
	if (!sec)
		dump_models("stage 3: item not found", f.source, f.target);
	g_assert_nonnull(sec);
	g_object_unref(sec);

	name = extract_bare_action(f.target, "Item");
	g_assert_nonnull(name);
	g_assert_true(g_action_group_has_action(importer_group, name));
	g_free(name);

	g_object_unref(importer_group);
	g_object_unref(my_action);
	fixture_teardown(&f);
	g_object_unref(section);
}
