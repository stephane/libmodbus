EXTRA_DIST = README

noinst_PROGRAMS = \
	random-test-slave \
	random-test-master \
	unit-test-slave \
	unit-test-master \
	bandwidth-slave-one \
	bandwidth-slave-many-up \
	bandwidth-master

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

bandwidth_slave_one_SOURCES = bandwidth-slave-one.c
bandwidth_slave_one_LDADD = $(common_ldflags)

bandwidth_slave_many_up_SOURCES = bandwidth-slave-many-up.c
bandwidth_slave_many_up_LDADD = $(common_ldflags)

bandwidth_master_SOURCES = bandwidth-master.c
bandwidth_master_LDADD = $(common_ldflags)

INCLUDES = -I$(top_srcdir)
CLEANFILES = *~
