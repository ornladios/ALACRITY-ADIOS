AC_DEFUN([AX_LIBRIDCOMPRESS], [

dnl Enable the --with-timer=path configure argument
AC_ARG_WITH(
  [ridcompress],
  [AS_HELP_STRING(
    [--with-ridcompress=DIR],
    [Location of the RID compression library]
  )]dnl
)

dnl If the timer lib was specified, verify that it exists and can compile
if test "x$with_ridcompress" != xno; then
    RIDCOMPRESS_CPPFLAGS="-I$with_ridcompress -I$with_ridcompress/include"
    RIDCOMPRESS_LDFLAGS="-L$with_ridcompress -L$with_ridcompress/lib"
    RIDCOMPRESS_LIBS="-lridcompress -lstdc++"

    saveCPPFLAGS="$CPPFLAGS"
    saveLDFLAGS="$LDFLAGS"
    CPPFLAGS="$CPPFLAGS $RIDCOMPRESS_CPPFLAGS"
    LDFLAGS="$LDFLAGS $RIDCOMPRESS_LDFLAGS"

    AC_CHECK_HEADERS(
      [pfordelta-c-interface.h],
      [],
      [AC_MSG_FAILURE(
        [Cannot find pfordelta-c-interface.h from the RID compression lib. Make sure it has been properly installed at the path specified ($with_ridcompress).]dnl
      )]dnl
    )

    AC_CHECK_LIB(
      [ridcompress],
      [encode_rids],
      [AC_DEFINE(
        [HAVE_LIBRIDCOMPRESS],
        [1],
        [Define if you have libridcompress]
      )],
      [AC_MSG_FAILURE(
        [Cannot successfully link with the RID compression lib. Make sure it has been properly installed at the path specified ($with_ridcompress).]dnl
      )],
      [-lstdc++]dnl
    )

    CPPFLAGS="$saveCPPFLAGS"
    LDFLAGS="$saveLDFLAGS"

    AC_SUBST(RIDCOMPRESS_CPPFLAGS)
    AC_SUBST(RIDCOMPRESS_LDFLAGS)
    AC_SUBST(RIDCOMPRESS_LIBS)
else
  AC_MSG_FAILURE(
    [--with-ridcompress is required]dnl
  )
fi

]) dnl End of DEFUN
