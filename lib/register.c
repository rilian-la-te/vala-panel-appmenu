// SPDX-License-Identifier: LGPL-3.0-or-later

#include <glib-object.h>

extern GObject* appmenu_backend_x11_get_backend (void) __attribute__((weak));
extern GObject* appmenu_backend_wayfire_get_backend (void) __attribute__((weak));
extern gboolean appmenu_backend_x11_is_applicable (void) __attribute__((weak));
extern gboolean appmenu_backend_wayfire_is_applicable (void) __attribute__((weak));

GObject* appmenu_get_applicable_backend() {
    if(appmenu_backend_x11_is_applicable != NULL && appmenu_backend_x11_is_applicable())
        return appmenu_backend_x11_get_backend();
    if(appmenu_backend_wayfire_is_applicable != NULL && appmenu_backend_wayfire_is_applicable())
        return appmenu_backend_wayfire_get_backend();
    return NULL;
}