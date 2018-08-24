#ifndef JAYATANAWINDOW_H
#define JAYATANAWINDOW_H

#include <gio/gio.h>

#include <jawt_md.h>
#include <libdbusmenu-glib/server.h>
#include <stdbool.h>

G_BEGIN_DECLS

/**
 * Control structure of global menu
 */
typedef struct
{
	jlong windowXID;
	jobject globalThat;

	int menu_counter;
	char *windowXIDPath;
	bool gdBusProxyRegistered;
	guint gBusWatcher;
	GDBusProxy *dbBusProxy;
	GVariant *dbBusProxyCallSync;

	DbusmenuServer *dbusMenuServer;
	DbusmenuMenuitem *dbusMenuRoot;

	jint registerState;
	GHashTable *menu_items;
} JAyatanaWindow;

JAyatanaWindow *jayatana_window_new();
JAyatanaWindow *jayatana_window_copy(JAyatanaWindow *src);
void jayatana_window_free(JAyatanaWindow *window);

void jayatana_window_remove_unparented(JAyatanaWindow *win);
DbusmenuMenuitem *jayatana_window_get_dbusmenu_item(JAyatanaWindow *win, jint id,
                                                    const char *cclabel);

G_END_DECLS

#endif // JAYATANAWINDOW_H
