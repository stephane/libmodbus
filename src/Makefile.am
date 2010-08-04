lib_LTLIBRARIES = libmodbus.la
libmodbus_la_SOURCES = \
        modbus.c \
        modbus.h \
        modbus-version.h
libmodbus_la_LDFLAGS = $(AM_LDFLAGS) -version-info $(LIBMODBUS_LT_VERSION_INFO)

# Header files to install
libmodbusincludedir = $(includedir)/modbus
libmodbusinclude_HEADERS = modbus.h modbus-version.h

DISTCLEANFILES = modbus-version.h
EXTRA_DIST = modbus-version.h.in
CLEANFILES = *~
