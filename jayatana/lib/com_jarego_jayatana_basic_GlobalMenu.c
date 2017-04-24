/*
 * Copyright (c) 2014 Jared González
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File:   com_jarego_jayatana_basic_GlobalMenu.c
 * Author: Jared González
 */
#include "com_jarego_jayatana_basic_GlobalMenu.h"

#include <jawt_md.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-glib/client.h>
#include "com_jarego_jayatana_jni.h"
#include "com_jarego_jayatana_collections.h"
#include "com_jarego_jayatana_jkey2xkey.h"

/**
 * Estructura de control de menu global
 */
typedef struct {
	jlong windowXID;
	jobject globalThat;

	gchar *windowXIDPath;
	gboolean gdBusProxyRegistered;
	guint gBusWatcher;
	GDBusProxy *dbBusProxy;
	GVariant *dbBusProxyCallSync;

	DbusmenuServer *dbusMenuServer;
	DbusmenuMenuitem *dbusMenuRoot;

	jint registerState;
} jayatana_globalmenu_window;

#define REGISTER_STATE_INITIAL 0
#define REGISTER_STATE_REFRESH 1

/**
 * Generar nueva instancia de jayatana_globalmenu_window
 */
#define jayatana_globalmenu_window_new \
		(jayatana_globalmenu_window *)malloc(sizeof(jayatana_globalmenu_window))
ListIndex *jayatana_globalmenu_windows;

/**
 * Obtener la ubicacion de la ventana
 */
gchar *jayatana_get_windowxid_path(long xid);
/**
 * Destruir todos los menus
 */
void jayatana_destroy_menuitem(gpointer data);
/**
 * Encuentra un menu basado en el identificador
 */
DbusmenuMenuitem *jayatana_find_menuid(DbusmenuMenuitem *parent, jint menuId);

/**
 * Inicializar estructuras para GlobalMenu
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_initialize
  (JNIEnv *env, jclass thatclass) {
	jayatana_globalmenu_windows = collection_list_index_new();
}
/**
 * Termina las estructuras para GlobalMenu
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_uninitialize
  (JNIEnv *env, jclass thatclass) {
	// eliminar las instancias que se hayan quedado
	int i; jayatana_globalmenu_window *globalmenu_window;
	for (i=0;i<jayatana_globalmenu_windows->size;i++) {
		globalmenu_window = (jayatana_globalmenu_window *)
				jayatana_globalmenu_windows->entries[i]->data;
		if (globalmenu_window != NULL) {
			if (globalmenu_window->gdBusProxyRegistered) {
				// liberar menus
				g_object_unref(G_OBJECT(globalmenu_window->dbusMenuRoot));
				g_object_unref(G_OBJECT(globalmenu_window->dbusMenuServer));
				// liberar bus
				g_variant_unref(globalmenu_window->dbBusProxyCallSync);
				g_object_unref(G_OBJECT(globalmenu_window->dbBusProxy));
				// liberar ruta de ventana
				free(globalmenu_window->windowXIDPath);
			}
			(*env)->DeleteGlobalRef(env, globalmenu_window->globalThat);
			g_bus_unwatch_name(globalmenu_window->gBusWatcher);
			free(globalmenu_window);
		}
	}
	collection_list_index_destory(jayatana_globalmenu_windows);
}

/**
 * Obtener la ubicacion de la ventana
 */
gchar *jayatana_get_windowxid_path(long xid) {
	gchar *xid_path;
	xid_path = (gchar *)malloc(sizeof(gchar *)*50);
	sprintf(xid_path, "/com/canonical/menu/%lx", xid);
	return xid_path;
}
/**
 * Destruir todos los menus
 */
void jayatana_destroy_menuitem(gpointer data) {
	g_object_unref(G_OBJECT(data));
}

/**
 * Encuentra un menu basado en el identificador
 */
DbusmenuMenuitem *jayatana_find_menuid(DbusmenuMenuitem *parent, jint menuId) {
	if (DBUSMENU_IS_MENUITEM(parent)) {
		if (menuId == -1) {
			return parent;
		}
		if (dbusmenu_menuitem_property_exist(parent, "jayatana-menuid") ?
				dbusmenu_menuitem_property_get_int(parent, "jayatana-menuid") == menuId : 0) {
			return parent;
		}
		GList *itemscurr;
		GList *items = dbusmenu_menuitem_get_children(parent);
		DbusmenuMenuitem *item, *itemfound;
		for(itemscurr = items; itemscurr; itemscurr = itemscurr->next) {
			item = (DbusmenuMenuitem *)itemscurr->data;
			itemfound = jayatana_find_menuid(item, menuId);
			if (itemfound != NULL)
				return itemfound;
		}
	}
	return NULL;
}

/**
 * Configurar aceleradores sobre el menu
 */
void jayatana_set_menuitem_shortcut(DbusmenuMenuitem *item, jint modifiers, jint keycode) {
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
	if ((modifiers & JK_SHIFT) == JK_SHIFT)
		g_variant_builder_add(&builder, "s", DBUSMENU_MENUITEM_SHORTCUT_SHIFT);
	if ((modifiers & JK_CTRL) == JK_CTRL)
		g_variant_builder_add(&builder, "s",
				DBUSMENU_MENUITEM_SHORTCUT_CONTROL);
	if ((modifiers & JK_ALT) == JK_ALT)
		g_variant_builder_add(&builder, "s", DBUSMENU_MENUITEM_SHORTCUT_ALT);
	const char *keystring = jkeycode_to_xkey(keycode);
	g_variant_builder_add(&builder, "s", keystring);
	GVariant *inside = g_variant_builder_end(&builder);
	g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
	g_variant_builder_add_value(&builder, inside);
	GVariant *outsidevariant = g_variant_builder_end(&builder);
	dbusmenu_menuitem_property_set_variant(item,
			DBUSMENU_MENUITEM_PROP_SHORTCUT, outsidevariant);
}

/**
 * Obtener el identificar X de una ventana AWT
 */
JNIEXPORT jlong JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_getWindowXID(
  JNIEnv *env, jclass thatclass, jobject window) {
	JAWT awt;
	JAWT_DrawingSurface* ds;
	JAWT_DrawingSurfaceInfo* dsi;
	JAWT_X11DrawingSurfaceInfo* dsi_x11;
	jint dsLock;
	Drawable drawable = -1l;
	awt.version = JAWT_VERSION_1_4;
	if (JAWT_GetAWT(env, &awt) != 0) {
		ds = awt.GetDrawingSurface(env, window);
		if (ds != NULL) {
			dsLock = ds->Lock(ds);
			if ((dsLock & JAWT_LOCK_ERROR) == 0) {
				dsi = ds->GetDrawingSurfaceInfo(ds);
				dsi_x11 = (JAWT_X11DrawingSurfaceInfo*) dsi->platformInfo;
				drawable = dsi_x11->drawable;
				ds->FreeDrawingSurfaceInfo(dsi);
				ds->Unlock(ds);
			}
		}
		awt.FreeDrawingSurface(ds);
	}
	return (long)drawable;
}

/**
 * Notificación de bus disponible para menu global
 */
void jayatana_on_registrar_available(
		GDBusConnection *connection, const gchar *name,
		const gchar *name_owner, gpointer user_data) {
	// recuperar el controlador
	jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window *)user_data;
	if (!globalmenu_window->gdBusProxyRegistered) {
		// generar menus
		globalmenu_window->windowXIDPath = jayatana_get_windowxid_path(globalmenu_window->windowXID);
		globalmenu_window->dbusMenuServer = dbusmenu_server_new(globalmenu_window->windowXIDPath);
		globalmenu_window->dbusMenuRoot = dbusmenu_menuitem_new();
		dbusmenu_server_set_root(globalmenu_window->dbusMenuServer,globalmenu_window->dbusMenuRoot);
		// registrar bus
		globalmenu_window->dbBusProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
				G_DBUS_PROXY_FLAGS_NONE, NULL,
				"com.canonical.AppMenu.Registrar",
				"/com/canonical/AppMenu/Registrar",
				"com.canonical.AppMenu.Registrar",
				NULL, NULL);
		globalmenu_window->dbBusProxyCallSync = g_dbus_proxy_call_sync(
				globalmenu_window->dbBusProxy, "RegisterWindow",
				g_variant_new("(uo)", (guint32)globalmenu_window->windowXID,
						globalmenu_window->windowXIDPath), G_DBUS_CALL_FLAGS_NONE, -1, NULL,
				NULL);
		jint register_state = globalmenu_window->registerState;
		if (globalmenu_window->registerState == REGISTER_STATE_REFRESH)
			globalmenu_window->registerState = REGISTER_STATE_INITIAL;
		// notificar a clase java la integración
		JNIEnv *env = NULL;
		(*jayatana_jvm)->AttachCurrentThread(jayatana_jvm, (void**)&env, NULL);
		jclass thatclass = (*env)->GetObjectClass(env, globalmenu_window->globalThat);
		jmethodID mid = (*env)->GetMethodID(env, thatclass, "register", "(I)V");
		(*env)->CallVoidMethod(env, globalmenu_window->globalThat, mid, register_state);
		(*env)->DeleteLocalRef(env, thatclass);
		(*jayatana_jvm)->DetachCurrentThread(jayatana_jvm);
		// marcar como instalado
		globalmenu_window->gdBusProxyRegistered = TRUE;
	}
}

/**
 * Notificación de bus no disponible para menu global
 */
void jayatana_on_registrar_unavailable(
		GDBusConnection *connection, const gchar *name,
		gpointer user_data) {
	//recuperar el controlador
	jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window *)user_data;
	if (globalmenu_window != NULL) {
		if (globalmenu_window->gdBusProxyRegistered) {
			// notificar a java el deregistro
			JNIEnv *env = NULL;
			(*jayatana_jvm)->AttachCurrentThread(jayatana_jvm, (void**)&env, NULL);
			jclass thatclass = (*env)->GetObjectClass(env, globalmenu_window->globalThat);
			jmethodID mid = (*env)->GetMethodID(env, thatclass, "unregister", "()V");
			(*env)->CallVoidMethod(env, globalmenu_window->globalThat, mid);
			(*env)->DeleteLocalRef(env, thatclass);
			(*jayatana_jvm)->DetachCurrentThread(jayatana_jvm);
			// liberar menus
			g_object_unref(G_OBJECT(globalmenu_window->dbusMenuRoot));
			g_object_unref(G_OBJECT(globalmenu_window->dbusMenuServer));
			g_variant_unref(globalmenu_window->dbBusProxyCallSync);
			g_object_unref(G_OBJECT(globalmenu_window->dbBusProxy));
			// liberar ruta de ventana
			free(globalmenu_window->windowXIDPath);
			// marcar como desinstaldo
			globalmenu_window->gdBusProxyRegistered = FALSE;
		}
	}
}

/**
 * Registrar un control de bus para menu global
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_registerWatcher
  (JNIEnv *env, jobject that, jlong windowXID) {
	//generar el controlador
	jayatana_globalmenu_window *globalmenu_window = jayatana_globalmenu_window_new;
	globalmenu_window->windowXID = windowXID;
	globalmenu_window->globalThat = (*env)->NewGlobalRef(env, that);
	globalmenu_window->gdBusProxyRegistered = FALSE;
	globalmenu_window->registerState = REGISTER_STATE_INITIAL;
	collection_list_index_add(jayatana_globalmenu_windows, windowXID, globalmenu_window);
	// iniciar bus para menu global
	globalmenu_window->gBusWatcher = g_bus_watch_name(G_BUS_TYPE_SESSION,
			"com.canonical.AppMenu.Registrar", G_BUS_NAME_WATCHER_FLAGS_NONE,
			jayatana_on_registrar_available, jayatana_on_registrar_unavailable,
			globalmenu_window, NULL);
}

/**
 * Deregistrar un control de bus para menu global
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_unregisterWatcher
  (JNIEnv *env, jobject that, jlong windowXID) {
	if (jayatana_globalmenu_windows != NULL) {
		//recuperar el controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_remove(jayatana_globalmenu_windows, windowXID);
		if (globalmenu_window != NULL) {
			if (globalmenu_window->gdBusProxyRegistered) {
				// notificar a clase java
				jclass thatclass = (*env)->GetObjectClass(env, that);
				jmethodID mid = (*env)->GetMethodID(env, thatclass, "unregister", "()V");
				(*env)->CallVoidMethod(env, that, mid);
				(*env)->DeleteLocalRef(env, thatclass);
				// liberar menus
				g_object_unref(G_OBJECT(globalmenu_window->dbusMenuRoot));
				g_object_unref(G_OBJECT(globalmenu_window->dbusMenuServer));
				g_variant_unref(globalmenu_window->dbBusProxyCallSync);
				g_object_unref(G_OBJECT(globalmenu_window->dbBusProxy));
				// liberar ruta de ventana
				free(globalmenu_window->windowXIDPath);
			}
			(*env)->DeleteGlobalRef(env, globalmenu_window->globalThat);
			g_bus_unwatch_name(globalmenu_window->gBusWatcher);
			free(globalmenu_window);
		}
	}
}

/**
 * Actualiza el bus para menus en caso de una recontrucción de menus
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_refreshWatcher
  (JNIEnv *env, jobject that, jlong windowXID) {
	if (jayatana_globalmenu_windows != NULL) {
		//recuperar el controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows, windowXID);
		if (globalmenu_window != NULL) {
			if (globalmenu_window->gdBusProxyRegistered) {
				// liberar menus
				// TODO: La liberación periva de los menus genera un crash en la aplicación
				// al momento de recontruir los menus, revisar posibles fugas de memoria.
				//g_list_free_full(dbusmenu_menuitem_take_children(globalmenu_window->dbusMenuRoot),
				//		jayatana_destroy_menuitem);
				g_object_unref(G_OBJECT(globalmenu_window->dbusMenuRoot));
				g_object_unref(G_OBJECT(globalmenu_window->dbusMenuServer));
				// liberar bus
				g_variant_unref(globalmenu_window->dbBusProxyCallSync);
				g_object_unref(G_OBJECT(globalmenu_window->dbBusProxy));
				// liberar ruta de ventana
				free(globalmenu_window->windowXIDPath);
			}

			// liberar unwatch
			g_bus_unwatch_name(globalmenu_window->gBusWatcher);
			// inicializa variables
			globalmenu_window->gdBusProxyRegistered = FALSE;
			globalmenu_window->registerState = REGISTER_STATE_REFRESH;
			// iniciar bus para menu global
			globalmenu_window->gBusWatcher = g_bus_watch_name(G_BUS_TYPE_SESSION,
					"com.canonical.AppMenu.Registrar", G_BUS_NAME_WATCHER_FLAGS_NONE,
					jayatana_on_registrar_available, jayatana_on_registrar_unavailable,
					globalmenu_window, NULL);
		}
	}
}

/**
 * Invoar generación el momento de presionar la expanción del menu
 */
void jayatana_item_about_to_show(DbusmenuMenuitem *item) {
	if (jayatana_globalmenu_windows != NULL) {
		// recuperar el controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows,
						g_variant_get_int64(dbusmenu_menuitem_property_get_variant(
								item, "jayatana-windowxid")));
		if (globalmenu_window != NULL) {
			// inicializar menu
			g_list_free_full(dbusmenu_menuitem_take_children(item), jayatana_destroy_menuitem);
			// invocar generacion de menus
			JNIEnv *env = NULL;
			(*jayatana_jvm)->AttachCurrentThread(jayatana_jvm, (void**) &env, NULL);
			jclass thatclass = (*env)->GetObjectClass(env, globalmenu_window->globalThat);
			jmethodID mid = (*env)->GetMethodID(env, thatclass, "menuAboutToShow", "(II)V");
			(*env)->CallVoidMethod(env, globalmenu_window->globalThat, mid,
					dbusmenu_menuitem_property_get_int(item, "jayatana-parent-menuid"),
					dbusmenu_menuitem_property_get_int(item, "jayatana-menuid"));
			(*env)->DeleteLocalRef(env, thatclass);
			(*jayatana_jvm)->DetachCurrentThread(jayatana_jvm);
			// marcar como atendido
			dbusmenu_menuitem_property_set_bool(item, "jayatana-need-open", FALSE);
		}
	}
}

/**
 * Invocar generación de menus desde HUD y cerrado de menus
 */
void jayatana_item_events(DbusmenuMenuitem *item, const char *event) {
	if (jayatana_globalmenu_windows != NULL) {
		// recuperar el controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows,
						g_variant_get_int64(dbusmenu_menuitem_property_get_variant(
								item, "jayatana-windowxid")));
		if (globalmenu_window != NULL) {
			if (strcmp(DBUSMENU_MENUITEM_EVENT_OPENED, event) == 0
					&& dbusmenu_menuitem_property_get_bool(item, "jayatana-need-open")) {
				// inicializar menu
				g_list_free_full(dbusmenu_menuitem_take_children(item), jayatana_destroy_menuitem);
				// invocar generacion de menus
				JNIEnv *env = NULL;
				(*jayatana_jvm)->AttachCurrentThread(jayatana_jvm, (void**) &env, NULL);
				jclass thatclass = (*env)->GetObjectClass(env, globalmenu_window->globalThat);
				jmethodID mid = (*env)->GetMethodID(env, thatclass, "menuAboutToShow", "(II)V");
				(*env)->CallVoidMethod(env, globalmenu_window->globalThat, mid,
						dbusmenu_menuitem_property_get_int(item, "jayatana-parent-menuid"),
						dbusmenu_menuitem_property_get_int(item, "jayatana-menuid"));
				(*env)->DeleteLocalRef(env, thatclass);
				(*jayatana_jvm)->DetachCurrentThread(jayatana_jvm);
				// marcar como atendido
				dbusmenu_menuitem_property_set_bool(item, "jayatana-need-open", FALSE);
			} else if (strcmp(DBUSMENU_MENUITEM_EVENT_CLOSED, event) == 0) {
				// invocar cerrado de menu
				JNIEnv *env = NULL;
				(*jayatana_jvm)->AttachCurrentThread(jayatana_jvm, (void**) &env, NULL);
				jclass thatclass = (*env)->GetObjectClass(env, globalmenu_window->globalThat);
				jmethodID mid = (*env)->GetMethodID(env, thatclass, "menuAfterClose", "(II)V");
				(*env)->CallVoidMethod(env, globalmenu_window->globalThat, mid,
						dbusmenu_menuitem_property_get_int(item, "jayatana-parent-menuid"),
						dbusmenu_menuitem_property_get_int(item, "jayatana-menuid"));
				(*env)->DeleteLocalRef(env, thatclass);
				(*jayatana_jvm)->DetachCurrentThread(jayatana_jvm);
				// marcar como pendiente de atencion
				dbusmenu_menuitem_property_set_bool(item, "jayatana-need-open", TRUE);
			}
		}
	}
}

/**
 * Invokar activación de menu
 */
void jayatana_item_activated(DbusmenuMenuitem *item, guint timestamp, gpointer user_data) {
	if (jayatana_globalmenu_windows != NULL) {
		// recuperar el controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows,
						g_variant_get_int64(dbusmenu_menuitem_property_get_variant(
								item, "jayatana-windowxid")));
		if (globalmenu_window != NULL) {
			//invocar hacia java
			JNIEnv *env = NULL;
			(*jayatana_jvm)->AttachCurrentThread(jayatana_jvm, (void**) &env, NULL);
			jclass thatclass = (*env)->GetObjectClass(env, globalmenu_window->globalThat);
			jmethodID mid = (*env)->GetMethodID(env, thatclass, "menuActivated", "(II)V");
			(*env)->CallVoidMethod(env, globalmenu_window->globalThat, mid,
					dbusmenu_menuitem_property_get_int(item, "jayatana-parent-menuid"),
					dbusmenu_menuitem_property_get_int(item, "jayatana-menuid"));
			(*env)->DeleteLocalRef(env, thatclass);
			(*jayatana_jvm)->DetachCurrentThread(jayatana_jvm);
		}
	}
}

/**
 * Agregar un nuevo menu
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_addMenu
  (JNIEnv *env, jobject that, jlong windowXID, jint menuParentID, jint menuID, jstring label, jboolean enabled, jboolean visible) {
	// recuperar controlador
	if (jayatana_globalmenu_windows != NULL) {
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows, windowXID);
		if (globalmenu_window != NULL) {
			DbusmenuMenuitem *parent = jayatana_find_menuid(globalmenu_window->dbusMenuRoot, menuParentID);
			if (parent != NULL) {
				// obtener etiqueta del menu
				const char *cclabel = (*env)->GetStringUTFChars(env, label, 0);
				// generar menu
				DbusmenuMenuitem *item = dbusmenu_menuitem_new();
				dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, cclabel);
				dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_CHILD_DISPLAY,
						DBUSMENU_MENUITEM_CHILD_DISPLAY_SUBMENU);
				dbusmenu_menuitem_property_set_int(item, "jayatana-parent-menuid", menuParentID);
				dbusmenu_menuitem_property_set_int(item, "jayatana-menuid", menuID);
				dbusmenu_menuitem_property_set_bool(item, "jayatana-need-open", TRUE);
				dbusmenu_menuitem_property_set_variant(item, "jayatana-windowxid",
						g_variant_new_int64(globalmenu_window->windowXID));
				dbusmenu_menuitem_property_set_bool(item, DBUSMENU_MENUITEM_PROP_ENABLED, (gboolean)enabled);
				dbusmenu_menuitem_property_set_bool(item, DBUSMENU_MENUITEM_PROP_VISIBLE, (gboolean)visible);

				g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_EVENT,
						G_CALLBACK(jayatana_item_events), NULL);
				g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ABOUT_TO_SHOW,
						G_CALLBACK(jayatana_item_about_to_show), NULL);

				DbusmenuMenuitem *foo = dbusmenu_menuitem_new();
				dbusmenu_menuitem_property_set(foo, DBUSMENU_MENUITEM_PROP_LABEL, "");
				dbusmenu_menuitem_child_append(item, foo);

				dbusmenu_menuitem_child_append(parent, item);
				// liberar etiqueta
				(*env)->ReleaseStringUTFChars(env, label, cclabel);
			}
		}
	}
}

/**
 * Agregar un nuevo elemento de menu
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_addMenuItem
  (JNIEnv *env, jobject that, jlong windowXID, jint menuParentID, jint menuID, jstring label, jboolean enabled, jint modifiers, jint keycode) {
	if (jayatana_globalmenu_windows != NULL) {
		// recuperar controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows, windowXID);
		if (globalmenu_window != NULL) {
			// obtener etiqueta del menu
			const char *cclabel = (*env)->GetStringUTFChars(env, label, 0);
			// generar menu
			DbusmenuMenuitem *item = dbusmenu_menuitem_new();
			dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, cclabel);
			dbusmenu_menuitem_property_set_bool(item, DBUSMENU_MENUITEM_PROP_ENABLED, (gboolean)enabled);
			dbusmenu_menuitem_property_set_int(item, "jayatana-parent-menuid", menuParentID);
			dbusmenu_menuitem_property_set_int(item, "jayatana-menuid", menuID);
			dbusmenu_menuitem_property_set_variant(item, "jayatana-windowxid",
						g_variant_new_int64(globalmenu_window->windowXID));
			if (modifiers > -1 && keycode > -1)
				jayatana_set_menuitem_shortcut(item, modifiers, keycode);
			g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
					G_CALLBACK(jayatana_item_activated), NULL);
			DbusmenuMenuitem *parent = jayatana_find_menuid(globalmenu_window->dbusMenuRoot, menuParentID);
			if (parent != NULL)
				dbusmenu_menuitem_child_append(parent, item);
			// liberar etiqueta
			(*env)->ReleaseStringUTFChars(env, label, cclabel);
		}
	}
}

/**
 * Agregar un nuevo elemento de menu tipo radio
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_addMenuItemRadio
  (JNIEnv *env, jobject that, jlong windowXID, jint menuParentID, jint menuID, jstring label, jboolean enabled, jint modifiers, jint keycode, jboolean selected) {
	if (jayatana_globalmenu_windows != NULL) {
		// recuperar controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows, windowXID);
		if (globalmenu_window != NULL) {
			// obtener etiqueta del menu
			const char *cclabel = (*env)->GetStringUTFChars(env, label, 0);
			// generar menu
			DbusmenuMenuitem *item = dbusmenu_menuitem_new();
			dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, cclabel);
			dbusmenu_menuitem_property_set_bool(item, DBUSMENU_MENUITEM_PROP_ENABLED, (gboolean)enabled);
			dbusmenu_menuitem_property_set_int(item, "jayatana-parent-menuid", menuParentID);
			dbusmenu_menuitem_property_set_int(item, "jayatana-menuid", menuID);
			dbusmenu_menuitem_property_set_variant(item, "jayatana-windowxid",
						g_variant_new_int64(globalmenu_window->windowXID));
			if (modifiers > -1 && keycode > -1)
				jayatana_set_menuitem_shortcut(item, modifiers, keycode);
			dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE,
					DBUSMENU_MENUITEM_TOGGLE_RADIO);
			dbusmenu_menuitem_property_set_int(item, DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
					selected ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
			g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
					G_CALLBACK(jayatana_item_activated), NULL);
			DbusmenuMenuitem *parent = jayatana_find_menuid(globalmenu_window->dbusMenuRoot, menuParentID);
			if (parent != NULL)
				dbusmenu_menuitem_child_append(parent, item);
			// liberar etiqueta
			(*env)->ReleaseStringUTFChars(env, label, cclabel);
		}
	}
}

/**
 * Agregar un nuevo elemento de menu tipo verificación
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_addMenuItemCheck
  (JNIEnv *env, jobject that, jlong windowXID, jint menuParentID, jint menuID, jstring label, jboolean enabled, jint modifiers, jint keycode, jboolean selected) {
	if (jayatana_globalmenu_windows != NULL) {
		// recuperar controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows, windowXID);
		if (globalmenu_window != NULL) {
			// obtener etiqueta del menu
			const char *cclabel = (*env)->GetStringUTFChars(env, label, 0);
			// generar menu
			DbusmenuMenuitem *item = dbusmenu_menuitem_new();
			dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, cclabel);
			dbusmenu_menuitem_property_set_bool(item, DBUSMENU_MENUITEM_PROP_ENABLED, (gboolean)enabled);
			dbusmenu_menuitem_property_set_int(item, "jayatana-parent-menuid", menuParentID);
			dbusmenu_menuitem_property_set_int(item, "jayatana-menuid", menuID);
			dbusmenu_menuitem_property_set_variant(item, "jayatana-windowxid",
						g_variant_new_int64(globalmenu_window->windowXID));
			if (modifiers > -1 && keycode > -1)
				jayatana_set_menuitem_shortcut(item, modifiers, keycode);
			dbusmenu_menuitem_property_set (item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE,
					DBUSMENU_MENUITEM_TOGGLE_CHECK);
			dbusmenu_menuitem_property_set_int(item, DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
					selected ? DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED : DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
			g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED,
					G_CALLBACK(jayatana_item_activated), NULL);
			DbusmenuMenuitem *parent = jayatana_find_menuid(globalmenu_window->dbusMenuRoot, menuParentID);
			if (parent != NULL)
				dbusmenu_menuitem_child_append(parent, item);
			// liberar etiqueta
			(*env)->ReleaseStringUTFChars(env, label, cclabel);
		}
	}
}

/**
 * Agregar un elemento de menu de separador
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_addSeparator
  (JNIEnv *env, jobject that, jlong windowXID, jint menuParentID) {
	if (jayatana_globalmenu_windows != NULL) {
		// recuperar controlador
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
				collection_list_index_get(jayatana_globalmenu_windows, windowXID);
		if (globalmenu_window != NULL) {
			// generar separador
			DbusmenuMenuitem *item = dbusmenu_menuitem_new();
			dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_TYPE, DBUSMENU_CLIENT_TYPES_SEPARATOR);
			DbusmenuMenuitem *parent = jayatana_find_menuid(globalmenu_window->dbusMenuRoot, menuParentID);
			if (parent != NULL)
				dbusmenu_menuitem_child_append(parent, item);
		}
	}
}

/**
 * Actualizar visibilidad, accesibilidad y etiqueta de menu
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GlobalMenu_updateMenu
  (JNIEnv *env, jobject that, jlong windowXID, jint menuID, jstring label, jboolean enabled, jboolean visible) {
	if (jayatana_globalmenu_windows != NULL) {
		jayatana_globalmenu_window *globalmenu_window = (jayatana_globalmenu_window*)
					collection_list_index_get(jayatana_globalmenu_windows, windowXID);
		if (globalmenu_window != NULL) {
			DbusmenuMenuitem *item = jayatana_find_menuid(globalmenu_window->dbusMenuRoot, menuID);
			if (item != NULL) {
				// actualizar menu
				const char *cclabel = (*env)->GetStringUTFChars(env, label, 0);
				dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, cclabel);
				dbusmenu_menuitem_property_set_bool(item, DBUSMENU_MENUITEM_PROP_ENABLED, (gboolean)enabled);
				dbusmenu_menuitem_property_set_bool(item, DBUSMENU_MENUITEM_PROP_VISIBLE, (gboolean)visible);
				// liberar etiqueta
				(*env)->ReleaseStringUTFChars(env, label, cclabel);
			}
		}
	}
}

