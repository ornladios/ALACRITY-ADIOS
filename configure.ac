AC_INIT([alacrity], [1.0.0], [nagiza.samatova@gmail.com])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_FILES([Makefile ridcompress/Makefile src/Makefile tools/Makefile tests/Makefile])
AC_CONFIG_SRCDIR([src/ALEncode.c])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

AC_PROG_CC
m4_ifdef([AC_PROG_CC_C99],
         [AC_PROG_CC_C99],
         [AC_MSG_WARN([AC_PROG_CC_C99 macro not supported in your version of autoconf. Attempting to use a workaround, but it may not work.]);
          CPPFLAGS="$CPPFLAGS -std=c99"])

AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_CC_C_O
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR 

#AC_CHECK_LIB([stdc++],[main],,[AC_MSG_ERROR(ALACRITY requires libstdc++)])

#m4_include([m4/ac_ridcompress.m4])

#AX_LIBRIDCOMPRESS

RIDCOMPRESS_CPPFLAGS="-I\${top_srcdir}/ridcompress"
RIDCOMPRESS_LDFLAGS="-L\${top_builddir}/ridcompress"
RIDCOMPRESS_LIBS="-lridcompress -lstdc++"
AC_SUBST(RIDCOMPRESS_CPPFLAGS)
AC_SUBST(RIDCOMPRESS_LDFLAGS)
AC_SUBST(RIDCOMPRESS_LIBS)

AC_OUTPUT
