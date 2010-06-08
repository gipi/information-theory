#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([itt], [0.1], [gianluca.pacchiella@ktln2.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lhuffman':
AC_CHECK_LIB([huffman], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h stddef.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup strerror])

AC_CONFIG_FILES([Makefile
		 huffman/Makefile
                 jpeg/Makefile])
AC_OUTPUT
