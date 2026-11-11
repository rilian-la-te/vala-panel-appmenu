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
void fixture_setup(Fixture *f, GMenuModel *source, GSimpleActionGroup *action_group);
void fixture_teardown(Fixture *f);
void dump_and_report(const char *tag, Fixture *f, GString *errors);
gboolean compare_and_check_actions(Fixture *f, GString *errors);