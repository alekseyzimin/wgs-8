AC_INIT([wgs], [2.2.4], [gmarcais@umd.edu])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
# AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

# Set the site name. By default is LOCAL
AC_ARG_VAR([SITE_NAME], [Site name. One of LOCAL, JCVI, BRI. Defaults to LOCAL])
AC_MSG_CHECKING([for SITE_NAME])
AS_IF([test x$SITE_NAME = x], [SITE_NAME=LOCAL],
      [test x$SITE_NAME != xLOCAL -a x$SITE_NAME != xJCVI -a x$SITE_NAME != BRI],
      [AC_MSG_ERROR([Invalid SITE_NAME "$SITE_NAME"])])
AC_MSG_RESULT([$SITE_NAME])
AM_CONDITIONAL([SITE_LOCAL], [test x$SITE_NAME = xLOCAL])
AM_CONDITIONAL([SITE_JCVI], [test x$SITE_NAME = xJCVI])
AM_CONDITIONAL([SITE_BRI], [test x$SITE_NAME = xBRI])

# Set LOCAL_NAME, the type of the machine
AC_MSG_CHECKING([for machine and os type LOCAL_NAME])
OSTYPE=`uname`
OSVERSION=`uname -r`
MACHINETYPE=`uname -m`
AS_IF([test "x$MACHINETYPE" = xx86_64], [MACHINETYPE=amd64],
      [test "x$MACHINETYPE" = "xPower Macintosh"], [MACHINETYPE=ppc])
AS_IF([test "x$OSTYPE" = xSunOS],
      [MACHINETYPE=`uname -p`]
      AS_IF([test "x$MACHINETYPE" = "xsparc"],
            [AS_IF([test x`/usr/bin/isainfo -b` = x64], [MACHINETYPE=sparc64], [MACHINETYPE=sparc32])]))
AS_IF([test "x$SITENAME" = xBRI], [LOCAL_NAME=$OSTYPE-$OSVERSION-$MACHINETYPE],
      [LOCAL_NAME=$OSTYPE-$MACHINETYPE])
AC_MSG_RESULT([$LOCAL_NAME])
AC_SUBST([LOCAL_NAME])

AC_OUTPUT([Makefile])
