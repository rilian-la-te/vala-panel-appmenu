AM_CFLAGS = $(GTK_CFLAGS) -Wall
AM_LDFLAGS = $(GTK_LIBS) -avoid-version -module -shared

lib_LTLIBRARIES = libunity-gtk-parser.la

libunity_gtk_parser_la_includedir = $(includedir)/unity-gtk-module

libunity_gtk_parser_la_include_HEADERS = \
	unity-gtk-parser.h		\
	unity-gtk-menu-shell.h		\
	unity-gtk-action-group.h

libunity_gtk_parser_la_SOURCES = \
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
