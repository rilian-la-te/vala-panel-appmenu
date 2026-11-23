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

#include <gio/gio.h>

#include "blacklist.h"
#include "consts.h"
#include "settings.h"

#include <stdlib.h>
#include <string.h>

static const char *const BLACKLIST[] = {
	"appmenu-mate",
	"budgie-panel",
	"indicator-applet",
	"mate-indicator-applet",
	"mate-indicator-applet-appmenu",
	"mate-indicator-applet-complete",
	"mate-menu",
	"mate-panel",
	"vala-panel",
	"wrapper-1.0",
	"wrapper-2.0",
	NULL
};

G_GNUC_INTERNAL
bool is_blacklisted(const char *name) {

	if (!name)
		return false;

	bool hardcoded_blacklist = g_strv_contains(BLACKLIST, name);

	GKeyFile *settings = appmenu_settings_get();
	GError *error      = NULL;
	g_auto(GStrv) array =
	    g_key_file_get_string_list(settings, APPMENU_CORE_CONFIG, BLACKLIST_KEY, NULL, &error);
	if (error)
	{
		g_clear_error(&error);
		return hardcoded_blacklist;
	}

	return hardcoded_blacklist || g_strv_contains((const char *const *)array, name);
}
