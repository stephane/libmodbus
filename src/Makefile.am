lib_LTLIBRARIES = libmodbus.la
libmodbus_la_SOURCES = modbus.c modbus.h

bin_PROGRAMS = test-modbus
test_modbus_SOURCES = test-modbus.c
test_modbus_INCLUDES = @GLIB_CFLAGS@
test_modbus_LDADD = libmodbus.la @GLIB_LIBS@

INCLUDES = @GLIB_CFLAGS@
LDADD = @GLIB_LIBS@
CLEANFILES = *~

