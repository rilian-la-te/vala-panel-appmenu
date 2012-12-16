AC_PREREQ([2.69])

AC_INIT([unity-gtk-module], [0.0.0])
AC_REVISION([0.0.0])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 foreign -Wall -Woverride -Wno-portability])
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG([0.26])

PKG_CHECK_MODULES([GTK], [gtk+-3.0])
AC_SUBST([GTK_CFLAGS])
AC_SUBST([GTK_LIBS])

AC_ARG_WITH([gtk-libdir], [AS_HELP_STRING([--with-gtk-libdir=DIR], [GTK+ library directory [default=`pkg-config --variable=libdir gtk+-3.0`]])], [], [with_gtk_libdir=`pkg-config --variable=libdir gtk+-3.0`])
AC_ARG_WITH([gtk-module-dir], [AS_HELP_STRING([--with-gtk-module-dir=DIR], [GTK+ module directory [default=$with_gtk_libdir/gtk-3.0/modules]])], [], [with_gtk_module_dir=$with_gtk_libdir/gtk-3.0/modules])
AC_SUBST([GTK_MODULE_DIR], [$with_gtk_module_dir])

AC_ARG_WITH([x11-sysconfdir], [AS_HELP_STRING([--with-x11-sysconfdir=DIR], [X11 configuration directory [default=$sysconfdir]])], [], [with_x11_sysconfdir=$sysconfdir])
AC_ARG_WITH([x-session-dir], [AS_HELP_STRING([--with-x-session-dir=DIR], [Xsession.d directory [default=$with_x11_sysconfdir/X11/Xsession.d]])], [], [with_x_session_dir=$with_x11_sysconfdir/X11/Xsession.d])
AC_SUBST([X_SESSION_DIR], [$with_x_session_dir])

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile])

AC_OUTPUT
