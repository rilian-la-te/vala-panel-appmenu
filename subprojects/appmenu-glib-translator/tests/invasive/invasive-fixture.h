/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* Shared scaffolding for the x-kde-title invasive tests.
 *
 * These tests exercise how the importer moves a section marker's title (its label
 * property) onto the section item it keeps: when the marker is re-served under a
 * different id, re-served without a label, replaced by a hidden marker, or dropped
 * entirely so the layout shrinks.
 *
 * A x-kde-title marker is only ever meaningful paired with type=separator: that is
 * what KDE exports, and the importer treats both as section markers anyway. A bare
 * x-kde-title would be a broken server, which these tests deliberately do not cover.
 * Every layout therefore pairs the two on the same marker.
 *
 * The first section of a model always lives: it belongs to the synthetic item the
 * importer builds on construction and no marker can own or remove it. A separator
 * only opens a new section once the current one already holds an item, so most
 * layouts put at least one real item before their first marker - except the ones
 * that deliberately test a leading (or leading-hidden) marker.
 *
 * Each scenario lives in its own C file and is built as its own executable, so a
 * failing scenario is reported on its own instead of aborting one big binary. This
 * header holds only the pieces every scenario reuses: the layout primitives, the
 * check helpers, the mock-server driver (InvasiveFixture) and invasive_scenario_run().
 * Each C file builds its own layouts and calls them from a scenario_* function handed
 * to invasive_scenario_run() in its test_func. */

#pragma once

#include <gio/gio.h>

#include "importer/importer.h"
#include "importer/item.h"
#include "importer/model.h"
#include "mock-server.h"

/* --- layout primitives ----------------------------------------------------- */

void invasive_add_string_prop(GVariantBuilder *props, const char *name, const char *value);
void invasive_add_boolean_prop(GVariantBuilder *props, const char *name, gboolean value);

/* Opens one item in the (av) child list of its parent. */
void invasive_begin_item(GVariantBuilder *items, u_int32_t id);
/* Opens the (av) child list of an item whose properties are complete. */
void invasive_begin_children(GVariantBuilder *items);
/* Closes a childless item opened by invasive_begin_item. */
void invasive_end_leaf_item(GVariantBuilder *items);
/* Closes the child list of a root item opened by invasive_begin_root. */
void invasive_end_root(GVariantBuilder *layout);

/* A titled x-kde-title marker: type=separator paired with x-kde-title and a label,
 * exactly what KDE exports for a titled section. */
void invasive_add_kde_title(GVariantBuilder *layout, u_int32_t id, const char *title);
/* A titled x-kde-title marker that is hidden. Hiding a section marker toggles it, so
 * it opens no section and its label is ignored. */
void invasive_add_hidden_kde_title(GVariantBuilder *layout, u_int32_t id, const char *title);
/* A section marker without x-kde-title: a plain separator. @label may be NULL, in which
 * case the marker carries no label at all. */
void invasive_add_separator(GVariantBuilder *layout, u_int32_t id, const char *label);
/* A regular labelled item. */
void invasive_add_label(GVariantBuilder *layout, u_int32_t id, const char *label);

/* Opens the root item whose id is the importer's parent_id. */
void invasive_begin_root(GVariantBuilder *layout);

/* --- checks on what reached the importer ----------------------------------- */

GMenuModel *invasive_section_model(GMenuModel *target, int section_index);

/* The label the importer shows for a section lives on the OUTER menu item that holds
 * the section link (the dump prints it as <item label='...'> wrapping the
 * <link name='section'>), not on the items inside. So this reads the attributes of
 * item @section_index of @target, not item 0 of the section link. */
void invasive_check_section_label(GMenuModel *target, int section_index, GString *errors, const char *who,
                             const char *expected);
/* Checks the items of one section; every item must carry an action.
 * @section is a ref owned by the caller (as returned by invasive_section_model): this
 * function consumes it and always unrefs it, so pass invasive_section_model(...) inline. */
void invasive_check_section_items(GMenuModel *section, GString *errors, const char *who, int expected_n,
                             const char *const *expected_labels);
void invasive_check_n_sections(GMenuModel *target, GString *errors, const char *who, int expected);

/* The internal section item at @section_index. Only an invasive test can reach it, and
 * it is the item the title has to be attached to. */
void invasive_check_internal_label(DBusMenuModel *model, int section_index, GString *errors, const char *who,
                              const char *expected);

/* The id the importer currently stores for the section at @section_index. */
u_int32_t invasive_section_item_id(DBusMenuModel *model, int section_index);

void invasive_dump(const char *tag, GMenuModel *target);

/* --- driving the mock server ----------------------------------------------- */

typedef struct
{
	DBusMenuMockServer *server;
	DBusMenuImporter   *importer;
	GMenuModel         *target;
	DBusMenuModel      *model;
} InvasiveFixture;

/* Serves @layout from a fresh mock server and waits until the importer parsed it.
 * @layout is floating: the menu sinks it, so we never own it either. */
void invasive_scenario_start(InvasiveFixture *s, GVariant *layout, const char *bus_name);
void invasive_scenario_stop(InvasiveFixture *s);
/* Swaps the layout the mock server answers with and lets the importer re-parse it. */
void invasive_scenario_serve(InvasiveFixture *s, GVariant *layout);

/* Runs @scenario against a fresh server/importer pair, collecting mismatches into an
 * errors string. Prints them and fails the test if any accumulated - so each
 * executable is a single self-contained test case. */
void invasive_scenario_run(void (*scenario)(GString *errors));
