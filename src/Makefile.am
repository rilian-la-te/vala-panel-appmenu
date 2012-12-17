libunity_gtk_module_LTLIBRARIES = libunity-gtk-module.la

AM_CFLAGS = $(GTK_CFLAGS) -Wall
AM_LDFLAGS = $(GTK_LIBS) -avoid-version -module -shared

libunity_gtk_moduledir = $(GTK_MODULE_DIR)

libunity_gtk_module_la_SOURCES = \
	main.c				\
	unity-gtk-menu-shell.h		\
	unity-gtk-menu-shell.c		\
	unity-gtk-menu-section.h	\
	unity-gtk-menu-section.c	\
	unity-gtk-menu-item.h		\
	unity-gtk-menu-item.c		\
	unity-gtk-action-group.h	\
	unity-gtk-action-group.c	\
	unity-gtk-action.h		\
	unity-gtk-action.c
