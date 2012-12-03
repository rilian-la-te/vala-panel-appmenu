gtkmodule_PROGRAMS    = hello unity-gtk-menu-tester
gtkmodule_LTLIBRARIES = libunity-gtk-module.la

AM_CFLAGS = $(gtk_CFLAGS) -Wall
AM_LDFLAGS = -shared -module -avoid-version

libunity_gtk_module_la_SOURCES = \
	unity-gtk-menu-shell.h		\
	unity-gtk-menu-shell.c			\
	unity-gtk-menu-section.h	\
	unity-gtk-menu-section.c		\
	unity-gtk-menu-item.h		\
	unity-gtk-menu-item.c			\
	main.c

hello_SOURCES = \
	hello.c

hello_CFLAGS = `pkg-config --cflags gtk+-3.0`
hello_LDFLAGS = `pkg-config --libs gtk+-3.0`

unity_gtk_menu_tester_SOURCES = \
	unitygtkmenutester.c

unity_gtk_menu_tester_CFLAGS = `pkg-config --cflags gtk+-3.0`
unity_gtk_menu_tester_LDFLAGS = `pkg-config --libs gtk+-3.0`
