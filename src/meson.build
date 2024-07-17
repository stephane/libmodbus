modbus_version_h = configure_file(input : 'modbus-version.h.in',
                                  output : 'modbus-version.h',
                                  configuration : ver_conf)

sources = files([
    'modbus.c',
    'modbus-data.c',
    'modbus-rtu.c',
    'modbus-tcp.c',
])

headers = [
    'modbus.h',
    modbus_version_h,
    'modbus-rtu.h',
    'modbus-tcp.h',
]
install_headers(headers, subdir : 'modbus')

lt_current = 6
lt_revision = 0
lt_age = 1
lt_version = '@0@.@1@.@2@'.format(lt_current - lt_age, lt_age, lt_revision)

libmodbus = shared_library('modbus', sources,
                           version: lt_version,
                           include_directories : include_directories('..'),
                           install : true)

incdir = include_directories('.')
libmodbus_dep = declare_dependency(link_with : libmodbus,
                                   include_directories: incdir)
