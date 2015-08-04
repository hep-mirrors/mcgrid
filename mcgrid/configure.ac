AC_PREREQ([2.69])
AC_INIT([MCgrid], [2.0], [mcgrid@projects.hepforge.org])
AC_CONFIG_SRCDIR([src/grid.cpp])
AC_CONFIG_HEADERS([mcgrid/config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([subdir-objects])

# Check for fillmode
AC_ARG_ENABLE([sherpafill],
    AS_HELP_STRING([--disable-sherpafill], [Disable the SHERPA fillmode]))

AS_IF([test "x$enable_sherpafill" != "xno"], [
    AC_DEFINE([FILLMODE_SHERPA],, [MCGrid SHERPA fillmode])
])

# Libtool
LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

# Checks for Rivet and grid implementations
AC_SEARCH_RIVET
AC_SEARCH_APPLGRID_OR_FASTNLO

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([pow sqrt])

AC_CONFIG_FILES([Makefile mcgrid.pc])
AC_OUTPUT
