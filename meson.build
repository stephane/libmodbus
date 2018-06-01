project('libmodbus', 'c')

#

conf_data = configuration_data()

conf_data.set('LIBMODBUS_VERSION_MAJOR', '3')
conf_data.set('LIBMODBUS_VERSION_MINOR', '1')
conf_data.set('LIBMODBUS_VERSION_MICRO', '4')

conf_data.set('LIBMODBUS_VERSION', '3.1.4')

configure_file(input : 'config.h.in',
               output : 'config.h',
               configuration : conf_data)

configure_file(input : 'modbus-version.h.in',
               output : 'modbus-version.h',
               configuration : conf_data)

#

libmodbus_la_SOURCES = [
    'src/modbus.c',
    'src/modbus-data.c',
    'src/modbus-rtu.c',
    'src/modbus-tcp.c',
]

incdir = include_directories('include')

modbus = static_library('libmodbus', libmodbus_la_SOURCES, include_directories : incdir)

executable('00_modbus_tcp_listen', 'example/00_modbus_tcp_listen.c', include_directories : incdir, link_with : [ modbus ] )
executable('01_modbus_tcp_connect', 'example/01_modbus_tcp_connect.c', include_directories : incdir, link_with : [ modbus ] )
