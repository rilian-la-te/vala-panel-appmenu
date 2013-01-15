AM_CFLAGS = -Wall
AM_LDFLAGS = -avoid-version

lib_LTLIBRARIES = libunitygtk$(GTK_VERSION)parser.la

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

libunitygtk__GTK_VERSION_parser_la_SOURCES = $(SOURCES)
libunitygtk__GTK_VERSION_parser_la_CFLAGS = $(GTK_CFLAGS) $(AM_CFLAGS)
libunitygtk__GTK_VERSION_parser_la_LDFLAGS = $(GTK_LIBS) $(AM_LDFLAGS)

libunitygtkparser_la_includedir = $(includedir)/unity-gtk-parser

libunitygtkparser_la_include_HEADERS = \
	unity-gtk-parser.h		\
	unity-gtk-menu-shell.h		\
	unity-gtk-action-group.h
