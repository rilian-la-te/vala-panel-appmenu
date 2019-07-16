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

#include <gtk/gtk.h>

#include "blacklist.h"
#include "consts.h"

static const char *const BLACKLIST[] = { "acroread",
	                                 "emacs",
	                                 "emacs23",
	                                 "emacs23-lucid",
	                                 "emacs24",
	                                 "emacs24-lucid",
	                                 "budgie-panel",
	                                 "mate-panel",
	                                 "mate-menu",
	                                 "vala-panel",
	                                 "wrapper-1.0",
	                                 "wrapper-2.0",
	                                 "indicator-applet",
	                                 "mate-indicator-applet",
	                                 "mate-indicator-applet-appmenu",
	                                 "mate-indicator-applet-complete",
	                                 "appmenu-mate",
	                                 NULL };

static bool is_string_in_array(const char *string, GVariant *array)
{
	GVariantIter iter;
	char *element;

	g_return_val_if_fail(array != NULL, false);
	g_return_val_if_fail(g_variant_is_of_type(array, G_VARIANT_TYPE("as")), false);

	g_variant_iter_init(&iter, array);
	while (g_variant_iter_loop(&iter, "&s", &element))
	{
		if (g_strcmp0(element, string) == 0)
			return true;
	}

	return false;
}

static bool is_listed(const char *name, const char *key)
{
	GSettings *settings       = g_settings_new(UNITY_GTK_MODULE_SCHEMA);
	g_autoptr(GVariant) array = g_settings_get_value(settings, key);
	bool listed               = is_string_in_array(name, array);
	g_clear_object(&settings);
	return listed;
}

G_GNUC_INTERNAL
bool is_blacklisted(const char *name)
{
	guint n;
	guint i;

	n = sizeof(BLACKLIST) / sizeof(const char *);

	for (i = 0; i < n; i++)
	{
		if (g_strcmp0(name, BLACKLIST[i]) == 0)
			return !is_listed(name, WHITELIST_KEY);
	}

	return is_listed(name, BLACKLIST_KEY);
}
