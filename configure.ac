AC_INIT([nomad program],[0.1],[nomad@pfeifer-syscon.de])

AC_CONFIG_MACRO_DIR([m4])

dnl use -no-portablitity to stop complaining about $(shell... in res Makefile.am
dnl i dont get it, as the shell is everywhere, turtle power!
dnl alternative use flavor foreign

AM_INIT_AUTOMAKE([1.11 -Wno-portability])

AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([config.h])

AX_CHECK_ENABLE_DEBUG([info], [DEBUG], [RELEASE], [IS-RELEASE])
AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([20], [noext], [optional])

LT_INIT

AC_CANONICAL_HOST
AC_MSG_NOTICE([host_os $host_os])

PKG_CHECK_MODULES([GTKMM], [gtkmm-3.0])

PKG_CHECK_MODULES([GLIBMM], [glibmm-2.4 giomm-2.4])

PKG_CHECK_MODULES(X11, x11 > 1.6.4, [USE_X11="true"], [USE_WIN="true"])

AC_SEARCH_LIBS([HPDF_Page_BeginText], [hpdf], [USE_LIBHARU="true"])

PKG_CHECK_MODULES([RSVG], [librsvg-2.0])

PKG_CHECK_MODULES([GENERICIMG], [genericimg >= 0.2.0])

AM_CONDITIONAL([HAVE_X11], [test x$USE_X11 = xtrue])
AM_CONDITIONAL([HAVE_WIN], [test x$USE_WIN = xtrue])
AM_CONDITIONAL([HAVE_PDF], [test x$USE_LIBHARU = xtrue])

if test "x$USE_LIBHARU" = xtrue; then
    AC_DEFINE([USE_PDF], [1], [Using hpdf])
fi

AX_CHECK_SED()

if test "$host_os" = "mingw32" ; then
    AC_SUBST(EXTRA_LDFLAGS, "")   dnl disabled -mwindows while debugging
fi


AC_CONFIG_FILES([
Makefile
res/Makefile
src/Makefile])
AC_OUTPUT
