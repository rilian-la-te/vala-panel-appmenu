// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef XCB_AUX_H
#define XCB_AUX_H

#include <gio/gio.h>
#include <stdbool.h>
#include <stdint.h>

G_BEGIN_DECLS

typedef struct XcbAux XcbAux;

XcbAux *vala_panel_xcb_aux_connect(void);
void vala_panel_xcb_aux_unref(void *connection);

char *vala_panel_xcb_aux_get_utf8_prop(void *conn, uint32_t window, const char *prop);

void vala_panel_xcb_aux_add_listener(void *conn, GObject *listener);
void vala_panel_xcb_aux_remove_listener(GObject *listener);
void vala_panel_xcb_aux_watch_root(void *conn);

uint32_t vala_panel_xcb_aux_get_active_window(void *conn);
uint32_t vala_panel_xcb_aux_get_transient_for(void *conn, uint32_t window);
uint32_t vala_panel_xcb_aux_get_parent(void *conn, uint32_t window);

char *vala_panel_xcb_aux_get_name(void *conn, uint32_t window);
int64_t vala_panel_xcb_aux_get_pid(void *conn, uint32_t window);

void vala_panel_xcb_aux_get_wm_class(void *conn, uint32_t window, char **instance, char **group);

gboolean vala_panel_xcb_aux_is_desktop_window(void *conn, uint32_t window);

GAppInfo *vala_panel_xcb_aux_match_app(void *matcher, void *conn, uint32_t window);

gboolean is_applicable(void);

G_END_DECLS

#endif /* XCB_AUX_H */