# Experimental Meson build, provided alongside the autotools one.
#
# Keep the version and ABI numbers below in sync with configure.ac.
project('libmodbus', 'c',
  version : '3.2.0',
  license : 'LGPL-2.1-or-later',
  meson_version : '>= 0.56.0',
  default_options : [
    'buildtype=debugoptimized',
    'default_library=shared',
    'warning_level=1',
  ])

libmodbus_version = meson.project_version()
ver_arr = libmodbus_version.split('.')

ver_conf = configuration_data()
ver_conf.set('LIBMODBUS_VERSION', libmodbus_version)
ver_conf.set('LIBMODBUS_VERSION_MAJOR', ver_arr[0])
ver_conf.set('LIBMODBUS_VERSION_MINOR', ver_arr[1])
ver_conf.set('LIBMODBUS_VERSION_MICRO', ver_arr[2])

# ABI version (libtool -version-info current:revision:age)
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
libmodbus_ld_current = 6
libmodbus_ld_revision = 0
libmodbus_ld_age = 1

cc = meson.get_compiler('c')
host_system = host_machine.system()

cdata = configuration_data()

# Equivalent of AC_USE_SYSTEM_EXTENSIONS (needed for accept4 among others)
cdata.set('_GNU_SOURCE', 1)
cdata.set('_ALL_SOURCE', 1)
cdata.set('_POSIX_PTHREAD_SEMANTICS', 1)
cdata.set('_TANDEM_SOURCE', 1)
cdata.set('__EXTENSIONS__', 1)

check_headers = [
  'arpa/inet.h',
  'byteswap.h',
  'errno.h',
  'fcntl.h',
  'inttypes.h',
  'limits.h',
  'linux/serial.h',
  'netdb.h',
  'netinet/in.h',
  'netinet/ip.h',
  'netinet/tcp.h',
  'stdint.h',
  'sys/ioctl.h',
  'sys/params.h',
  'sys/socket.h',
  'sys/time.h',
  'sys/types.h',
  'termios.h',
  'time.h',
  'unistd.h',
]

foreach h : check_headers
  if cc.has_header(h)
    cdata.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

# Cygwin defines IPTOS_LOWDELAY but can't handle that flag so it's necessary to
# workaround that problem and Cygwin doesn't define MSG_DONTWAIT.
cdata.set10('HAVE_DECL___CYGWIN__', cc.has_header_symbol('stddef.h', '__CYGWIN__'))

# System libraries required for networking
libmodbus_deps = []
if host_system == 'windows'
  # Required for getaddrinfo (TCP IP - IPv6)
  libmodbus_deps += cc.find_library('ws2_32')
  # Required for MinGW with GCC v4.8.1 on Win7
  cdata.set('WINVER', '0x0501')
elif host_system == 'sunos'
  libmodbus_deps += [cc.find_library('nsl'), cc.find_library('socket')]
elif not cc.has_function('accept', prefix : '#include <sys/socket.h>')
  # libnetwork for Haiku, libsocket for QNX
  foreach l : ['network', 'socket']
    dep = cc.find_library(l, required : false)
    if dep.found() and cc.has_function('accept', dependencies : dep)
      libmodbus_deps += dep
      break
    endif
  endforeach
endif

if host_system == 'windows'
  cdata.set('HAVE_WINSOCK2_H', 1)
  func_prefix = '#include <winsock2.h>\n#include <ws2tcpip.h>\n#include <string.h>'
else
  func_prefix = '''#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>'''
endif

check_functions = [
  'accept4',
  'gai_strerror',
  'getaddrinfo',
  'gettimeofday',
  'inet_pton',
  'inet_ntop',
  'select',
  'socket',
  'strerror',
  'strlcpy',
]

foreach f : check_functions
  if cc.has_function(f, prefix : '#define _GNU_SOURCE\n' + func_prefix,
                     dependencies : libmodbus_deps)
    cdata.set('HAVE_' + f.to_upper(), 1)
  endif
endforeach

# Check for RS485 support (Linux kernel version 2.6.28+)
cdata.set10('HAVE_DECL_TIOCSRS485', cc.has_header_symbol('sys/ioctl.h', 'TIOCSRS485'))
# Check for RTS flags
cdata.set10('HAVE_DECL_TIOCM_RTS', cc.has_header_symbol('sys/ioctl.h', 'TIOCM_RTS'))
# Check for termios2 support
if cc.has_type('struct termios2', prefix : '#include <asm/termbits.h>')
  cdata.set('HAVE_STRUCT_TERMIOS2', 1)
endif

config_h = configure_file(output : 'config.h', configuration : cdata)
root_inc = include_directories('.')

# Like the autotools build, force-include config.h in every translation unit
# as some sources test HAVE_* macros before including it themselves.
if cc.get_argument_syntax() == 'msvc'
  config_h_args = ['/FI' + meson.current_build_dir() / 'config.h']
else
  config_h_args = ['-include', meson.current_build_dir() / 'config.h']
endif

add_project_arguments(cc.get_supported_arguments([
  '-Wmissing-declarations',
  '-Wmissing-prototypes',
  '-Wnested-externs',
  '-Wpointer-arith',
  '-Wsign-compare',
  '-Wchar-subscripts',
  '-Wstrict-prototypes',
  '-Wshadow',
  '-Wformat-security',
]), language : 'c')

subdir('src')

if get_option('tests')
  subdir('tests')
endif

pkg = import('pkgconfig')
pkg.generate(libmodbus,
  name : 'modbus',
  filebase : 'libmodbus',
  description : 'Modbus library',
  subdirs : 'modbus')

install_data('AUTHORS', 'NEWS.md', 'README.md',
  install_dir : get_option('datadir') / 'doc' / meson.project_name())

summary({
  'prefix' : get_option('prefix'),
  'sysconfdir' : get_option('sysconfdir'),
  'libdir' : get_option('libdir'),
  'includedir' : get_option('includedir'),
}, section : 'Directories')

summary({
  'compiler' : cc.get_id(),
  'buildtype' : get_option('buildtype'),
  'library' : get_option('default_library'),
  'tests' : get_option('tests'),
}, section : 'Configuration', bool_yn : true)
