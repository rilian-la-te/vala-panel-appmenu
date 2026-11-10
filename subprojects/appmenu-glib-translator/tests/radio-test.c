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

/* ------------------------------ fixture ----------------------------- */

static void
fixture_setup(Fixture *f, GMenuModel *source, GSimpleActionGroup *action_group)
{
    f->source       = source;
    f->action_group = g_object_ref_sink(action_group);
    f->exporter     = dbus_menu_exporter_new(source, "app",
                                             G_ACTION_GROUP(f->action_group),
                                             TEST_BUS_NAME, TEST_OBJECT_PATH);
    wait();
    f->importer = dbus_menu_importer_new(TEST_BUS_NAME, TEST_OBJECT_PATH);
    wait();
    g_object_get(f->importer, "model", &f->target, NULL);
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

/* -------- presence-only action check (без unity-префикса) ---------- */

static void
check_has_any_action(GMenuModel *model, gboolean *found)
{
    int n = g_menu_model_get_n_items(model);

    for (int i = 0; i < n && !*found; i++)
    {
        char *action = NULL;
        if (g_menu_model_get_item_attribute(model, i,
                                            G_MENU_ATTRIBUTE_ACTION,
                                            "s", &action) &&
            action != NULL)
            *found = TRUE;
        g_free(action);

        GMenuModel *sub = g_menu_model_get_item_link(model, i, G_MENU_LINK_SUBMENU);
        if (sub != NULL)
        {
            check_has_any_action(sub, found);
            g_object_unref(sub);
        }
        GMenuModel *sec = g_menu_model_get_item_link(model, i, G_MENU_LINK_SECTION);
        if (sec != NULL)
        {
            check_has_any_action(sec, found);
            g_object_unref(sec);
        }
    }
}

static gboolean
compare_and_check_actions(Fixture *f, GString *errors)
{
    gboolean ok = compare_models(f->source, f->target, errors, 0);

    gboolean has_actions = FALSE;
    check_has_any_action(f->target, &has_actions);
    if (!has_actions)
    {
        g_string_append(errors,
                        "parsed model contains no action attributes; "
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
    g_menu_markup_print_to_console(f->target);
}

/* ================================================================== */
/* 1. dynamic modification                                            */
/* ================================================================== */

static void
test_dynamic_modification(void)
{
    GMenu *root     = g_menu_new();
    GMenu *section1 = g_menu_new();
    g_menu_append(section1, "New", "app.new");
    g_menu_append_section(root, NULL, G_MENU_MODEL(section1));

    Fixture f;
    fixture_setup(&f, G_MENU_MODEL(root), g_simple_action_group_new());

    GString *errors = g_string_new(NULL);

    gboolean ok = compare_and_check_actions(&f, errors);
    if (!ok)
        dump_and_report("dynamic/first comparison", &f, errors);
    g_assert_true(ok);
    g_string_set_size(errors, 0);

    /* Change model - should not desync */
    g_menu_append(section1, "Open", "app.open");
    g_menu_append(section1, "Save", "app.save");

    GMenu *section2 = g_menu_new();
    g_menu_append(section2, "Quit", "app.quit");
    g_menu_append_section(root, NULL, G_MENU_MODEL(section2));
    g_object_unref(section2);

    wait();

    ok = compare_and_check_actions(&f, errors);
    if (!ok)
        dump_and_report("dynamic/second comparison", &f, errors);
    g_assert_true(ok);

    g_string_free(errors, TRUE);
    fixture_teardown(&f);
    g_object_unref(section1);
}

/* ================================================================== */
/* 2. submenu-action                                                  */
/* ================================================================== */

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

static void
on_load_recent_change_state(GSimpleAction *action, GVariant *value,
                            gpointer user_data)
{
    (void)user_data;
    g_simple_action_set_state(action, value);
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

    GMenuItem *recent = g_menu_item_new_submenu("Recent",
                                                G_MENU_MODEL(g_dynamic_submenu));
    g_menu_item_set_attribute(recent,
                              G_MENU_ATTRIBUTE_SUBMENU_ACTION,
                              "s", "app.load-recent");
    g_menu_append_item(section, recent);
    g_object_unref(recent);

    g_menu_append(section, "Quit", "app.quit");
    g_menu_append_section(root, NULL, G_MENU_MODEL(section));
    g_object_unref(section);

    GSimpleAction *load_action = g_simple_action_new_stateful(
        "load-recent", G_VARIANT_TYPE_BOOLEAN, g_variant_new_boolean(FALSE));
    g_signal_connect(load_action, "change-state",
                     G_CALLBACK(on_load_recent_change_state), NULL);

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
    wait();

    g_assert_true(g_dynamic_loaded);

    ok = compare_and_check_actions(&f, errors);
    if (!ok)
        dump_and_report("submenu-action/post-request comparison", &f, errors);
    g_assert_true(ok);

    g_action_group_change_action_state(G_ACTION_GROUP(trigger_group),
                                       "load-recent",
                                       g_variant_new_boolean(FALSE));

    wait();
    g_string_free(errors, TRUE);
    g_object_unref(trigger_group);
    g_object_unref(load_action);
    fixture_teardown(&f);
    g_clear_object(&g_dynamic_submenu);
}

/* ================================================================== */
/* 3. radio-selection-change                                          */
/* ================================================================== */

static void
size_state_change(GSimpleAction *action, GVariant *value, gpointer user_data)
{
    (void)user_data;
    g_simple_action_set_state(action, value);
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

    GSimpleActionGroup *action_group = g_simple_action_group_new();

    GSimpleAction *size_action = g_simple_action_new_stateful(
        "size", G_VARIANT_TYPE_STRING, g_variant_new_string("small"));
    g_signal_connect(size_action, "change-state",
                     G_CALLBACK(size_state_change), NULL);
    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(size_action));

    Fixture f;
    fixture_setup(&f, G_MENU_MODEL(root), action_group);

    GString *errors = g_string_new(NULL);

    /* --- Initial state ("small"). --- */
    gboolean ok = compare_and_check_actions(&f, errors);
    if (!ok)
        dump_and_report("radio-selection-change/initial", &f, errors);
    g_assert_true(ok);
    g_string_set_size(errors, 0);

    /* --- string state change. --- */
    g_action_group_change_action_state(G_ACTION_GROUP(action_group),
                                       "size",
                                       g_variant_new_string("large"));
    wait();

    /* --- string state change works? --- */
    GVariant *state = g_action_group_get_action_state(
        G_ACTION_GROUP(action_group), "size");
    g_assert_nonnull(state);
    g_assert_cmpstr(g_variant_get_string(state, NULL), ==, "large");
    g_variant_unref(state);

    /* --- Compare models again. --- */
    ok = compare_and_check_actions(&f, errors);
    if (!ok)
        dump_and_report("radio-selection-change/after-change", &f, errors);
    g_assert_true(ok);

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

    g_test_add_func("/dbusmenu/dynamic-modification",   test_dynamic_modification);
    g_test_add_func("/dbusmenu/radio-selection-change", test_radio_selection_change);
    g_test_add_func("/dbusmenu/submenu-action",         test_submenu_action);

    int ret = g_test_run();

    g_test_dbus_down(bus);
    g_object_unref(bus);

    return ret;
}