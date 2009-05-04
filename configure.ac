#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libmodbus, 2.0.3, stephane.raimbault@gmail.com)
AC_CONFIG_SRCDIR([src/modbus.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AM_DISABLE_STATIC

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([termios.h sys/time.h unistd.h errno.h limits.h fcntl.h sys/types.h sys/socket.h sys/ioctl.h netinet/in.h netinet/ip.h netinet/tcp.h arpa/inet.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_CHECK_DECLS([IPTOS_LOWDELAY],
               [have_iptos_lowdelay=yes], [have_iptos_lowdelay=no],
               [#include <netinet/ip.h>])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([gettimeofday inet_ntoa memset select socket strerror])

AC_OUTPUT([
        Makefile
        src/Makefile
        tests/Makefile
        modbus.pc
])
