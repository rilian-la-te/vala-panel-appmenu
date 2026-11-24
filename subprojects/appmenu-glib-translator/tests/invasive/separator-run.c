/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include <stdio.h>

#include "comparator.h"
#include "importer.h"
#include "mock-server.h"

/* --- building the layout --------------------------------------------------- */

static void add_string_prop(GVariantBuilder *props, const char *name, const char *value)
{
	g_variant_builder_add(props, "{sv}", name, g_variant_new_string(value));
}

static void add_boolean_prop(GVariantBuilder *props, const char *name, gboolean value)
{
	g_variant_builder_add(props, "{sv}", name, g_variant_new_boolean(value));
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

/* A type=separator marker. The importer only turns one of those into a section
 * while it also carries x-kde-title, so both properties are set together. */
static void add_separator(GVariantBuilder *layout, u_int32_t id)
{
	begin_item(layout, id);
	add_string_prop(layout, DBUS_MENU_PROP_TYPE, DBUS_MENU_TYPE_SEPARATOR);
	add_boolean_prop(layout, DBUS_MENU_PROP_KDE_TITLE, TRUE);
	end_leaf_item(layout);
}

static void add_label(GVariantBuilder *layout, u_int32_t id, const char *label)
{
	begin_item(layout, id);
	add_string_prop(layout, DBUS_MENU_PROP_LABEL, label);
	end_leaf_item(layout);
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

	for (u_int32_t id = 28; id <= 38; id++)
		add_separator(&layout, id);
	/* 1: x-kde-title. It is the only marker that can carry a label through to
	 * the model, so it doubles as the title of the first section. */
	begin_item(&layout, 1);
	add_boolean_prop(&layout, DBUS_MENU_PROP_KDE_TITLE, TRUE);
	add_string_prop(&layout, DBUS_MENU_PROP_LABEL, "Kde titled separator");
	end_leaf_item(&layout);

	/* 2: type=separator */
	add_separator(&layout, 2);

	/* 3-4: two meaningful items */
	add_label(&layout, 3, "First item");
	add_label(&layout, 4, "Second item");

	/* 5-6: two separators */
	add_separator(&layout, 5);
	add_separator(&layout, 6);

	/* 7: one meaningful item */
	add_label(&layout, 7, "Third item");

	/* 8-12: five separators */
	for (u_int32_t id = 8; id <= 12; id++)
		add_separator(&layout, id);

	g_variant_builder_close(&layout); /* av */
	return g_variant_builder_end(&layout);
}

/* --- checking what reached the importer ------------------------------------ */

static void check_labelled_items(GMenuModel *section, GString *errors, const char *who,
                                 int expected_n, const char *const *expected_labels)
{
	int n = g_menu_model_get_n_items(section);

	if (n != expected_n)
		g_string_append_printf(errors,
		                       "%s: expected %d items, got %d\n",
		                       who,
		                       expected_n,
		                       n);

	for (int i = 0; i < MIN(n, expected_n); i++)
	{
		g_autoptr(GVariant) label =
		    g_menu_model_get_item_attribute_value(section,
		                                          i,
		                                          G_MENU_ATTRIBUTE_LABEL,
		                                          G_VARIANT_TYPE_STRING);
		const char *got = label ? g_variant_get_string(label, NULL) : "(no label)";

		if (g_strcmp0(got, expected_labels[i]) != 0)
			g_string_append_printf(errors,
			                       "%s: item %d has label '%s', expected '%s'\n",
			                       who,
			                       i,
			                       got,
			                       expected_labels[i]);

		/* Every meaningful item has to stay actionable. */
		g_autoptr(GVariant) action =
		    g_menu_model_get_item_attribute_value(section,
		                                          i,
		                                          G_MENU_ATTRIBUTE_ACTION,
		                                          G_VARIANT_TYPE_STRING);
		if (action == NULL || g_variant_get_string(action, NULL)[0] == '\0')
			g_string_append_printf(errors,
			                       "%s: '%s' carries no action attribute\n",
			                       who,
			                       expected_labels[i]);
	}
}

/* --- the test itself ------------------------------------------------------- */

G_GNUC_INTERNAL void test_func(void)
{
	static const char *const first_section_items[]  = {"First item", "Second item"};
	static const char *const second_section_items[] = {"Third item"};

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
	int n_sections = g_menu_model_get_n_items(target);
	if (n_sections != 2)
	{
		g_printerr("expected 2 sections, got %d\n", n_sections);
		g_menu_markup_print_to_console(target);
	}
	g_assert_cmpint(n_sections, ==, 2);

	GString *errors = g_string_new(NULL);

	for (int s = 0; s < n_sections; s++)
	{
		GMenuModel *section = g_menu_model_get_item_link(target, s, G_MENU_LINK_SECTION);
		if (section == NULL)
		{
			g_string_append_printf(errors, "top level item %d has no section link\n", s);
			continue;
		}

		char who[32];
		snprintf(who, sizeof(who), "section %d", s);

		if (s == 0)
			check_labelled_items(section, errors, who, 2, first_section_items);
		else if (s == 1)
			check_labelled_items(section, errors, who, 1, second_section_items);
		else
			check_labelled_items(section, errors, who, 0, NULL);

		g_object_unref(section);
	}

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
