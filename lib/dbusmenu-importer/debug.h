#ifndef DEBUG_H
#define DEBUG_H

#include <gio/gio.h>
GString *g_menu_markup_print_string(GString *string, GMenuModel *model, gint indent, gint tabstop);
#endif // DEBUG_H
