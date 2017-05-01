/*
 * Copyright © 2011 Canonical Limited
 *
 * All rights reserved.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef __ALT_MONITOR_H__
#define __ALT_MONITOR_H__

#include <gdk/gdk.h>

#define ALT_MONITOR_TYPE (alt_monitor_get_type())
#define ALT_MONITOR(inst) (G_TYPE_CHECK_INSTANCE_CAST((inst), ALT_MONITOR_TYPE, AltMonitor))
#define IS_ALT_MONITOR(inst) (G_TYPE_CHECK_INSTANCE_TYPE((inst), ALT_MONITOR_TYPE))

typedef struct _AltMonitor AltMonitor;

GType alt_monitor_get_type(void) G_GNUC_CONST;
AltMonitor *alt_monitor_get_for_display(GdkDisplay *display);
gboolean alt_monitor_get_alt_pressed(AltMonitor *monitor);

#endif /* __ALT_MONITOR_H__ */
