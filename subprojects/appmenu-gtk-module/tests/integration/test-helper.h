/* SPDX-License-Identifier: LGPL-3.0-or-later */
#pragma once

#include "appmenu-gtk-parser.h"
#include "comparator.h"
#include "exporter.h"
#include "importer.h"

#include <gtk/gtk.h>

typedef struct
{
	GMenu *root;
	GMenu *section1;
	GMenu *section2;
	GSimpleActionGroup *actions; /* source: new, size, autosave, … */
	GMenuModel *source;

	GtkWidget *menubar;
	UnityGtkMenuShell *shell;
	UnityGtkActionGroup *unity_actions; /* bridge: app.* ⇄ unity.* */

	DBusMenuExporter *exporter;
	DBusMenuImporter *importer;
	GMenuModel *imported_model;
	GActionGroup *imported_actions;
} E2E;

extern int g_new_fires;

void   e2e_setup(E2E *e);
void   e2e_teardown(E2E *e);
void   assert_models(E2E *e, const char *tag);
char  *imported_action_for_label(GMenuModel *m, const char *label);
void   dump_e2e(const char *tag, E2E *e);
void   settle(guint ms);
void   pump_events(void);
void   load_imported(E2E *e);
gboolean radio_state_is(GActionGroup *g, const char *action, const char *expected);

#define CHECK(e, tag, cond)                                                                 \
	do                                                                                   \
	{                                                                                    \
		if (!(cond))                                                                 \
		{                                                                            \
			dump_e2e((tag), (e));                                                \
			g_assert_true(cond);                                                 \
		}                                                                            \
	} while (0)

#define DBUS_MENU_ACTION_RADIO_SELECTED "+"
#define DBUS_MENU_ACTION_RADIO_UNSELECTED "-"
