lib_LTLIBRARIES = libmodbus.la
libmodbus_la_SOURCES = modbus.c modbus.h
libmodbus_la_LDFLAGS = -version-info 2:0:0
service_CFLAGS = -I$(top_srcdir)/

# Include files to install
libmodbusincludedir = $(includedir)/modbus
libmodbusinclude_HEADERS = modbus.h

CLEANFILES = *~
