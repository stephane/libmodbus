noinst_PROGRAMS = \
	unit-test-master \
	unit-test-slave \
	test-master-random

common_ldflags = \
	$(top_builddir)/modbus/libmodbus.la

unit_test_master_SOURCES = unit-test-master.c
unit_test_master_LDADD = $(common_ldflags)

unit_test_slave_SOURCES = unit-test-slave.c
unit_test_slave_LDADD = $(common_ldflags)

test_master_random_SOURCES = test-master-random.c
test_master_random_LDADD = $(common_ldflags)

INCLUDES = -I$(top_srcdir)
CLEANFILES = *~
