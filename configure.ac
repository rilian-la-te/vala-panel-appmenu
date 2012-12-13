AC_PREREQ([2.69])

AC_INIT([unity-gtk-module], [0.0.0])
AC_REVISION([0.0.0])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 foreign -Wall -Woverride -Wno-portability])
AM_SILENT_RULES([yes])

AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_CC

PKG_CHECK_MODULES([gtk], [gtk+-3.0])

AC_ARG_WITH([gtk-module-dir], [AS_HELP_STRING([--with-gtk-module-dir=PATH], [GTK+ module directory [default=LIBDIR/gtk-3.0/modules]])], [], with_gtk_module_dir=${libdir}/gtk-3.0/modules)
AC_SUBST([GTK_MODULE_DIR], $with_gtk_module_dir)

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile])

AC_OUTPUT
