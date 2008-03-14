lib_LTLIBRARIES = libmodbus.la
libmodbus_la_SOURCES = modbus.c modbus.h

bin_PROGRAMS = test-modbus-master test-modbus-slave
test_modbus_master_SOURCES = test-modbus-master.c
test_modbus_master_LDADD = libmodbus.la

test_modbus_slave_SOURCES = test-modbus-slave.c
test_modbus_slave_LDADD = libmodbus.la

INCLUDES = -I$(top_srcdir)/include
CLEANFILES = *~
