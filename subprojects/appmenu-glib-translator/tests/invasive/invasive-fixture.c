/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* Shared scaffolding for the mock-server invasive tests: layout primitives, check
 * helpers and the InvasiveFixture driver. See invasive-fixture.h for what they are for
 * and how each scenario builds its own layouts on top of them. This file is never a
 * test on its own. */

#include "comparator.h"
#include "invasive-fixture.h"

/* --- layout primitives ----------------------------------------------------- */

void invasive_add_string_prop(GVariantBuilder *props, const char *name, const char *value)
{
	g_variant_builder_add(props, "{sv}", name, g_variant_new_string(value));
}

void invasive_add_boolean_prop(GVariantBuilder *props, const char *name, gboolean value)
{
	g_variant_builder_add(props, "{sv}", name, g_variant_new_boolean(value));
}

/* Opens one item in the (av) child list of its parent. */
void invasive_begin_item(GVariantBuilder *items, u_int32_t id)
{
	g_variant_builder_open(items, G_VARIANT_TYPE("v"));
	g_variant_builder_open(items, G_VARIANT_TYPE("(ia{sv}av)"));
	g_variant_builder_add(items, "i", (gint32)id);
	g_variant_builder_open(items, G_VARIANT_TYPE("a{sv}"));
}

/* Opens the (av) child list of an item whose properties are complete. */
void invasive_begin_children(GVariantBuilder *items)
{
	g_variant_builder_close(items); /* a{sv} */
	g_variant_builder_open(items, G_VARIANT_TYPE("av"));
}

/* Closes a childless item opened by invasive_begin_item. */
void invasive_end_leaf_item(GVariantBuilder *items)
{
	invasive_begin_children(items);
	g_variant_builder_close(items); /* av */
	g_variant_builder_close(items); /* (ia{sv}av) */
	g_variant_builder_close(items); /* v */
}

/* Closes the child list of a root item opened by invasive_begin_root. */
void invasive_end_root(GVariantBuilder *layout)
{
	g_variant_builder_close(layout); /* av */
}

/* A titled x-kde-title marker: type=separator paired with x-kde-title and a label,
 * exactly what KDE exports for a titled section. */
void invasive_add_kde_title(GVariantBuilder *layout, u_int32_t id, const char *title)
{
	invasive_begin_item(layout, id);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_TYPE, DBUS_MENU_TYPE_SEPARATOR);
	invasive_add_boolean_prop(layout, DBUS_MENU_PROP_KDE_TITLE, TRUE);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_LABEL, title);
	invasive_end_leaf_item(layout);
}

/* A titled x-kde-title marker that is hidden. Hiding a section marker toggles it, so
 * it opens no section and its label is ignored. */
void invasive_add_hidden_kde_title(GVariantBuilder *layout, u_int32_t id, const char *title)
{
	invasive_begin_item(layout, id);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_TYPE, DBUS_MENU_TYPE_SEPARATOR);
	invasive_add_boolean_prop(layout, DBUS_MENU_PROP_KDE_TITLE, TRUE);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_LABEL, title);
	invasive_add_boolean_prop(layout, DBUS_MENU_PROP_VISIBLE, FALSE);
	invasive_end_leaf_item(layout);
}

/* A section marker without x-kde-title: a plain separator. @label may be NULL, in which
 * case the marker carries no label at all. */
void invasive_add_separator(GVariantBuilder *layout, u_int32_t id, const char *label)
{
	invasive_begin_item(layout, id);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_TYPE, DBUS_MENU_TYPE_SEPARATOR);
	if (label != NULL)
		invasive_add_string_prop(layout, DBUS_MENU_PROP_LABEL, label);
	invasive_end_leaf_item(layout);
}

/* A regular labelled item. */
void invasive_add_label(GVariantBuilder *layout, u_int32_t id, const char *label)
{
	invasive_begin_item(layout, id);
	invasive_add_string_prop(layout, DBUS_MENU_PROP_LABEL, label);
	invasive_end_leaf_item(layout);
}

/* Opens the root item whose id is the importer's parent_id. */
void invasive_begin_root(GVariantBuilder *layout)
{
	g_variant_builder_init(layout, G_VARIANT_TYPE("(ia{sv}av)"));
	g_variant_builder_add(layout, "i", (gint32) 0);

	g_variant_builder_open(layout, G_VARIANT_TYPE("a{sv}"));
	invasive_add_string_prop(layout,
	                    DBUS_MENU_PROP_CHILDREN_DISPLAY,
	                    DBUS_MENU_CHILDREN_DISPLAY_SUBMENU);
	invasive_begin_children(layout);
}

/* --- checks on what reached the importer ----------------------------------- */

/* Returns a NEW ref (the caller owns it). It exists to be passed straight to
 * invasive_check_section_items, which consumes that ref - so call sites use it inline and
 * never leak the link. */
GMenuModel *invasive_section_model(GMenuModel *target, int section_index)
{
	return g_menu_model_get_item_link(target, section_index, G_MENU_LINK_SECTION);
}

void invasive_check_section_label(GMenuModel *target, int section_index, GString *errors, const char *who,
                             const char *expected)
{
	if (section_index >= g_menu_model_get_n_items(target))
	{
		g_string_append_printf(errors, "%s: target has no item %d\n", who, section_index);
		return;
	}

	g_autoptr(GVariant) label = g_menu_model_get_item_attribute_value(target,
	                                                                section_index,
	                                                                G_MENU_ATTRIBUTE_LABEL,
	                                                                G_VARIANT_TYPE_STRING);
	const char *got = label ? g_variant_get_string(label, NULL) : "(no label)";

	if (g_strcmp0(got, expected ? expected : "(no label)") != 0)
		g_string_append_printf(errors,
		                       "%s: section label is '%s', expected '%s'\n",
		                       who,
		                       got,
		                       expected ? expected : "(no label)");
}

/* Consumes the reference returned by invasive_section_model: it always unrefs @section
 * before returning, so passing invasive_section_model(...) inline leaks nothing. */
void invasive_check_section_items(GMenuModel *section, GString *errors, const char *who, int expected_n,
                             const char *const *expected_labels)
{
	if (section == NULL)
	{
		g_string_append_printf(errors, "%s: has no section link\n", who);
		return;
	}

	int n = g_menu_model_get_n_items(section);
	if (n != expected_n)
		g_string_append_printf(errors, "%s: expected %d items, got %d\n", who, expected_n, n);

	for (int i = 0; i < MIN(n, expected_n); i++)
	{
		g_autoptr(GVariant) label =
		    g_menu_model_get_item_attribute_value(section, i, G_MENU_ATTRIBUTE_LABEL, G_VARIANT_TYPE_STRING);
		const char *got = label ? g_variant_get_string(label, NULL) : "(no label)";

		if (g_strcmp0(got, expected_labels[i]) != 0)
			g_string_append_printf(errors,
			                       "%s: item %d is '%s', expected '%s'\n",
			                       who,
			                       i,
			                       got,
			                       expected_labels[i]);

		g_autoptr(GVariant) action =
		    g_menu_model_get_item_attribute_value(section, i, G_MENU_ATTRIBUTE_ACTION, G_VARIANT_TYPE_STRING);
		if (action == NULL || g_variant_get_string(action, NULL)[0] == '\0')
			g_string_append_printf(errors, "%s: '%s' carries no action attribute\n", who, got);
	}

	g_object_unref(section);
}

void invasive_check_n_sections(GMenuModel *target, GString *errors, const char *who, int expected)
{
	int n = g_menu_model_get_n_items(target);
	if (n != expected)
		g_string_append_printf(errors, "%s: expected %d sections, got %d\n", who, expected, n);
}

void invasive_check_internal_label(DBusMenuModel *model, int section_index, GString *errors, const char *who,
                              const char *expected)
{
	DBusMenuItem *item = dbus_menu_model_find_item(model, section_index, -1);
	if (item == NULL)
	{
		g_string_append_printf(errors, "%s: no internal section item\n", who);
		return;
	}

	GVariant *label = g_hash_table_lookup(item->attrs, G_MENU_ATTRIBUTE_LABEL);
	const char *got = label ? g_variant_get_string(label, NULL) : "(no label)";

	if (g_strcmp0(got, expected ? expected : "(no label)") != 0)
		g_string_append_printf(errors,
		                       "%s: internal section attrs hold '%s', expected '%s'\n",
		                       who,
		                       got,
		                       expected ? expected : "(no label)");
}

u_int32_t invasive_section_item_id(DBusMenuModel *model, int section_index)
{
	DBusMenuItem *item = dbus_menu_model_find_item(model, section_index, -1);
	return item == NULL ? 0 : item->id;
}

void invasive_dump(const char *tag, GMenuModel *target)
{
	g_print("--- %s ---\n", tag);
	dbus_menu_print_stdout(target);
}

/* --- driving the mock server ----------------------------------------------- */

void invasive_scenario_start(InvasiveFixture *s, GVariant *layout, const char *bus_name)
{
	s->server   = dbus_menu_mock_server_new(layout, bus_name, TEST_OBJECT_PATH);
	s->importer = dbus_menu_importer_new(bus_name, TEST_OBJECT_PATH);

	for (int i = 0; i < 10 && s->target == NULL; i++)
	{
		test_wait();
		GMenuModel *model = NULL;
		g_object_get(s->importer, "model", &model, NULL);
		if (model != NULL && g_menu_model_get_n_items(model) > 0)
			s->target = model;
		else
			g_clear_object(&model);
	}

	g_assert_nonnull(s->target);
	/* target IS the importer's top model (a DBusMenuModel). We keep a borrowed
	 * pointer for the invasive helpers rather than an extra strong ref: an extra ref
	 * here would be dropped in stop() but, combined with the deferred parse source,
	 * leaves the first-section link alive past exit and LSan reports it. The importer
	 * owns the model and keeps it alive for our whole scenario. */
	s->model = DBUS_MENU_MODEL(s->target);
}

void invasive_scenario_stop(InvasiveFixture *s)
{
	/* s->model is borrowed (no ref) — just clear the pointer. Drop target before the
	 * importer so, as in the roundtrip fixture, the model's last test-side ref goes
	 * away and the importer disposes its top_model cleanly. */
	s->model = NULL;
	g_clear_object(&s->target);
	g_clear_object(&s->importer);
	g_clear_object(&s->server);
	test_wait();
}

/* Swaps the layout the mock server answers with.
 *
 * "layout" is G_PARAM_READWRITE (CONSTRUCT but not CONSTRUCT_ONLY), so a plain
 * g_object_set drives it: the setter sinks its own reference, reindexes and emits
 * LayoutUpdated. The layout we hand in is floating from the builder, so we sink it
 * into an autoptr first; g_object_set takes its own reference on top of that and we
 * drop ours at scope end. */
void invasive_scenario_serve(InvasiveFixture *s, GVariant *layout)
{
	g_autoptr(GVariant) owned = g_variant_ref_sink(layout);
	g_object_set(s->server, "layout", owned, NULL);

	/* LayoutUpdated makes the importer ask for a fresh layout; the parse itself is
	 * deferred behind a timeout source, so give it two rounds. */
	test_wait();
}

/* --- shared runner --------------------------------------------------------- */

void invasive_scenario_run(void (*scenario)(GString *errors))
{
	GString *errors = g_string_new(NULL);

	scenario(errors);

	if (errors->len > 0)
		g_printerr("invasive checks mismatched:\n%s", errors->str);

	int n_errors = (int) errors->len;
	g_string_free(errors, TRUE);
	g_assert_cmpint(n_errors, ==, 0);
}
