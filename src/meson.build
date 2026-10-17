modbus_version_h = configure_file(input : 'modbus-version.h.in',
                                  output : 'modbus-version.h',
                                  configuration : ver_conf)

if host_system == 'windows'
  configure_file(input : 'win32/modbus.dll.manifest.in',
                 output : 'modbus.dll.manifest',
                 configuration : ver_conf)
endif

sources = files(
  'modbus.c',
  'modbus-data.c',
  'modbus-rtu.c',
  'modbus-tcp.c',
)

install_headers(
  'modbus.h',
  modbus_version_h,
  'modbus-rtu.h',
  'modbus-tcp.h',
  subdir : 'modbus')

lib_c_args = config_h_args
if cc.get_argument_syntax() == 'msvc'
  lib_c_args += ['-DDLLBUILD']
endif

# Map libtool's current:revision:age to the same file names libtool produces
lt_major = libmodbus_ld_current - libmodbus_ld_age
lt_version = '@0@.@1@.@2@'.format(lt_major, libmodbus_ld_age, libmodbus_ld_revision)
darwin_current = libmodbus_ld_current + 1

libmodbus = library('modbus', sources,
  version : lt_version,
  soversion : lt_major,
  darwin_versions : [darwin_current, '@0@.@1@'.format(darwin_current, libmodbus_ld_revision)],
  c_args : lib_c_args,
  include_directories : root_inc,
  dependencies : libmodbus_deps,
  install : true)

libmodbus_dep = declare_dependency(link_with : libmodbus,
  include_directories : include_directories('.'))

meson.override_dependency('libmodbus', libmodbus_dep)
