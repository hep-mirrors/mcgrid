# AC_SEARCH_APPLGRID_OR_FASTNLO(actionIfFound, actionIfNotFound)
AC_DEFUN([AC_SEARCH_APPLGRID_OR_FASTNLO], [

AC_MSG_NOTICE([checking if APPLgrid and/or fastNLO are detectable])

# other possible values: both, applgrid, fastnlo
applgrid_or_fastnlo_enabled=none

AC_PATH_PROG(APPLGRIDCONFIG, applgrid-config, [], [$PATH])
if test -f "$APPLGRIDCONFIG"; then
  AC_DEFINE([APPLGRID_ENABLED], [1], [Define if applgrid is available.])
  applgrid_or_fastnlo_enabled=applgrid
  APPLGRID_CPPFLAGS=`$APPLGRIDCONFIG --cxxflags`
  APPLGRID_LDFLAGS=`$APPLGRIDCONFIG --ldflags`
fi
AC_SUBST(APPLGRID_CPPFLAGS)
AC_SUBST(APPLGRID_LDFLAGS)

AC_PATH_PROG(FASTNLOCONFIG, fnlo-tk-config, [], [$PATH])
if test -f "$FASTNLOCONFIG"; then
  AC_DEFINE([FASTNLO_ENABLED], [1], [Define if fastNLO is available.])
  if test $applgrid_or_fastnlo_enabled = applgrid; then
    applgrid_or_fastnlo_enabled=both
  else
    applgrid_or_fastnlo_enabled=fastnlo
  fi
  FASTNLO_CPPFLAGS=`$FASTNLOCONFIG --cppflags`
  FASTNLO_LDFLAGS=`$FASTNLOCONFIG --ldflags`
fi
AC_SUBST(FASTNLO_CPPFLAGS)
AC_SUBST(FASTNLO_LDFLAGS)

if test $applgrid_or_fastnlo_enabled = both; then
    AC_MSG_NOTICE([both APPLgrid and fastNLO are available])
elif test $applgrid_or_fastnlo_enabled = applgrid; then
    AC_MSG_WARN([only APPLgrid is available, you will not be able to fill fastNLO grids])
elif test $applgrid_or_fastnlo_enabled = fastnlo; then
    AC_MSG_WARN([only fastNLO is available, you will not be able to fill APPLgrid grids])
else
    AC_MSG_ERROR([neither APPLgrid nor fastNLO can be found, at least one must be available])
fi
$1
])