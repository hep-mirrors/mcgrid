# AC_SEARCH_RIVET(actionIfFound, actionIfNotFound)
AC_DEFUN([AC_SEARCH_RIVET], [

AC_PATH_PROG(RIVETCONFIG, rivet-config, [], [$PATH])
if test -f "$RIVETCONFIG"; then
  RIVET_CPPFLAGS=`$RIVETCONFIG --cppflags`
  RIVET_LDFLAGS=`$RIVETCONFIG --ldflags`
else
  AC_MSG_ERROR([Rivet cannot be found!])
  exit 1
fi
AC_SUBST(RIVET_CPPFLAGS)
AC_SUBST(RIVET_LDFLAGS)
$1
])

