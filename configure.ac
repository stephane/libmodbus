#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libmodbus, 2.0.3, stephane.raimbault@gmail.com)
AC_CONFIG_SRCDIR([modbus/modbus.c])
AM_INIT_AUTOMAKE
AM_DISABLE_STATIC

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([termios.h sys/time.h unistd.h errno.h limits.h fcntl.h sys/types.h sys/socket.h sys/ioctl.h netinet/in.h netinet/ip.h netinet/tcp.h arpa/inet.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([inet_ntoa memset select socket])

AC_OUTPUT([
        Makefile
        modbus/Makefile
        tests/Makefile
        modbus.pc
])
