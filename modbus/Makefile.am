lib_LTLIBRARIES = libmodbus.la
libmodbus_la_SOURCES = modbus.c modbus.h version.h
libmodbus_la_LDFLAGS = $(MB_LT_LDFLAGS)

# Include files to install
libmodbusincludedir = $(includedir)/modbus
libmodbusinclude_HEADERS = modbus.h version.h

DISTCLEANFILES = version.h
EXTRA_DIST = version.h.in
CLEANFILES = *~
