EXTRA_DIST =
lib_LTLIBRARIES = libmodbus.la

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -DSYSCONFDIR=\""$(sysconfdir)"\" \
    -DLIBEXECDIR=\""$(libexecdir)"\" \
    -I${top_srcdir}/src

AM_CFLAGS = ${WARNING_CFLAGS}

libmodbus_la_SOURCES = \
        modbus.c \
        modbus.h \
        modbus-config.h \
        modbus-data.c \
        modbus-private.h \
        modbus-rtu.c \
        modbus-rtu.h \
        modbus-rtu-private.h \
        modbus-tcp.c \
        modbus-tcp.h \
        modbus-tcp-private.h \
        modbus-version.h

if WITH_LIBUSB
libmodbus_la_SOURCES += \
        modbus-rtu-usb.c \
        modbus-rtu-usb.h \
        modbus-rtu-usb-private.h
endif

libmodbus_la_LDFLAGS = -no-undefined \
        -version-info $(LIBMODBUS_LT_VERSION_INFO)

libmodbus_la_LIBADD =

if WITH_LIBUSB
libmodbus_la_LIBADD += $(LIBUSB_LIBS)
endif

if OS_WIN32
libmodbus_la_LIBADD += -lwsock32
endif

if OS_QNX
libmodbus_la_LIBADD += -lsocket
endif

# Header files to install
libmodbusincludedir = $(includedir)/modbus
libmodbusinclude_HEADERS = modbus.h modbus-config.h modbus-version.h modbus-rtu.h modbus-tcp.h

if WITH_LIBUSB
libmodbusinclude_HEADERS += modbus-rtu-usb.h
endif

DISTCLEANFILES = modbus-config.h modbus-version.h
EXTRA_DIST += modbus-config.h.in modbus-version.h.in
CLEANFILES = *~
