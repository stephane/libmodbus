A groovy modbus library
=======================

Overview
--------

libmodbus is a free software library to send/receive data with a device which
respects the Modbus protocol. This library can use a serial port or an Ethernet
connection.

The functions included in the library have been derived from the Modicon Modbus
Protocol Reference Guide which can be obtained from Schneider at
[www.schneiderautomation.com](http://www.schneiderautomation.com).

The license of libmodbus is *LGPL v2.1 or later*.

The documentation is available as manual pages (`man libmodbus` to read general
description and list of available functions) or Web pages
[www.libmodbus.org/documentation/](http://libmodbus.org/documentation/). The
documentation is licensed under the Creative Commons Attribution-ShareAlike
License 3.0 (Unported) (<http://creativecommons.org/licenses/by-sa/3.0/>).

The official website is [www.libmodbus.org](http://www.libmodbus.org).

The library is written in C and designed to run on Linux, Mac OS X, FreeBSD and
QNX and Windows.

Installation
------------

You will only need to install automake, autoconf, libtool and a C compiler (gcc
or clang) to compile the library and asciidoc and xmlto to generate the
documentation (optional).

To install, just run the usual dance, `./configure && make install`. Run
`./autogen.sh` first to generate the `configure` script if required.

You can change installation directory with prefix option, eg. `./configure
--prefix=/usr/local/`. You have to check that the installation library path is
properly set up on your system (`/etc/ld.so.conf.d`) and library cache is up to
date (run `ldconfig` as root if required).

The library provides a `libmodbus.pc` file to use with `pkg-config` to ease your
program compilation and linking.

If you want to compile with Microsoft Visual Studio, you need to install
<http://code.google.com/p/msinttypes/> to fill the absence of stdint.h.

To compile under Windows, install [MinGW](http://www.mingw.org/) and MSYS then
select the common packages (gcc, automake, libtool, etc). The directory
`./src/win32/` contains a Visual C project.

To compile under OS X with [homebrew](http://mxcl.github.com/homebrew/), you will need
to install the following dependencies first: `brew install autoconf automake libtool`.

Testing
-------

Some tests are provided in *tests* directory, you can freely edit the source
code to fit your needs (it's Free Software :).

See *tests/README* for a description of each program.

For a quick test of libmodbus, you can run the following programs in two shells:

1. ./unit-test-server
2. ./unit-test-client

By default, all TCP unit tests will be executed (see --help for options).

Report a Bug
------------

Before reporting a bug, take care to read the documentation (RTFM!) and to
provide enough information:

1. libmodbus version
2. OS/environment/architecture
3. libmodbus backend (TCP, RTU, IPv6)
3. Modbus messages when running in debug mode (`man modbus_set_debug`)

To report your problem, you can:

* fill a bug report on the issue tracker <http://github.com/stephane/libmodbus/issues>.
* or send an email to the libmodbus mailing list [libmodbus@googlegroups.com](https://groups.google.com/forum/#!forum/libmodbus).

If your prefer live talk when your're looking for help or to offer contribution,
there is also a channel called #libmodbus on Freenode.
