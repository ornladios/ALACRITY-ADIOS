AC_DEFUN([AX_LIBFASTBIT], [

dnl Enable the --with-timer=path configure argument
AC_ARG_WITH(
  [fastbit],
  [AS_HELP_STRING(
    [--with-fastbit=DIR],
    [Location of the FastBit library]
  )]dnl
)

dnl If the timer lib was specified, verify that it exists and can compile
if test "x$with_fastbit" != xno; then
    FASTBIT_CPPFLAGS="-I$with_fastbit -I$with_fastbit/include"
    FASTBIT_LDFLAGS="-L$with_fastbit -L$with_fastbit/lib"
    FASTBIT_LIBS="-lfastbit"

    saveCPPFLAGS="$CPPFLAGS"
    saveLDFLAGS="$LDFLAGS"
    CPPFLAGS="$CPPFLAGS $FASTBIT_CPPFLAGS"
    LDFLAGS="$LDFLAGS $FASTBIT_LDFLAGS"

    AC_CHECK_HEADERS(
      [ibis.h],
      [],
      [AC_MSG_FAILURE(
        [Cannot find ibis.h from the FastBit lib. Make sure it has been properly installed at the path specified ($with_fastbit).]dnl
      )]dnl
    )

    AC_CHECK_LIB(
      [fastbit],
      [],
      [AC_DEFINE(
        [HAVE_LIBFASTBIT],
        [1],
        [Define if you have libfastbit]
      )],
      [AC_MSG_FAILURE(
        [Cannot successfully link with the FastBit lib. Make sure it has been properly installed at the path specified ($with_fastbit).]dnl
      )]dnl
    )

    CPPFLAGS="$saveCPPFLAGS"
    LDFLAGS="$saveLDFLAGS"

    AC_SUBST(FASTBIT_CPPFLAGS)
    AC_SUBST(FASTBIT_LDFLAGS)
    AC_SUBST(FASTBIT_LIBS)
else
  AC_MSG_FAILURE(
    [--with-fastbit is required]dnl
  )
fi

]) dnl End of DEFUN
