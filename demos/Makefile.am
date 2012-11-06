noinst_PROGRAMS = client black

AM_CFLAGS = $(libwnck_CFLAGS) $(gtk_CFLAGS) $(gtkapp_CFLAGS) -Wall

client_LDADD = $(libwnck_LIBS) $(gtk_LIBS) $(gtkapp_LIBS) -lX11
client_SOURCES = \
	altgrabber.h		\
	altgrabber.c		\
	altmonitor.h		\
	altmonitor.c		\
	menusource.h		\
	menusource.c		\
	client.c

black_CFLAGS = $(gtk_CFLAGS)
black_VALAFLAGS = --pkg gtk+-3.0
black_LDADD = $(gtk_LIBS)
black_SOURCES = black.vala
