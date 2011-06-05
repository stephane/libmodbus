#! /usr/bin/env python
# encoding: utf-8

VERSION='2.0.4'
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
          if conf.check_header(header):
               headers_found.append(header)

     functions_defines = (
          ('setsockopt', 'HAVE_SETSOCKOPT'),
          ('inet_ntoa', 'HAVE_INET_NTOA'),
          ('memset', 'HAVE_MEMSET'),
          ('select', 'HAVE_SELECT'),
          ('socket', 'HAVE_SOCKET'))

     for (function, define) in functions_defines:
          e = conf.create_function_enumerator()
          e.mandatory = True
          e.function = function
          e.headers = headers_found
          e.define = define
          e.run()

     conf.define('VERSION', VERSION)
     conf.define('PACKAGE', 'libmodbus')

     conf.write_config_header()

def build(bld):
     import misc

     bld.add_subdirs('modbus tests')  

     obj = bld.create_obj('subst')
     obj.source = 'modbus.pc.in'
     obj.target = 'modbus.pc'
     obj.dict = {'VERSION' : VERSION, 
                 'prefix': bld.env()['PREFIX'], 
                 'exec_prefix': bld.env()['PREFIX'],
                 'libdir': bld.env()['PREFIX'] + 'lib', 
                 'includedir': bld.env()['PREFIX'] + 'include'}

     install_files('PREFIX', 'lib/pkgconfig', 'modbus.pc')

def shutdown():
     import UnitTest
     unittest = UnitTest.unit_test()
     unittest.run()
     unittest.print_results()
