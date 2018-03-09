#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libmem], [0.1], [shilling.jake@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])
AC_CONFIG_SRCDIR([src/mem.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stddef.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap])

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 tests/Makefile])
AC_OUTPUT
