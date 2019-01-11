/*
 * vala-panel
 * Copyright (C) 2018 Konstantin Pugin <ria.freelander@gmail.com>
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

#ifndef MATCHER_H
#define MATCHER_H

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <libwnck/libwnck.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelMatcher, vala_panel_matcher, VALA_PANEL, MATCHER, GObject)

ValaPanelMatcher *vala_panel_matcher_new();
GDesktopAppInfo *vala_panel_matcher_match_wnck_window(ValaPanelMatcher *self, WnckWindow *window);
char *vala_panel_matcher_get_gtk_application_id(ulong window);
char *vala_panel_matcher_get_x11_atom_string(ulong xid, GdkAtom atom, bool utf8);

G_END_DECLS

#endif // MATCHER_H
