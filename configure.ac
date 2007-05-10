#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libmodbus, 1.2, stephane.raimbault@gmail.com)
AM_INIT_AUTOMAKE(libmodbus, 1.2.0)

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.
AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR(libmodbus needs GLib 2.X))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h termio.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([inet_ntoa memset select socket])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	])
AC_OUTPUT
