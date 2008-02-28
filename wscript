#! /usr/bin/env python
# encoding: utf-8

VERSION='1.2.4'
APPNAME='libmodbus'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = '_build_'

def init():
     print "A groovy libmodbus for Linux!"

def set_options(opt):
     # options provided by the modules
     opt.tool_options('compiler_cc')

def configure(conf):
     conf.check_tool('compiler_cc')
     conf.check_pkg('glib-2.0', destvar='GLIB', mandatory=True)

     e = conf.create_header_enumerator()
     e.mandatory = 1
     # Add AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h termio.h termios.h unistd.h])
     e.name = 'arpa/inet.h'
     e.run()

     # Add AC_CHECK_FUNCS([inet_ntoa memset select socket])
     conf.define('VERSION', VERSION)
     conf.define('PACKAGE', 'libmodbus')

     conf.write_config_header()

def build(bld):
     bld.add_subdirs('src');  

#    Doesn't work I need some help from the WAF project
#      obj = bld.create_obj('subst')
#      obj.source = 'modbus.pc.in'
#      obj.target = 'modbus.pc'
#      obj.dict = {'VERSION' : VERSION }

def shutdown():
     import UnitTest
     unittest = UnitTest.unit_test()
     unittest.run()
     unittest.print_results()
