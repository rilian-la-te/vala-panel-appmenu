/*
 * Copyright (c) 2014 Jared Gonzalez
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
 * File:   com_jarego_jayatana_basic_GMainLoop.c
 * Author: Jared Gonzalez
 */
#include "com_jarego_jayatana_basic_GMainLoop.h"

#include <gio/gio.h>
#include <glib.h>
#include <pthread.h>
#include <stdio.h>

/**
 * GMainLoop para comunicacion con glib/dbus
 */
GMainLoop *com_jarego_jayatana_gmainloop = NULL;

/**
 * Hilo que mantiene en ejecucion un GMainLoop
 */
gpointer com_jarego_jayatana_gmainloop_thread(gpointer data)
{
	// ejecutar GMainLoop
	com_jarego_jayatana_gmainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(com_jarego_jayatana_gmainloop);
	return NULL;
}

/**
 * Iniciar un GMainLoop para comunicacion con glib/dbus al iniciar el programa
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GMainLoop_installGMainLoop(JNIEnv *env,
                                                                                 jclass jclass)
{
	// lanza el hilo para el GMainLoop
	g_thread_new("jayatana_gmainloop", com_jarego_jayatana_gmainloop_thread, NULL);
}

/**
 * Detener un GMainLoop para comunicacion con glib/dbus el finalizar el programa
 */
JNIEXPORT void JNICALL Java_com_jarego_jayatana_basic_GMainLoop_uninstallGMainLoop(JNIEnv *env,
                                                                                   jclass jclass)
{
	// detiene el GMainLoop
	g_main_loop_quit(com_jarego_jayatana_gmainloop);
}
