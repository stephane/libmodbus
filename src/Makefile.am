lib_LTLIBRARIES = libmodbus.la
libmodbus_la_SOURCES = modbus.c modbus.h

bin_PROGRAMS = test-modbus-master test-modbus-slave
test_modbus_master_SOURCES = test-modbus-master.c
test_modbus_master_INCLUDES = @GLIB_CFLAGS@
test_modbus_master_LDADD = libmodbus.la @GLIB_LIBS@

test_modbus_slave_SOURCES = test-modbus-slave.c
test_modbus_slave_INCLUDES = @GLIB_CFLAGS@
test_modbus_slave_LDADD = libmodbus.la @GLIB_LIBS@

INCLUDES = @GLIB_CFLAGS@
LDADD = @GLIB_LIBS@
CLEANFILES = *~

# Include files to install
libmodbusincludedir = $(includedir)/libmodbus
libmodbusinclude_HEADERS = modbus.h
