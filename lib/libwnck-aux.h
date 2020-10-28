/*
 * vala-panel
 * Copyright (C) 2020 Konstantin Pugin <ria.freelander@gmail.com>
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
 */

#include "matcher.h"
#include <libwnck/libwnck.h>

#ifndef WNCK_AUX_INCLUDED
#define WNCK_AUX_INCLUDED

G_BEGIN_DECLS

char *libwnck_aux_get_utf8_prop(ulong window, const char *prop);
GDesktopAppInfo *libwnck_aux_match_wnck_window(ValaPanelMatcher *self, WnckWindow *window);

G_END_DECLS

#endif
