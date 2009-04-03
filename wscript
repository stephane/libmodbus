#! /usr/bin/env python
# encoding: utf-8

VERSION='2.0.3'
APPNAME='libmodbus'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

def set_options(opt):
     # options provided by the modules
     opt.tool_options('compiler_cc')

def configure(conf):
     conf.check_tool('compiler_cc')
     conf.check_tool('misc')

     headers = 'string.h termios.h sys/time.h \
                unistd.h errno.h limits.h fcntl.h \
                sys/types.h sys/socket.h sys/ioctl.h \
                netinet/in.h netinet/ip.h netinet/tcp.h arpa/inet.h'

     # check for headers and append found headers to headers_found for later use
     headers_found = []
     for header in headers.split():
          if conf.check_cc(header_name=header):
               headers_found.append(header)

     functions_headers = (
          ('setsockopt', 'sys/socket.h'),
          ('inet_ntoa', 'arpa/inet.h'),
          ('memset', 'string.h'),
          ('select', 'sys/select.h'),
          ('socket', 'sys/socket.h'),
          )

     for (function, headers) in functions_headers:
          conf.check_cc(function_name=function, header_name=headers, mandatory=1)

     conf.define('VERSION', VERSION)
     conf.define('PACKAGE', 'libmodbus')

     conf.write_config_header()

def build(bld):
     import misc

     bld.add_subdirs('src tests')

     obj = bld.new_task_gen(features='subst',
                            source='modbus.pc.in',
                            target='modbus.pc',
                            dict = {'VERSION' : VERSION,
                                    'prefix': bld.env['PREFIX'],
                                    'exec_prefix': bld.env['PREFIX'],
                                    'libdir': bld.env['PREFIX'] + 'lib',
                                    'includedir': bld.env['PREFIX'] + 'include'}
                            )

     bld.install_files('${PREFIX}/lib/pkgconfig', 'modbus.pc')

def shutdown():
     import UnitTest
     unittest = UnitTest.unit_test()
     unittest.run()
     unittest.print_results()
