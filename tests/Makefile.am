EXTRA_DIST = README

# Very basic test
TESTS = version

noinst_PROGRAMS = \
	bandwidth-server-one \
	bandwidth-server-many-up \
	bandwidth-client \
	random-test-server \
	random-test-client \
	unit-test-server \
	unit-test-client \
    unit-test-harness \
	version

common_ldflags = \
	$(top_builddir)/src/libmodbus.la

bandwidth_server_one_SOURCES = bandwidth-server-one.c
bandwidth_server_one_LDADD = $(common_ldflags)

bandwidth_server_many_up_SOURCES = bandwidth-server-many-up.c
bandwidth_server_many_up_LDADD = $(common_ldflags)

bandwidth_client_SOURCES = bandwidth-client.c
bandwidth_client_LDADD = $(common_ldflags)

random_test_server_SOURCES = random-test-server.c
random_test_server_LDADD = $(common_ldflags)

random_test_client_SOURCES = random-test-client.c
random_test_client_LDADD = $(common_ldflags)

unit_test_server_SOURCES = unit-test-server.c \
                           unit-test.c \
                           unit-test.h
unit_test_server_LDADD = $(common_ldflags)

unit_test_client_SOURCES = unit-test-client.c \
                           unit-test.c \
                           unit-test.h
unit_test_client_LDADD = $(common_ldflags)

if USE_PTHREAD
unit_test_harness_CC = $(PTHREAD_CC)
unit_test_harness_CFLAGS = $(PTHREAD_CFLAGS)
unit_test_harness_SOURCES = unit-test-harness.c \
                            unit-test.c \
                            unit-test.h

unit_test_harness_LDADD = $(common_ldflags) $(PTHREAD_LIBS)
TESTS += unit-test-harness
endif

version_SOURCES = version.c
version_LDADD = $(common_ldflags)

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -DSYSCONFDIR=\""$(sysconfdir)"\" \
    -DLIBEXECDIR=\""$(libexecdir)"\" \
    -I${top_srcdir}/src \
    -I${top_builddir}/src

AM_CFLAGS = ${my_CFLAGS}

CLEANFILES = *~
