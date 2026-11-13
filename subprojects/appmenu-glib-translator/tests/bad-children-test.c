/* SPDX-License-Identifier: LGPL-3.0-or-later */

#include "comparator.h"
#include "glib.h"
#include "glibconfig.h"

/*
 * Regression test: flipping the source's submenu-action state must not
 * make the exporter re-announce "children-display" for the item that
 * owns the submenu.
 *
 * Setup: a submenu-owning item gated by a boolean submenu-action whose
 * initial state is FALSE.  The importer exposes a synthesized
 * "dbusmenu.submenu-N" action for the submenu item; activating it
 * simulates a user click, which flips the source's submenu-action
 * state to TRUE.  That flip is exactly what used to leak a spurious
 * "children-display" property into ItemsPropertiesUpdated.
 *
 * The test verifies:
 *   1. The state actually flipped to TRUE, otherwise the buggy path is
 *      never reached and the test would trivially pass.
 *   2. No "children-display" appears in any emitted
 *      ItemsPropertiesUpdated (updated or removed).
 */

typedef struct {
    gboolean became_true;
    gboolean saw_children_display;
} SignalSpy;

/* Standard stateful action pattern: apply the requested state. */
static void sub_state_change(GSimpleAction *action, GVariant *value, gpointer user_data)
{
    (void)user_data;
    g_simple_action_set_state(action, value);
}

static void on_sub_state_notify(GObject *object, GParamSpec *pspec, gpointer user_data)
{
    (void)pspec;
    SignalSpy *spy = user_data;
    GVariant *state = g_action_get_state(G_ACTION(object));
    if (g_variant_get_boolean(state))
        spy->became_true = TRUE;
    g_variant_unref(state);
}

static void on_items_properties_updated(GDBusConnection *conn,
                                        const char *sender,
                                        const char *object_path,
                                        const char *interface,
                                        const char *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data)
{
    (void)conn; (void)sender; (void)object_path;
    (void)interface; (void)signal_name;

    SignalSpy *spy = user_data;

    /* com.canonical.dbusmenu: (a(ia{sv}) updated, a(ias) removed) */
    GVariantIter *updated = NULL;
    GVariantIter *removed = NULL;
    g_variant_get(parameters, "(a(ia{sv})a(ias))", &updated, &removed);

    gint32 item_id;
    GVariant *props = NULL;
    g_print("%s\n", g_variant_print(parameters, TRUE));
    while (g_variant_iter_next(updated, "(i@a{sv})", &item_id, &props))
    {
        GVariant *v = g_variant_lookup_value(props, "children-display", NULL);
        if (v != NULL)
        {
            spy->saw_children_display = TRUE;
            g_variant_unref(v);
        }
        g_variant_unref(props);
    }

    g_variant_iter_free(updated);
    g_variant_iter_free(removed);
}

/* The importer synthesizes "dbusmenu.submenu-<id>" for each
 * submenu-owning item.  We have exactly one, so pick it. */
static char *find_submenu_action(GActionGroup *group)
{
    gchar **actions = g_action_group_list_actions(group);
    char *result = NULL;
    for (int i = 0; actions[i] != NULL; i++)
    {
        if (g_str_has_prefix(actions[i], "submenu-"))
        {
            result = g_strdup(actions[i]);
            break;
        }
    }
    g_strfreev(actions);
    return result;
}

G_GNUC_INTERNAL void test_func(void)
{
    GMenu *root    = g_menu_new();
    GMenu *submenu = g_menu_new();

    GMenuItem *child = g_menu_item_new("Child", "app.child");
    g_menu_append_item(submenu, child);
    g_object_unref(child);

    /* The submenu-owning item, gated by a boolean submenu-action. */
    GMenuItem *toggle = g_menu_item_new("Toggleable", NULL);
    g_menu_item_set_submenu(toggle, G_MENU_MODEL(submenu));
    g_menu_item_set_attribute(toggle, "submenu-action", "s", "app.sub-action");
    g_menu_append_item(root, toggle);
    g_object_unref(toggle);

    GSimpleActionGroup *action_group = g_simple_action_group_new();
    GSimpleAction *sub_action =
        g_simple_action_new_stateful("sub-action",
                                     G_VARIANT_TYPE_BOOLEAN,
                                     g_variant_new_boolean(FALSE));
    g_signal_connect(sub_action, "change-state",
                     G_CALLBACK(sub_state_change), NULL);

    SignalSpy spy = { FALSE, FALSE };
    g_signal_connect(sub_action, "notify::state",
                     G_CALLBACK(on_sub_state_notify), &spy);

    g_action_map_add_action(G_ACTION_MAP(action_group), G_ACTION(sub_action));

    /* Subscribe to ItemsPropertiesUpdated to catch any spurious
     * children-display property. */
    GDBusConnection *conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
    g_assert_nonnull(conn);
    guint sub_id = g_dbus_connection_signal_subscribe(conn,
                                                      NULL,
                                                      "com.canonical.dbusmenu",
                                                      "ItemsPropertiesUpdated",
                                                      NULL,
                                                      NULL,
                                                      G_DBUS_SIGNAL_FLAGS_NONE,
                                                      on_items_properties_updated,
                                                      &spy,
                                                      NULL);

    Fixture f;
    fixture_setup(&f, G_MENU_MODEL(root), action_group);

    GActionGroup *importer_group = NULL;
    g_object_get(f.importer, "action-group", &importer_group, NULL);
    g_assert_nonnull(importer_group);

    /* Activate the synthesized submenu action on the importer side,
     * simulating a user click on the submenu-owning item. */
    char *submenu_action = find_submenu_action(importer_group);
    g_assert_nonnull(submenu_action);
    g_action_group_activate_action(importer_group,
                                   submenu_action,
                                   g_variant_new_boolean(TRUE));
    test_wait();

    /* The flip to TRUE must have happened; otherwise the buggy path
     * is never reached and the assertion below would be vacuous. */
    g_assert_true(spy.became_true);

    /* And no spurious children-display was sent. */
    g_assert_false(spy.saw_children_display);

    g_free(submenu_action);
    g_dbus_connection_signal_unsubscribe(conn, sub_id);
    g_object_unref(conn);
    g_object_unref(importer_group);
    g_object_unref(sub_action);
    g_object_unref(action_group);
    fixture_teardown(&f);
    g_object_unref(submenu);
}