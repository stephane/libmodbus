#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
AC_INIT([libmodbus],[2.1.0],[stephane.raimbault@gmail.com])
AC_CONFIG_SRCDIR([modbus/modbus.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign])
# enable nice build output on automake1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
LT_INIT([disable-static])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([termios.h sys/time.h unistd.h errno.h limits.h fcntl.h sys/types.h sys/socket.h sys/ioctl.h netinet/in.h netinet/ip.h netinet/tcp.h arpa/inet.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
# AC_TYPE_UINT*_T: not supported by autoconf-2.59 of CentOS 5.3
# AC_TYPE_UINT16_T
# AC_TYPE_UINT32_T
# AC_TYPE_UINT8_T

# Cygwin defines IPTOS_LOWDELAY but can't handle that flag so it's necessary to
# workaround that problem and Cygwin doesn't define MSG_DONTWAIT.
AC_CHECK_DECLS([__CYGWIN__])

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([gettimeofday inet_ntoa memset select socket strerror])

AC_CONFIG_FILES([
        Makefile
        modbus/Makefile
        tests/Makefile
        modbus.pc
])
AC_OUTPUT
