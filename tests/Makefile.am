EXTRA_DIST = README

noinst_PROGRAMS = \
	bandwidth-server-one \
	bandwidth-server-many-up \
	bandwidth-client \
	random-test-server \
	random-test-client \
	unit-test-server \
	unit-test-client \
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

unit_test_server_SOURCES = unit-test-server.c unit-test.h
unit_test_server_LDADD = $(common_ldflags)

unit_test_client_SOURCES = unit-test-client.c unit-test.h
unit_test_client_LDADD = $(common_ldflags)

version_SOURCES = version.c
version_LDADD = $(common_ldflags)

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src
CLEANFILES = *~
