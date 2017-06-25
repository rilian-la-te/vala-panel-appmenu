#ifndef PLATFORM_H
#define PLATFORM_H

#include <gtk/gtk.h>

void gtk_widget_set_property_string(GtkWidget *widget, const gchar *name,
                                               const gchar *value);

gchar *gtk_widget_get_property_string(GtkWidget *widget, const gchar *name);
#endif // PLATFORM_H
