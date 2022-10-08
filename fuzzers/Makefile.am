EXTRA_DIST = README.md

noinst_PROGRAMS = \
	FuzzServer \
	FuzzClient

common_ldflags = \
	$(top_builddir)/src/libmodbus.la

FuzzServer_SOURCES = FuzzServer.c
FuzzServer_LDADD = $(common_ldflags) $(FUZZENGINE)

FuzzClient_SOURCES = FuzzClient.c
FuzzClient_LDADD = $(common_ldflags) $(FUZZENGINE)

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -I${top_srcdir}/src \
    -I${top_builddir}/src

AM_CFLAGS = ${my_CFLAGS}

CLEANFILES = *~ 
