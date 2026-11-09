/* SPDX-License-Identifier: LGPL-3.0-or-later */

#pragma once

#include <gio/gio.h>

#include "debug.h"
#include "definitions.h"

#define TEST_BUS_NAME "org.glib-translator-test.exporter"
#define TEST_OBJECT_PATH "/exporter"

gboolean compare_models(GMenuModel *a, GMenuModel *b, GString *errors, int depth);