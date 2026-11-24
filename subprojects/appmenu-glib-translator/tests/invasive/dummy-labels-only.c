/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* Drives the importer against a hand-written com.canonical.dbusmenu server that
 * has no exporter and no GMenuModel behind it: everything it serves is the one
 * layout variant handed to its constructor. See mock-server.c. */

#include "comparator.h"
#include "mock-server.h"

/* --- building the layout --------------------------------------------------- */

static void add_string_prop(GVariantBuilder *props, const char *name, const char *value)
{
	g_variant_builder_add(props, "{sv}", name, g_variant_new_string(value));
}

/* Opens one item inside the (av) child list of its parent. */
static void begin_item(GVariantBuilder *items, u_int32_t id)
{
	g_variant_builder_open(items, G_VARIANT_TYPE("v"));
	g_variant_builder_open(items, G_VARIANT_TYPE("(ia{sv}av)"));
	g_variant_builder_add(items, "i", (gint32)id);
	g_variant_builder_open(items, G_VARIANT_TYPE("a{sv}"));
}

/* Opens the (av) child list of an item whose properties are complete. */
static void begin_children(GVariantBuilder *items)
{
	g_variant_builder_close(items); /* a{sv} */
	g_variant_builder_open(items, G_VARIANT_TYPE("av"));
}

/* Closes a childless item opened by begin_item. */
static void end_leaf_item(GVariantBuilder *items)
{
	begin_children(items);
	g_variant_builder_close(items); /* av */
	g_variant_builder_close(items); /* (ia{sv}av) */
	g_variant_builder_close(items); /* v */
}

/* The root id must be the parent_id of the importer's top model, which is 0. */
#define ROOT_ID 0

static GVariant *build_frozen_layout(void)
{
	GVariantBuilder layout;
	g_variant_builder_init(&layout, G_VARIANT_TYPE("(ia{sv}av)"));
	g_variant_builder_add(&layout, "i", (gint32)ROOT_ID);

	g_variant_builder_open(&layout, G_VARIANT_TYPE("a{sv}"));
	add_string_prop(&layout,
	                DBUS_MENU_PROP_CHILDREN_DISPLAY,
	                DBUS_MENU_CHILDREN_DISPLAY_SUBMENU);
	begin_children(&layout);

	/* A separator opens the single section of this menu; the importer drops it
	 * once the items behind it are placed. */
	begin_item(&layout, 1);
	add_string_prop(&layout, DBUS_MENU_PROP_TYPE, DBUS_MENU_TYPE_SEPARATOR);
	end_leaf_item(&layout);

	/* Two plain labelled items: no toggle-type and no children-display, so the
	 * importer has to synthesize a plain stateless action for each. */
	begin_item(&layout, 2);
	add_string_prop(&layout, DBUS_MENU_PROP_LABEL, "First item");
	end_leaf_item(&layout);

	begin_item(&layout, 3);
	add_string_prop(&layout, DBUS_MENU_PROP_LABEL, "Second item");
	end_leaf_item(&layout);

	g_variant_builder_close(&layout); /* av */
	return g_variant_builder_end(&layout);
}

/* --- checking what reached the importer ------------------------------------ */

static void check_section(GMenuModel *model, GString *errors)
{
	GMenuModel *section = g_menu_model_get_item_link(model, 0, G_MENU_LINK_SECTION);
	g_assert_nonnull(section);

	int n = g_menu_model_get_n_items(section);
	if (n != 2)
		g_string_append_printf(errors, "expected 2 items in the section, got %d\n", n);

	GActionGroup *group = NULL;
	g_object_get(model, "action-group", &group, NULL);
	g_assert_nonnull(group);

	for (int i = 0; i < n; i++)
	{
		g_autoptr(GVariant) label =
		    g_menu_model_get_item_attribute_value(section,
		                                          i,
		                                          G_MENU_ATTRIBUTE_LABEL,
		                                          G_VARIANT_TYPE_STRING);
		const char *name = label ? g_variant_get_string(label, NULL) : "(no label)";

		g_autoptr(GVariant) action =
		    g_menu_model_get_item_attribute_value(section,
		                                          i,
		                                          G_MENU_ATTRIBUTE_ACTION,
		                                          G_VARIANT_TYPE_STRING);
		if (action == NULL)
		{
			g_string_append_printf(errors, "'%s': no action attribute at all\n", name);
			continue;
		}
		const char *action_name = g_variant_get_string(action, NULL);

		/* The attribute carries the "dbusmenu." namespace prefix while the
		 * importer's own action group stores the bare name, which is what
		 * extract_bare_action strips off for us. Both are checked: the attribute
		 * has to be present and non-empty, the bare name has to resolve. */
		if (action_name == NULL || *action_name == '\0')
		{
			g_string_append_printf(errors, "'%s': action attribute is empty\n", name);
			continue;
		}

		g_autofree char *bare = extract_bare_action(model, name);
		if (bare == NULL)
		{
			g_string_append_printf(errors,
			                       "'%s': no bare action could be extracted\n",
			                       name);
			continue;
		}

		if (!g_action_group_has_action(group, bare))
			g_string_append_printf(errors,
			                       "'%s': action '%s' is missing\n",
			                       name,
			                       bare);
		else if (!g_action_group_get_action_enabled(group, bare))
			g_string_append_printf(errors,
			                       "'%s': action '%s' is NOT enabled\n",
			                       name,
			                       bare);

		/* A plain item carries no state and takes no parameter. */
		if (g_action_group_get_action_parameter_type(group, bare) != NULL)
			g_string_append_printf(errors,
			                       "'%s': action '%s' takes a parameter\n",
			                       name,
			                       bare);
		if (g_action_group_get_action_state(group, bare) != NULL)
			g_string_append_printf(errors, "'%s': action '%s' has state\n", name, bare);
	}

	g_object_unref(group);
	g_object_unref(section);
}

/* --- the test itself ------------------------------------------------------- */

G_GNUC_INTERNAL void test_func(void)
{
	GVariant *layout         = build_frozen_layout();
	g_autofree char *as_text = g_variant_print(layout, TRUE);
	g_print("frozen layout (%s): %s\n", g_variant_get_type_string(layout), as_text);

	/* g_variant_builder_end hands us a floating ref, which the menu consumes and
	 * sinks; there is no extra reference of ours to drop here. */
	DBusMenuMockServer *menu =
	    dbus_menu_mock_server_new(layout, TEST_BUS_NAME, TEST_OBJECT_PATH);
	test_wait();

	DBusMenuImporter *importer = dbus_menu_importer_new(TEST_BUS_NAME, TEST_OBJECT_PATH);
	test_wait();

	GMenuModel *target = NULL;
	g_object_get(importer, "model", &target, NULL);
	g_assert_nonnull(target);

	/* The layout must reach the importer as exactly one section holding the two
	 * labelled items - no exporter and no model on the serving side. */
	if (g_menu_model_get_n_items(target) != 1)
	{
		g_printerr("expected 1 section, got %d\n", g_menu_model_get_n_items(target));
		g_menu_markup_print_to_console(target);
	}
	g_assert_cmpint(g_menu_model_get_n_items(target), ==, 1);

	GString *errors = g_string_new(NULL);
	check_section(target, errors);
	if (errors->len > 0)
	{
		g_printerr("Imported model does not match the frozen layout:\n%s", errors->str);
		g_menu_markup_print_to_console(target);
	}

	g_object_unref(target);
	g_object_unref(importer);
	g_object_unref(menu);
	test_wait();

	int n_errors = errors->len;
	g_string_free(errors, TRUE);
	g_assert_cmpint(n_errors, ==, 0);
}
