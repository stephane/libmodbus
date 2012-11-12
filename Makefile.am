EXTRA_DIST = MIGRATION README.md libmodbus.spec
CLEANFILES =
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmodbus.pc
EXTRA_DIST += libmodbus.pc.in
CLEANFILES += libmodbus.pc

SUBDIRS = src doc tests
