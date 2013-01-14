AM_CFLAGS = -Wall
AM_LDFLAGS = -avoid-version

libunity_gtk_parserdir = $(libdir)/unity-gtk-module

libunity_gtk_parser_LTLIBRARIES = \
	libunity-gtk2-parser.la	\
	libunity-gtk3-parser.la

SOURCES = \
	unity-gtk-menu-shell.h			\
	unity-gtk-menu-shell-private.h		\
	unity-gtk-menu-shell.c			\
	unity-gtk-menu-section-private.h	\
	unity-gtk-menu-section.c		\
	unity-gtk-menu-item-private.h		\
	unity-gtk-menu-item.c			\
	unity-gtk-action-group.h		\
	unity-gtk-action-group-private.h	\
	unity-gtk-action-group.c		\
	unity-gtk-action-private.h		\
	unity-gtk-action.c

libunity_gtk2_parser_la_SOURCES = $(SOURCES)
libunity_gtk2_parser_la_CFLAGS = $(GTK2_CFLAGS) $(AM_CFLAGS)
libunity_gtk2_parser_la_LDFLAGS = $(GTK2_LIBS) $(AM_LDFLAGS)

libunity_gtk3_parser_la_SOURCES = $(SOURCES)
libunity_gtk3_parser_la_CFLAGS = $(GTK3_CFLAGS) $(AM_CFLAGS)
libunity_gtk3_parser_la_LDFLAGS = $(GTK3_LIBS) $(AM_LDFLAGS)

libunity_gtk_parser_la_includedir = $(includedir)/unity-gtk-module

libunity_gtk_parser_la_include_HEADERS = \
	unity-gtk-parser.h		\
	unity-gtk-menu-shell.h		\
	unity-gtk-action-group.h
