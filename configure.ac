# Process this file with autoconf to produce a configure script.
AC_INIT(Affine, 0.2, bclg@iup.edu)
AC_CONFIG_SRCDIR([libaffine/affine.cpp])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_FILES([include/replace.h include/affine/config.h Makefile libaffine/Makefile tests/Makefile examples/Makefile examples/implicit-affine/Makefile])

# Checks for programs.
AC_PROG_CXX([g++-3.5 g++-3.4 g++-3.3 g++-3.2 g++-3.1 g++-3.0])
#AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AC_LANG([C++])

#Defaults
USE_ROUNDING=no
LIST_TYPE="palist"
REAL_TYPE="double"
if which git &>/dev/null && test -d "$srcdir"/.git ; then
  REVISION='".'`git rev-list --count HEAD`'"'
else
  REVISION='""'
fi
DATE='"'`date +%Y%m%d`'"'

AH_TEMPLATE([ARCH], [Define to the host architecture.])
AH_TEMPLATE([LIST_TYPE], [Define to the desired type of list to use.])
AH_TEMPLATE([REVISION], [Define to the source code revision.])
AH_TEMPLATE([DATE], [Define to the build date.])
AH_TEMPLATE([USE_SINCOS], [Define to 1 if a sincos for Real is available.])
AH_TEMPLATE([USE_COPYSIGN], [Define to 1 if a copysign for Real is available.])
AH_TOP([
@%:@define ARCH_I386  0
@%:@define ARCH_OTHER 1
@%:@define LIST_PALIST  0
@%:@define LIST_STDLIST 1])
AH_BOTTOM([@%:@include <replace.h>])

AC_CANONICAL_HOST
case $host_cpu in
  i?86) AC_SUBST([FLOAT_STORE], [])
        AC_DEFINE([ARCH], [ARCH_I386]) ;;
  *) AC_SUBST([FLOAT_STORE], [-ffloat-store])
     AC_DEFINE([ARCH], [ARCH_OTHER]) ;;
esac

case $host_cpu in
  i?86|ia64)
# see http://bugs.debian.org/153022
  AC_ARG_ENABLE([rounding],
    [AC_HELP_STRING([--enable-rounding],
      [enable floating-point rounding control [default=yes]])],
    [case "${enableval}" in
       yes) AC_CHECK_HEADERS([fenv.h],
              [AC_CHECK_FUNCS([fesetround],
                [AC_SUBST([USE_ROUNDING_VAL], ['@%:@define USE_ROUNDING 1'])],
                [AC_MSG_ERROR(
                  [Rounding control selected but fesetround not found])])],
              [AC_MSG_ERROR([Rounding control selected but fenv.h not found])])
            ;;
       no)  ;;
       *) AC_MSG_ERROR([Illegal value for --enable-rounding]) ;;
     esac],
    [if test x"$USE_ROUNDING" = xyes; then
       AC_CHECK_HEADERS([fenv.h],
         [AC_CHECK_FUNCS([fesetround],
           [AC_SUBST([USE_ROUNDING_VAL], ['@%:@define USE_ROUNDING 1'])])])
     fi])
    ;;
esac

AC_ARG_WITH([real-type],
  [AC_HELP_STRING([--with-real-type=TYPE],
    [type of Real @<:@default=double@:>@])],
  [REAL_TYPE=${withval}])
case "${REAL_TYPE}" in
  float) AC_SUBST([REAL_TYPE], [REAL_TYPE_FLOAT])
         REAL_SUFFIX=f
         AC_CHECK_FUNC([copysignf], [AC_DEFINE([USE_COPYSIGN])])
         AC_CHECK_FUNC([sincosf], [AC_DEFINE([USE_SINCOS])]) ;;
  double) AC_SUBST([REAL_TYPE], [REAL_TYPE_DOUBLE])
         REAL_SUFFIX=
         AC_CHECK_FUNC([copysign], [AC_DEFINE([USE_COPYSIGN])])
         AC_CHECK_FUNC([sincos], [AC_DEFINE([USE_SINCOS])]) ;;
  "long double") AC_SUBST([REAL_TYPE], [REAL_TYPE_L_DOUBLE])
         REAL_SUFFIX=l
         AC_CHECK_FUNC([copysignl], [AC_DEFINE([USE_COPYSIGN])])
         AC_CHECK_FUNC([sincosl], [AC_DEFINE([USE_SINCOS])]) ;;
  *) AC_MSG_ERROR([Unknown floating point type: ${REAL_TYPE}]) ;;
esac
AC_SUBST([REAL_SUFFIX])

AC_ARG_WITH([devlist],
  [AC_HELP_STRING([--with-devlist={palist[[[,]]] stdlist}],
    [(Advanced) which deviation list implementation to use @<:@default=palist@:>@])],
  [LIST_TYPE=${withval}])
case ${LIST_TYPE} in
  palist) AC_DEFINE([LIST_TYPE], [LIST_PALIST])
    AC_SUBST([LIST_OBJ], [palist.o]) ;;
  stdlist) AC_DEFINE([LIST_TYPE], [LIST_STDLIST]) ;;
  *) AC_MSG_ERROR([Unknown list type: ${LIST_TYPE}]) ;;
esac

AC_DEFINE_UNQUOTED([REVISION], [${REVISION}])
AC_DEFINE_UNQUOTED([DATE], [${DATE}])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-laffine':
#AC_CHECK_LIB([affine], [main])

# Checks for header files.
AC_CHECK_HEADERS([unistd.h fenv.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_HEADER_STDC
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([dup2 floor pow sqrt strstr fegetenv fesetenv])

AC_OUTPUT
