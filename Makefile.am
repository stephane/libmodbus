CLEANFILES =
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

# GNU make-ism, precludes builds with BSD and Sun Makes at least
#AM_MAKEFLAGS = --no-print-directory

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmodbus.pc
EXTRA_DIST = libmodbus.pc.in
CLEANFILES += libmodbus.pc

dist_doc_DATA = AUTHORS NEWS README.md

SUBDIRS = src

if BUILD_TESTS
SUBDIRS += tests
endif
