AC_INIT([unity-gtk-module], [0])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign 1.11 -Wno-portability no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_LIBTOOL
AM_PROG_VALAC
AC_PROG_CC

# Use GSettings
GLIB_GSETTINGS

# Dependencies
PKG_CHECK_MODULES(libwnck, libwnck-3.0)
PKG_CHECK_MODULES(gtk, gtk+-3.0)

AC_ARG_WITH(gtkmoduledir, [  --with-gtk-module-dir=PATH choose GTK3 module directory, [default=LIBDIR/gtk-3.0/modules]], gtkmoduledir="$withval", gtkmoduledir=${libdir}/gtk-3.0/modules)
AC_SUBST(gtkmoduledir)

AC_CONFIG_FILES([
  data/Makefile
  src/Makefile
  demos/Makefile
  Makefile
])
AC_OUTPUT
