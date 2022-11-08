configure_file(input : 'unit-test.h.in',
               output : 'unit-test.h',
               configuration : cdata)

bandwidth_server_one = executable(
    'bandwidth-server-one', 'bandwidth-server-one.c',
    dependencies : libmodbus_dep
)

bandwidth_server_many_up = executable(
    'bandwidth-server-many-up', 'bandwidth-server-many-up.c',
    dependencies : libmodbus_dep
)

bandwidth_client = executable(
    'bandwidth-client', 'bandwidth-client.c',
    dependencies : libmodbus_dep
)

random_test_server = executable(
    'random-test-server', 'random-test-server.c',
    dependencies : libmodbus_dep
)

random_test_client = executable(
    'random-test-client', 'random-test-client.c',
    dependencies : libmodbus_dep
)

unit_test_server = executable(
    'unit-test-server', 'unit-test-server.c',
    dependencies : libmodbus_dep
)

unit_test_client = executable(
    'unit-test-client', 'unit-test-client.c',
    dependencies : libmodbus_dep
)

test('unit-tests', find_program('unit-tests.sh'),
     workdir : meson.current_build_dir())
