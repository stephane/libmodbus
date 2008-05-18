EXTRA_DIST = README

noinst_PROGRAMS = \
	random-test-slave \
	random-test-master \
	unit-test-slave \
	unit-test-master \
	bench-bandwidth-slave \
	bench-bandwidth-master

common_ldflags = \
	$(top_builddir)/modbus/libmodbus.la

random_test_slave_SOURCES = random-test-slave.c
random_test_slave_LDADD = $(common_ldflags)

random_test_master_SOURCES = random-test-master.c
random_test_master_LDADD = $(common_ldflags)

unit_test_slave_SOURCES = unit-test-slave.c unit-test.h
unit_test_slave_LDADD = $(common_ldflags)

unit_test_master_SOURCES = unit-test-master.c unit-test.h
unit_test_master_LDADD = $(common_ldflags)

bench_bandwidth_slave_SOURCES = bench-bandwidth-slave.c
bench_bandwidth_slave_LDADD = $(common_ldflags)

bench_bandwidth_master_SOURCES = bench-bandwidth-master.c
bench_bandwidth_master_LDADD = $(common_ldflags)

INCLUDES = -I$(top_srcdir)
CLEANFILES = *~
