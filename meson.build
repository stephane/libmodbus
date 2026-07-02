project('libmodbus', 'c', version : '3.1.8')

libmodbus_version = meson.project_version()
ver_arr = libmodbus_version.split('.')

libmodbus_version_major = ver_arr[0]
libmodbus_version_minor = ver_arr[1]
libmodbus_version_micro = ver_arr[2]

ver_conf = configuration_data()
ver_conf.set('LIBMODBUS_VERSION', libmodbus_version)
ver_conf.set('LIBMODBUS_VERSION_MAJOR', libmodbus_version_major)
ver_conf.set('LIBMODBUS_VERSION_MINOR', libmodbus_version_minor)
ver_conf.set('LIBMODBUS_VERSION_MICRO', libmodbus_version_micro)

cc = meson.get_compiler('c')
cdata = configuration_data()
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

check_functions = [
  ['HAVE_ACCEPT4', 'accept4', '#include<sys/socket.h>'],
  ['HAVE_GETADDRINFO', 'getaddrinfo', '#include<netdb.h>'],
  ['HAVE_GETTIMEOFDAY', 'gettimeofday', '#include<sys/time.h>'],
  ['HAVE_SELECT', 'select', '#include<sys/select.h>'],
  ['HAVE_SOCKET', 'socket', '#include<sys/socket.h>'],
  ['HAVE_STRERROR', 'strerror', '#include<string.h>'],
  ['HAVE_STRLCPY', 'strlcpy', '#include<string.h>'],
]

foreach f : check_functions
  if cc.has_function(f.get(1), prefix : f.get(2))
    cdata.set(f.get(0), 1)
  endif
endforeach

configure_file(output : 'config.h',
               configuration : cdata)

subdir('src')
subdir('tests')
