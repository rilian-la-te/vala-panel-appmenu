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
	                                 "indicator-applet",
	                                 "mate-indicator-applet",
	                                 "mate-indicator-applet-appmenu",
	                                 "mate-indicator-applet-complete",
	                                 NULL };

static gboolean is_string_in_array(const gchar *string, GVariant *array)
{
	GVariantIter iter;
	const gchar *element;

	g_return_val_if_fail(array != NULL, FALSE);
	g_return_val_if_fail(g_variant_is_of_type(array, G_VARIANT_TYPE("as")), FALSE);

	g_variant_iter_init(&iter, array);
	while (g_variant_iter_next(&iter, "&s", &element))
	{
		if (g_strcmp0(element, string) == 0)
			return TRUE;
	}

	return FALSE;
}

static gboolean is_listed(const gchar *name, const gchar *key)
{
	GSettings *settings;
	GVariant *array;
	gboolean listed;

	settings = g_settings_new(UNITY_GTK_MODULE_SCHEMA);
	array    = g_settings_get_value(settings, key);
	listed   = is_string_in_array(name, array);

	g_variant_unref(array);
	g_object_unref(settings);

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
