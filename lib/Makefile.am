AM_CFLAGS = -Wall
AM_LDFLAGS = -avoid-version

lib_LTLIBRARIES = libgtk3menuparser.la

libgtk3menuparser_la_SOURCES = \
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

libgtk3menuparser_la_CFLAGS = $(GTK3_CFLAGS) $(AM_CFLAGS)
libgtk3menuparser_la_LDFLAGS = $(GTK3_LIBS) $(AM_LDFLAGS)

libgtkmenuparser_la_includedir = $(includedir)/gtk-menu-parser

libgtkmenuparser_la_include_HEADERS = \
	unity-gtk-parser.h		\
	unity-gtk-menu-shell.h		\
	unity-gtk-action-group.h
