#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([manybody],[0.1],[teuben@astro.umd.edu])

AC_CONFIG_SRCDIR([quick_test])

# root
manybody="`pwd`"
AC_ARG_WITH(manybody, [  --with-manybody          set manybody root abs path], with_manybody=$withval, with_kira=$MANYBODY)

echo "MANYBODY=$with_manybody"
MANYBODY=$with_manybody
AC_SUBST(MANYBODY)


# compilers
AC_PROG_CXX
AC_PROG_CC
AC_PROG_F77

AC_CONFIG_FILES([manybody_start.csh])
AC_CONFIG_FILES([manybody_start.sh])

AC_OUTPUT
