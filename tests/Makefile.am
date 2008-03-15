noinst_PROGRAMS = \
	test-modbus-master \
	test-modbus-slave

common_ldflags = \
	$(top_builddir)/modbus/libmodbus.la

test_modbus_master_SOURCES = test-modbus-master.c
test_modbus_master_LDADD = $(common_ldflags)

test_modbus_slave_SOURCES = test-modbus-slave.c
test_modbus_slave_LDADD = $(common_ldflags)

INCLUDES = -I$(top_srcdir)
CLEANFILES = *~
