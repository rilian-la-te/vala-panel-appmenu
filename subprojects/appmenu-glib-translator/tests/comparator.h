/* SPDX-License-Identifier: LGPL-3.0-or-later */

#pragma once

#include <gio/gio.h>

#include "debug.h"
#include "definitions.h"
#include "exporter/exporter.h"
#include "importer/importer.h"

#define TEST_BUS_NAME "org.glib-translator-test.exporter"
#define TEST_OBJECT_PATH "/exporter"

gboolean compare_models(GMenuModel *a, GMenuModel *b, GString *errors, int depth);

typedef struct
{
	GMenuModel *source;
	GSimpleActionGroup *action_group;
	DBusMenuExporter *exporter;
	DBusMenuImporter *importer;
	GMenuModel *target;
} Fixture;

void test_wait(void);
void test_wait_ms(guint ms);
void fixture_setup(Fixture *f, GMenuModel *source, GSimpleActionGroup *action_group);
void fixture_teardown(Fixture *f);
void dump_and_report(const char *tag, Fixture *f, GString *errors);
gboolean compare_and_check_actions(Fixture *f, GString *errors);
G_GNUC_INTERNAL char *extract_bare_action(GMenuModel *model, const char *label);
G_GNUC_INTERNAL char *extract_submenu_action(GMenuModel *model, const char *label);
G_GNUC_INTERNAL GMenuModel *find_item_section(GMenuModel *model, const char *label,
                                     int *out_index);
