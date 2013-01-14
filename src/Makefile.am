AM_CPPFLAGS = -I../lib
AM_CFLAGS = $(GTK3_CFLAGS) -Wall
AM_LDFLAGS = $(GTK3_LIBS) -avoid-version -module -shared

libunitygtkmoduledir = $(GTK_MODULE_DIR)
libunitygtkmodule_LTLIBRARIES = libunitygtk3module.la

libunitygtk3module_la_SOURCES = \
	main.c

libunitygtk3module_la_LDFLAGS = \
	$(top_builddir)/lib/libgtk3menuparser.la
