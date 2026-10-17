configure_file(input : 'unit-test.h.in',
               output : 'unit-test.h',
               configuration : cdata)

test_programs = [
  'bandwidth-server-one',
  'bandwidth-server-many-up',
  'bandwidth-client',
  'random-test-server',
  'random-test-client',
  'unit-test-server',
  'unit-test-client',
  'proxy-test-server',
  'proxy-test-client',
  'version',
]

foreach p : test_programs
  executable(p, p + '.c',
    c_args : config_h_args,
    include_directories : root_inc,
    dependencies : libmodbus_dep)
endforeach

# The scripts start the programs from the current directory and the servers
# listen on fixed TCP ports, so the tests must not run in parallel.
test('unit-tests', find_program('unit-tests.sh'),
     workdir : meson.current_build_dir(),
     is_parallel : false,
     timeout : 120)

test('proxy-tests', find_program('proxy-tests.sh'),
     workdir : meson.current_build_dir(),
     is_parallel : false,
     timeout : 120)
