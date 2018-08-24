
#include "jayatana-window.h"

JAyatanaWindow *jayatana_window_new()
{
	JAyatanaWindow *ret = (JAyatanaWindow *)g_malloc0(sizeof(JAyatanaWindow));
	ret->menu_items   = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
	ret->menu_counter = 1;
	return ret;
}

JAyatanaWindow *jayatana_window_copy(JAyatanaWindow *src)
{
	JAyatanaWindow *ret = (JAyatanaWindow *)g_malloc0(sizeof(JAyatanaWindow));
	ret->windowXID      = src->windowXID;
	ret->globalThat     = src->globalThat;
	ret->menu_counter   = src->menu_counter;

	ret->windowXIDPath        = g_strdup(src->windowXIDPath);
	ret->gdBusProxyRegistered = src->gdBusProxyRegistered;
	ret->gBusWatcher          = src->gBusWatcher;
	ret->dbBusProxy           = G_DBUS_PROXY(g_object_ref(src->dbBusProxy));
	ret->dbBusProxyCallSync   = g_variant_ref(src->dbBusProxyCallSync);

	ret->dbusMenuServer = DBUSMENU_SERVER(g_object_ref(src->dbusMenuServer));
	ret->dbusMenuRoot   = DBUSMENU_MENUITEM(g_object_ref(src->dbusMenuRoot));

	ret->registerState = src->registerState;
	ret->menu_items    = g_hash_table_ref(src->menu_items);
	return ret;
}

void jayatana_window_free(JAyatanaWindow *window)
{
	if (window->gdBusProxyRegistered)
	{
		g_clear_pointer(&window->windowXIDPath, g_free);
		g_clear_object(&window->dbBusProxy);
		g_clear_pointer(&window->dbBusProxyCallSync, g_variant_unref);
		g_clear_object(&window->dbusMenuServer);
		g_clear_object(&window->dbusMenuRoot);
	}
	g_clear_pointer(&window->menu_items, g_hash_table_unref);
	g_clear_pointer(&window, g_free);
}

G_DEFINE_BOXED_TYPE(JAyatanaWindow, jayatana_window, (GBoxedCopyFunc)jayatana_window_copy,
                    (GBoxedFreeFunc)jayatana_window_free)

extern void jayatana_item_activated(DbusmenuMenuitem *item, guint timestamp, gpointer user_data);
extern void jayatana_item_events(DbusmenuMenuitem *item, const char *event);
extern void jayatana_item_about_to_show(DbusmenuMenuitem *item);

DbusmenuMenuitem *jayatana_window_get_dbusmenu_item(JAyatanaWindow *win, jint id,
                                                    const char *cclabel)
{
	DbusmenuMenuitem *it;
	char *checksum = g_compute_checksum_for_data(G_CHECKSUM_SHA512,
	                                             (const unsigned char *)cclabel,
	                                             strlen(cclabel));
	it             = DBUSMENU_MENUITEM(g_hash_table_lookup(win->menu_items, checksum));
	if (it == NULL)
	{
		it = dbusmenu_menuitem_new_with_id(id > 0 ? id : win->menu_counter++);
		g_hash_table_insert(win->menu_items, checksum, it);
	}
	else
	{
		g_signal_handlers_disconnect_by_func(it, (gpointer)jayatana_item_activated, NULL);
		g_signal_handlers_disconnect_by_func(it, (gpointer)jayatana_item_events, NULL);
		g_signal_handlers_disconnect_by_func(it,
		                                     (gpointer)jayatana_item_about_to_show,
		                                     NULL);
	}
	return it;
}

void jayatana_window_remove_unparented(JAyatanaWindow *win)
{
	GHashTableIter iter;
	char *key;
	DbusmenuMenuitem *value;
	g_hash_table_iter_init(&iter, win->menu_items);
	while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&value))
	{
		if (dbusmenu_menuitem_get_parent(value) == NULL)
			g_hash_table_iter_remove(&iter);
	}
}
