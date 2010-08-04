lib_LTLIBRARIES = libmodbus.la
libmodbus_la_SOURCES = modbus.c modbus.h
libmodbus_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBMODBUS_LT_VERSION_INFO)

# Include files to install
libmodbusincludedir = $(includedir)
libmodbusinclude_HEADERS = modbus.h

DISTCLEANFILES = modbus.h
EXTRA_DIST = modbus.h.in
CLEANFILES = *~
