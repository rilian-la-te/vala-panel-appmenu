AM_CFLAGS = -Wall
AM_LDFLAGS = -avoid-version

lib_LTLIBRARIES = libunitygtk3parser.la

libunitygtk3parser_la_SOURCES = \
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

libunitygtk3parser_la_CFLAGS = $(GTK3_CFLAGS) $(AM_CFLAGS)
libunitygtk3parser_la_LDFLAGS = $(GTK3_LIBS) $(AM_LDFLAGS)

libunity_gtk_parser_la_includedir = $(includedir)/unity-gtk-parser

libunity_gtk_parser_la_include_HEADERS = \
	unity-gtk-parser.h		\
	unity-gtk-menu-shell.h		\
	unity-gtk-action-group.h
