AC_PREREQ([2.69])

AC_INIT([unity-gtk-module], [0.0.0])
AC_REVISION([0.0.0])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 foreign -Wall -Woverride -Wno-portability])
AM_SILENT_RULES([yes])

LT_INIT

AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG([0.26])

AC_ARG_WITH([gtk], [AS_HELP_STRING([--with-gtk=2|3], [GTK+ version [default=3]])], [], [with_gtk=3])
AC_ARG_WITH([gtk-libdir], [AS_HELP_STRING([--with-gtk-libdir=DIR], [GTK+ library directory [default=`pkg-config --variable=libdir gtk+-3.0`]])], [], [with_gtk_libdir=`pkg-config --variable=libdir gtk+-\$with_gtk.0`])
AC_ARG_WITH([gtk-module-dir], [AS_HELP_STRING([--with-gtk-module-dir=DIR], [GTK+ module directory [default=`pkg-config --variable=libdir gtk+-3.0`/gtk-3.0/modules]])], [], [with_gtk_module_dir=$with_gtk_libdir/gtk-$with_gtk.0/modules])

AM_PATH_PYTHON

GLIB_GSETTINGS

AC_SUBST([GTK_VERSION], [$with_gtk])
AC_SUBST([GTK_MODULE_DIR], [$with_gtk_module_dir])

AM_CONDITIONAL([GTK3], [test GTK$with_gtk = GTK3])

PKG_CHECK_MODULES([GTK], [gtk+-$with_gtk.0])
AC_SUBST([GTK_CFLAGS])
AC_SUBST([GTK_LIBS])

PKG_CHECK_MODULES([X11], [x11])
AC_SUBST([X11_CFLAGS])
AC_SUBST([X11_LIBS])

AC_MSG_CHECKING([for the systemd user units directory])
PKG_CHECK_VAR([SYSTEMD_USERUNITDIR], [systemd], [systemduserunitdir])
AS_IF([test "x$SYSTEMD_USERUNITDIR" = "x"], [
       SYSTEMD_USERUNITDIR='/usr/lib/systemd/user'
       AC_MSG_RESULT([systemd not found, using the default of $SYSTEMD_USERUNITDIR])
       ], [
       AC_MSG_RESULT([$SYSTEMD_USERUNITDIR])
])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$SYSTEMD_USERUNITDIR" != "x"])


m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.18], [--flavour no-tmpl])
], [
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

AC_CONFIG_FILES([Makefile
                 data/Makefile
                 data/unity-gtk2-parser.pc
                 data/unity-gtk3-parser.pc
                 docs/Makefile
                 docs/reference/Makefile
                 docs/reference/unity-gtk-module/Makefile
                 lib/Makefile
                 src/Makefile
                 tests/Makefile
                 tests/autopilot/Makefile])

AC_OUTPUT
