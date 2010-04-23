# libmodbus package version number, (as distinct from shared library version)
# An odd micro number indicates in-progress development from Git
# An even micro number indicates a released version
#
# Making a point release:
# - increase mb_version_micro to the next even number
#
# After the release:
# - increase mb_version_minor to the next odd number
#
# Take care to update the libtool versioning when required (MB_LD_*).
# http://www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html
#
m4_define([mb_version_major], [2])
m4_define([mb_version_minor], [1])
m4_define([mb_version_micro], [1])

m4_define([mb_release_status],
            [m4_if(m4_eval(mb_version_minor % 2), [1], [snapshot],
               [release])])

m4_define([mb_version], [mb_version_major.mb_version_minor.mb_version_micro])

AC_PREREQ(2.63)
AC_INIT([libmodbus],[mb_version],[stephane.raimbault@gmail.com])
AC_CONFIG_SRCDIR([modbus/modbus.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
# enable nice build output on automake1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

MB_VERSION_MAJOR=mb_version_major
MB_VERSION_MINOR=mb_version_minor
MB_VERSION_MICRO=mb_version_micro
MB_VERSION=mb_version
AC_SUBST(MB_VERSION_MAJOR)
AC_SUBST(MB_VERSION_MINOR)
AC_SUBST(MB_VERSION_MICRO)
AC_SUBST(MB_VERSION)

# ABI version
MB_LD_CURRENT=3
MB_LD_REVISION=0
MB_LD_AGE=0
MB_LT_LDFLAGS="-version-info $MB_LD_CURRENT:$MB_LD_REVISION:$MB_LD_AGE"
AC_SUBST(MB_LT_LDFLAGS)

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
        modbus/version.h
        tests/Makefile
        libmodbus.pc
])
AC_OUTPUT
