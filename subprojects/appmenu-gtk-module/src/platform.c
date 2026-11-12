/*
 * appmenu-gtk-module
 * Copyright 2012 Canonical Ltd.
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Ryan Lortie <desrt@desrt.ca>
 *          William Hua <william.hua@canonical.com>
 *          Konstantin Pugin <ria.freelander@gmail.com>
 *          Lester Carballo Perez <lestcape@gmail.com>
 */

#include "platform.h"
#include "consts.h"
#include "datastructs-private.h"
#include "datastructs.h"

extern const ModuleBackend x11_platform __attribute__((weak));
extern const ModuleBackend gtk_shell_platform __attribute__((weak));

static const ModuleBackend *const backends[] = {
	&x11_platform,
    &gtk_shell_platform,
    NULL
};

static WindowData* applicable_backends_establish_window_data(GtkWindow *window, bool before) {	
	for(int i = 0; i < G_N_ELEMENTS(backends); i++) {
		const ModuleBackend* b = backends[i];
		if (!b)
			continue;
		if(b->is_applicable(window) && (!before || (b->before_realize_allowed == before))) {
			return b->establish_window_data(window);
		}
	}
	return NULL;
}

G_GNUC_INTERNAL bool gtk_module_has_backend() {
	for(int i = 0; i < G_N_ELEMENTS(backends); i++) {
		const ModuleBackend* b = backends[i];
		if (!b)
			continue;
		if(b->is_runnable()) {
			return true;
		}
	}
	return false;
}

G_GNUC_INTERNAL WindowData *backend_get_window_data(GtkWindow *window, bool before)
{
	WindowData *window_data;

	g_return_val_if_fail(GTK_IS_WINDOW(window), NULL);

	window_data = g_object_get_qdata(G_OBJECT(window), window_data_quark());

	if(!window_data) {
		window_data = applicable_backends_establish_window_data(window, before);

	    g_object_set_qdata_full(G_OBJECT(window),
                            window_data_quark(),
                            window_data,
                            window_data_free);
	}

	return window_data;
}
