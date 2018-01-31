#ifndef DEBUG_H
#define DEBUG_H

#include <gio/gio.h>
G_GNUC_INTERNAL GString *g_menu_markup_print_string(GString *string, GMenuModel *model, gint indent,
                                                    gint tabstop);
G_GNUC_INTERNAL void g_menu_markup_print_to_console(GMenuModel *menu);
#endif // DEBUG_H
