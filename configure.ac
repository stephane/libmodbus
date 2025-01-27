# libmodbus package version number, (as distinct from shared library version)
# An odd micro number indicates in-progress development from Git
# An even micro number indicates a released version
#
# Making a point release:
# - increase libmodbus_version_micro to the next even number
#
# After the release:
# - increase libmodbus_version_minor to the next odd number
#
# Take care to update the libtool versioning when required (LIBMODBUS_LD_*).
# http://www.gnu.org/software/libtool/manual/html_node/Libtool-versioning.html
#
m4_define([libmodbus_version_major], [3])
m4_define([libmodbus_version_minor], [1])
m4_define([libmodbus_version_micro], [11])

m4_define([libmodbus_release_status],
    [m4_if(m4_eval(libmodbus_version_minor % 2), [1], [snapshot], [release])])

m4_define([libmodbus_version],
    [libmodbus_version_major.libmodbus_version_minor.libmodbus_version_micro])

AC_PREREQ([2.63])
AC_INIT([libmodbus],
        [libmodbus_version],
        [https://github.com/stephane/libmodbus/issues],
        [libmodbus],
        [http://libmodbus.org/])
AC_CONFIG_SRCDIR([src/modbus.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([check-news foreign 1.11 silent-rules tar-pax subdir-objects])
AC_PROG_CPP
AC_PROG_CC
AM_PROG_CC_C_O
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])
dnl This feature seems to require automake-1.13 or newer (1.11+ by other info)
dnl On very old systems can comment it away with little loss (then automake-1.10
dnl is known to suffice):
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])],
        [AC_MSG_NOTICE([Silent Rules feature not defined in this automake version, skipped])])
AM_SILENT_RULES([yes])

LIBMODBUS_VERSION_MAJOR=libmodbus_version_major
LIBMODBUS_VERSION_MINOR=libmodbus_version_minor
LIBMODBUS_VERSION_MICRO=libmodbus_version_micro
LIBMODBUS_VERSION=libmodbus_version
AC_SUBST(LIBMODBUS_VERSION_MAJOR)
AC_SUBST(LIBMODBUS_VERSION_MINOR)
AC_SUBST(LIBMODBUS_VERSION_MICRO)
AC_SUBST(LIBMODBUS_VERSION)

# ABI version
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
LIBMODBUS_LD_CURRENT=6
LIBMODBUS_LD_REVISION=0
LIBMODBUS_LD_AGE=1
LIBMODBUS_LT_VERSION_INFO=$LIBMODBUS_LD_CURRENT:$LIBMODBUS_LD_REVISION:$LIBMODBUS_LD_AGE
AC_SUBST(LIBMODBUS_LT_VERSION_INFO)

AC_CANONICAL_HOST

# OS check
os_win32="false"
os_cygwin="false"
os_qnx="false"
os_sunos="false"
case "${host_os}" in
      *mingw32*)
    os_win32="true"
  ;;
      *nto-qnx*)
    os_qnx="true"
  ;;
      *cygwin*)
    os_cygwin="true"
  ;;
      *solaris*)
    os_sunos="true"
  ;;
esac
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "true")
AM_CONDITIONAL(OS_QNX, test "$os_qnx" = "true")

m4_pattern_forbid([^LT_INIT])dnl
LT_INIT([disable-static win32-dll pic-only])

AC_CHECK_HEADERS([ \
    arpa/inet.h \
    byteswap.h \
    errno.h \
    fcntl.h \
    limits.h \
    linux/serial.h \
    netdb.h \
    netinet/in.h \
    netinet/tcp.h \
    sys/ioctl.h \
    sys/params.h \
    sys/socket.h \
    sys/time.h \
    sys/types.h \
    termios.h \
    time.h \
    unistd.h \
])

dnl On some platforms like FreeBSD and OpenIndiana (illumos) the
dnl netinet/ip.h requires netinet/in.h explicitly included first:
AC_CHECK_HEADERS([ \
    netinet/ip.h \
], [], [], [
    AC_INCLUDES_DEFAULT
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
])

# Cygwin defines IPTOS_LOWDELAY but can't handle that flag so it's necessary to
# workaround that problem and Cygwin doesn't define MSG_DONTWAIT.
AC_CHECK_DECLS([__CYGWIN__])

# Check for network function in libnetwork for Haiku
AC_SEARCH_LIBS(accept, network socket)

# Checks for library functions.
AC_CHECK_FUNCS([accept4 gai_strerror getaddrinfo gettimeofday select socket strerror strlcpy])

# Required for MinGW with GCC v4.8.1 on Win7
AC_DEFINE(WINVER, 0x0501, _)

# Required for bswap
AC_C_INLINE

# libtool
AC_PROG_CXX

# Various types
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

if test "$os_cygwin" = "false"; then
    AC_CHECK_HEADERS([windows.h], HAVE_WINDOWS_H=yes)

    # Required for getaddrinfo (TCP IP - IPv6)
    AC_CHECK_HEADERS([winsock2.h], HAVE_WINSOCK2_H=yes)
    if test "x$HAVE_WINSOCK2_H" = "xyes"; then
        LIBS="$LIBS -lws2_32"
        AC_SUBST(LIBS)
    fi

    dnl Can bring inet_ntop()/inet_pton()... or not, depending on distro
    dnl (e.g. mingw "native" with MSYS2 or cross-built from Linux); that
    dnl is further checked below:
    AC_CHECK_HEADERS([ws2tcpip.h], HAVE_WS2TCPIP_H=yes)
fi

if test "$os_sunos" = "true"; then
    LIBS="$LIBS -lnsl -lsocket"
    AC_SUBST(LIBS)
fi

# Check for RS485 support (Linux kernel version 2.6.28+)
AC_CHECK_DECLS([TIOCSRS485], [], [], [[#include <sys/ioctl.h>]])
# Check for RTS flags
AC_CHECK_DECLS([TIOCM_RTS], [], [], [[#include <sys/ioctl.h>]])

WARNING_CFLAGS="-Wall \
-Wmissing-declarations -Wmissing-prototypes \
-Wnested-externs -Wpointer-arith \
-Wsign-compare -Wchar-subscripts \
-Wstrict-prototypes -Wshadow \
-Wformat-security"

dnl FIXME: define more thoroughly if C++ code ever appears here
WARNING_CXXFLAGS="$WARNING_CFLAGS"

dnl Adapted from NUT v2.8.2 configure.ac :
myCFLAGS="$CFLAGS"
AS_IF([test "${GCC}" = "yes"],
    [CFLAGS="$myCFLAGS -Werror -Werror=implicit-function-declaration"],
    [dnl # Don't know what to complain about for unknown compilers
     dnl # FIXME: We presume here they have at least a "-Werror" option
     CFLAGS="$myCFLAGS -Werror"
    ])

AC_CACHE_CHECK([for inet_ntop() with IPv4 and IPv6 support],
    [ac_cv_func_inet_ntop],
    [AC_LANG_PUSH([C])
     AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[
#if HAVE_WINDOWS_H
# undef inline
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
# if HAVE_WINSOCK2_H
#  include <winsock2.h>
# endif
# if HAVE_WS2TCPIP_H
#  include <ws2tcpip.h>
# endif
#else
# include <arpa/inet.h>
#endif
#include <stdio.h>
]],
            [[/* const char* inet_ntop(int af, const void* src, char* dst, size_t cnt); */
char buf[128];
printf("%s", inet_ntop(AF_INET, "1.2.3.4", buf, 10));
printf("%s", inet_ntop(AF_INET6, "::1", buf, 10))
/* autoconf adds ";return 0;" */
]])],
        [ac_cv_func_inet_ntop=yes], [ac_cv_func_inet_ntop=no]
    )
    AC_LANG_POP([C])
])
AS_IF([test x"${ac_cv_func_inet_ntop}" = xyes],
    [AC_DEFINE([HAVE_INET_NTOP], 1, [defined if system has the inet_ntop() method])],
    [AC_MSG_WARN([Required C library routine inet_ntop() not found])
     AS_IF([test "${os_win32}" = "true"], [AC_MSG_WARN([Windows antivirus might block this test])])
    ]
)

AC_CACHE_CHECK([for inet_pton() with IPv4 and IPv6 support],
    [ac_cv_func_inet_pton],
    [AC_LANG_PUSH([C])
     AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[
#if HAVE_WINDOWS_H
# undef inline
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
# if HAVE_WINSOCK2_H
#  include <winsock2.h>
# endif
# if HAVE_WS2TCPIP_H
#  include <ws2tcpip.h>
# endif
#else
# include <arpa/inet.h>
#endif
#include <stdio.h>
]],
            [[/* int inet_pton(int af, const char *src, char *dst); */
struct in_addr  ipv4;
struct in6_addr ipv6;
printf("%i ", inet_pton(AF_INET, "1.2.3.4", &ipv4));
printf("%i ", inet_pton(AF_INET6, "::1", &ipv6))
/* autoconf adds ";return 0;" */
]])],
        [ac_cv_func_inet_pton=yes], [ac_cv_func_inet_pton=no]
    )
    AC_LANG_POP([C])
])
AS_IF([test x"${ac_cv_func_inet_pton}" = xyes],
    [AC_DEFINE([HAVE_INET_PTON], 1, [defined if system has the inet_pton() method])],
    [AC_MSG_WARN([Required C library routine inet_pton() not found])
     AS_IF([test "${os_win32}" = "true"], [AC_MSG_WARN([Windows antivirus might block this test])])
    ]
)
CFLAGS="$myCFLAGS"


# Check for libusb-1.0
AC_ARG_WITH([libusb],
    AS_HELP_STRING([--with-libusb],[Use libusb-1.0 for RTU USB support (default is auto)]),,
    [with_libusb=auto])

have_libusb=no
LIBUSB_LIBS=""

dnl FIXME: Pass paths and flags on command line or find via pkg-config
AS_IF([test "x$with_libusb" != "xno"], [
    LIBUSB_LIB_TRYNAME="usb-1.0"
    AC_CHECK_LIB(${LIBUSB_LIB_TRYNAME}, libusb_init, [
        LIBUSB_LIBS="-l${LIBUSB_LIB_TRYNAME}"
    ], [
        LIBUSB_LIB_TRYNAME="usb"
        AC_CHECK_LIB(${LIBUSB_LIB_TRYNAME}, libusb_init, [
            LIBUSB_LIBS="-l${LIBUSB_LIB_TRYNAME}"
        ], [
            LIBUSB_LIB_TRYNAME="none"
        ])
    ])

    AS_IF([test x"${LIBUSB_LIBS}" != x], [
        AC_CHECK_LIB(${LIBUSB_LIB_TRYNAME}, libusb_get_parent, [:], [LIBUSB_LIBS=""])
    ])

    AS_IF([test x"${LIBUSB_LIBS}" != x], [
        AC_CHECK_HEADERS([libusb-1.0/libusb.h], HAVE_LIBUSB_1_0_LIBUSB_H=yes)
        AC_CHECK_HEADERS([libusb.h], HAVE_LIBUSB_H=yes)
    ])

    AS_IF([test x"${LIBUSB_LIBS}" != x && test x"${HAVE_LIBUSB_1_0_LIBUSB_H}" = xyes -o x"${HAVE_LIBUSB_H}" = xyes], [
        AC_DEFINE(HAVE_LIBUSB, 1, [defined if libusb is available])
        have_libusb=yes

        AC_MSG_CHECKING([for libusb pollfd accessibility])
        AC_LANG_PUSH([C])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
#if HAVE_LIBUSB_1_0_LIBUSB_H
# include <libusb-1.0/libusb.h>
#else
# if HAVE_LIBUSB_H
#  include <libusb.h>
# endif
#endif
],[
libusb_context *ctx = NULL;
const struct libusb_pollfd** list = libusb_get_pollfds(ctx);
/* Do not care about actual return value in this test,
 * normally check for non-zero meaning to look in errno */
]
            )],
            [AC_DEFINE(HAVE_LIBUSB_POLLFD, 1, [defined if libusb pollfd is available])
             AC_MSG_RESULT([ok])
            ],
            [AC_MSG_RESULT([no])]
        )
    ], [
        AS_IF([test "x$with_libusb" = "xyes"], [
            AC_MSG_ERROR(["libusb-1.0 not found. Please install libusb-1.0."])
        ], [
            AC_MSG_WARN(["libusb-1.0 not found. Please install libusb-1.0 if you want to use it."])
        ])
    ])
])
AC_SUBST(LIBUSB_LIBS)
AM_CONDITIONAL(WITH_LIBUSB, [test x"${have_libusb}" = x"yes"])

# Build options
AC_ARG_ENABLE(tests,
	AS_HELP_STRING([--disable-tests],
	[Build tests (default: yes)]),,
	[enable_tests=yes])
AM_CONDITIONAL(BUILD_TESTS, [test $enable_tests != no])

AC_CONFIG_HEADERS([config.h tests/unit-test.h src/modbus-config.h])
AC_CONFIG_FILES([
        Makefile
        src/Makefile
        src/modbus-version.h
        src/win32/modbus.dll.manifest
        tests/Makefile
        libmodbus.pc
])

AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug],
    [Enable debug symbols (default is no)])],
  [enable_debug=$enableval],
  [enable_debug=no])

AS_IF([test "x$enable_debug" = "xyes"], [
  CFLAGS="-g -O0"
  CXXFLAGS="-g -O0"
], [
  CFLAGS="-O2"
  CXXFLAGS="-O2"
])

dnl NOTE: Do not pass these among C(XX)FLAGS to the configure script itself,
dnl they can break common tests unexpectedly. Variants below should work for
dnl GCC and CLANG, and other compilers that emulate them in terms of CLI API.
AC_ARG_ENABLE([Werror],
  [AS_HELP_STRING([--enable-Werror],
    [Enable compilation failure on warnings (default is no)])],
  [enable_Werror=$enableval],
  [enable_Werror=no])
AS_IF([test "x$enable_Werror" = "xyes"], [
  WARNING_CFLAGS="$WARNING_CFLAGS -Werror"
  WARNING_CXXFLAGS="$WARNING_CXXFLAGS -Werror"
])

AC_SUBST([WARNING_CFLAGS])
AC_SUBST([WARNING_CXXFLAGS])

_PKG_VER_TITLE="$PACKAGE $VERSION"
_PKG_VER_SEPARATOR="`echo "${_PKG_VER_TITLE}" | sed 's,.,=,g'`" \
&& test x"${_PKG_VER_SEPARATOR}" != x || _PKG_VER_SEPARATOR="================"
AC_OUTPUT
AC_MSG_RESULT([
        ${_PKG_VER_TITLE}
        ${_PKG_VER_SEPARATOR}

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        includedir:             ${includedir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS} ${WARNING_CFLAGS}
        ldflags:                ${LDFLAGS}

        build shared lib:       ${enable_shared}
        build static lib:       ${enable_static}

        tests:                  ${enable_tests}
        RTU USB:                ${have_libusb}
])
