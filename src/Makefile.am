gtkmodule_LTLIBRARIES = libunity-gtk-module.la
gtkmodule_PROGRAMS    = unity-gtk-menu-tester test-radio hello

AM_CFLAGS = $(gtk_CFLAGS) -Wall
AM_LDFLAGS = -shared -module -avoid-version

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

unity_gtk_menu_tester_SOURCES = \
	unity-gtk-menu-tester.c

unity_gtk_menu_tester_CFLAGS = `pkg-config --cflags gtk+-3.0`
unity_gtk_menu_tester_LDFLAGS = `pkg-config --libs gtk+-3.0`

test_radio_SOURCES = \
	test-radio.c

test_radio_CFLAGS = `pkg-config --cflags gtk+-3.0`
test_radio_LDFLAGS = `pkg-config --libs gtk+-3.0`

hello_SOURCES = \
	hello.c

hello_CFLAGS = `pkg-config --cflags gtk+-3.0`
hello_LDFLAGS = `pkg-config --libs gtk+-3.0`
