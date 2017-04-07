AM_CPPFLAGS = -I$(top_srcdir)/lib
AM_CFLAGS = $(GTK_CFLAGS) $(X11_CFLAGS) -Wall
AM_LDFLAGS = $(GTK_LIBS) $(X11_LIBS) -avoid-version -module -shared

libunity_gtk_moduledir = $(GTK_MODULE_DIR)
libunity_gtk_module_LTLIBRARIES = libunity-gtk-module.la

libunity_gtk_module_la_SOURCES = main.c
libunity_gtk_module_la_LIBADD = $(top_builddir)/lib/libunity-gtk$(GTK_VERSION)-parser.la
