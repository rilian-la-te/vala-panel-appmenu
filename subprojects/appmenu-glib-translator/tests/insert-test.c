/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"
#include "exporter/exporter.h"
#include "importer/importer.h"
#include <debug.h>

typedef struct {
    GMenuModel         *source;
    GSimpleActionGroup *action_group;
    DBusMenuExporter   *exporter;
    DBusMenuImporter   *importer;
    GMenuModel         *target;
} Fixture;

/* ------------------------------ timing ------------------------------ */

static gboolean
on_timeout(gpointer data)
{
    g_main_loop_quit((GMainLoop *)data);
    return G_SOURCE_REMOVE;
}

static void
wait(void)
{
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_timeout_add(500, on_timeout, loop);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
}

/* ------------------------------ helpers ----------------------------- */

static void
dump_and_report(const char *tag, Fixture *f, GString *errors)
{
    g_printerr("%s failed:\n%s\n", tag, errors->str);
    g_printerr("--- source ---\n");
    g_menu_markup_print_to_console(f->source);
    g_printerr("--- parsed ---\n");
    g_menu_markup_print_to_console(f->target);
}

static void
fixture_teardown(Fixture *f)
{
    g_object_unref(f->target);
    g_object_unref(f->importer);
    g_object_unref(f->exporter);
    g_object_unref(f->action_group);
    g_object_unref(f->source);
    wait();
}

/* ================================================================== */
/* radio-selection-change with dynamic insert/remove                  */
/* ================================================================== */

static void
size_state_change(GSimpleAction *action, GVariant *value, gpointer user_data)
{
    (void)user_data;
    g_simple_action_set_state(action, value);
}

static void
test_radio_insert_remove(void)
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

    GSimpleActionGroup *action_group = g_simple_action_group_new();

    GSimpleAction *size_action = g_simple_action_new_stateful(
        "size", G_VARIANT_TYPE_STRING, g_variant_new_string("small"));
    g_signal_connect(size_action, "change-state",
                     G_CALLBACK(size_state_change), NULL);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(size_action));

    Fixture f;
    f.source       = G_MENU_MODEL(root);
    f.action_group = g_object_ref_sink(action_group);
    f.exporter     = dbus_menu_exporter_new(f.source,
                                            NULL, NULL,   /* prefix, group */
                                            TEST_BUS_NAME, TEST_OBJECT_PATH);
    wait();
    f.importer = dbus_menu_importer_new(TEST_BUS_NAME, TEST_OBJECT_PATH);
    wait();
    g_object_get(f.importer, "model", &f.target, NULL);

    GString *errors = g_string_new(NULL);

    /* --- 1. Before insert: source and target must NOT be equal. --- */
    gboolean equal = compare_models(f.source, f.target, errors, 0);
    if (equal)
        dump_and_report("radio-insert-remove/pre-insert (expected differ)",
                        &f, errors);
    g_assert_false(equal);
    g_string_set_size(errors, 0);

    /* --- 2. Insert the action group under prefix "app". --- */
    dbus_menu_exporter_insert(f.exporter,
                              "app", G_ACTION_GROUP(action_group));
    wait();

    /* --- 3. After insert: source and target MUST be equal. --- */
    equal = compare_models(f.source, f.target, errors, 0);
    if (!equal)
        dump_and_report("radio-insert-remove/post-insert (expected equal)",
                        &f, errors);
    g_assert_true(equal);
    g_string_set_size(errors, 0);

    /* --- 4. Remove the action group. --- */
    dbus_menu_exporter_remove(f.exporter, "app");
    wait();

    /* --- 5. After remove: source and target must NOT be equal again. --- */
    equal = compare_models(f.source, f.target, errors, 0);
    if (equal)
        dump_and_report("radio-insert-remove/post-remove (expected differ)",
                        &f, errors);
    g_assert_false(equal);

    g_string_free(errors, TRUE);
    g_object_unref(size_action);
    g_object_unref(action_group);
    fixture_teardown(&f);
    g_object_unref(section);
}

/* ================================================================== */

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    GTestDBus *bus = g_test_dbus_new(G_TEST_DBUS_NONE);
    g_test_dbus_up(bus);

    g_test_add_func("/dbusmenu/radio-insert-remove",
                    test_radio_insert_remove);

    int ret = g_test_run();

    g_test_dbus_down(bus);
    g_object_unref(bus);

    return ret;
}