/*
 * Copyright © 2011 Canonical Limited
 *
 * All rights reserved.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef __ALT_GRABBER_H__
#define __ALT_GRABBER_H__

#include <gdk/gdk.h>

#define ALT_GRABBER_TYPE (alt_grabber_get_type())
#define ALT_GRABBER(inst) (G_TYPE_CHECK_INSTANCE_CAST((inst), ALT_GRABBER_TYPE, AltGrabber))
#define IS_ALT_GRABBER(inst) (G_TYPE_CHECK_INSTANCE_TYPE((inst), ALT_GRABBER_TYPE))

typedef struct _AltGrabber AltGrabber;

typedef void (*AltGrabberCallback)(AltGrabber *grabber, gunichar c, gpointer user_data);

GType alt_grabber_get_type(void) G_GNUC_CONST;
AltGrabber *alt_grabber_get_for_screen(GdkScreen *screen);
void alt_grabber_clear(AltGrabber *grabber);
void alt_grabber_add_unichar(AltGrabber *grabber, gunichar c, AltGrabberCallback callback,
                             gpointer user_data, GDestroyNotify notify);

#endif /* __ALT_GRABBER_H__ */
