# A groovy modbus library

![Build Status](https://github.com/stephane/libmodbus/actions/workflows/build.yml/badge.svg)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/8058/badge.svg)](https://scan.coverity.com/projects/libmodbus)

## Overview

libmodbus is a free software library to send/receive data with a device which
respects the Modbus protocol. This library can use a serial port or an Ethernet
connection.

The functions included in the library have been derived from the Modicon Modbus
Protocol Reference Guide which can be obtained from [www.modbus.org](http://www.modbus.org).

The license of libmodbus is _LGPL v2.1 or later_.

The official website is [www.libmodbus.org](http://www.libmodbus.org). The
website contains the latest version of the documentation.

The library is written in C and designed to run on Linux, Mac OS X, FreeBSD, Embox,
QNX and Windows.

You can use the library on MCUs with Embox RTOS.

## Installation

You will only need to install automake, autoconf, libtool and a C compiler (gcc
or clang) to compile the library and asciidoc and xmlto to generate the
documentation (optional).

To install, just run the usual dance, `./configure && make install`. Run
`./autogen.sh` first to generate the `configure` script if required.

You can change installation directory with prefix option, eg. `./configure
--prefix=/usr/local/`. You have to check that the installation library path is
properly set up on your system (_/etc/ld.so.conf.d_) and library cache is up to
date (run `ldconfig` as root if required).

The library provides a _libmodbus.pc_ file to use with `pkg-config` to ease your
program compilation and linking.

### Meson (experimental)

The library can also be built with [Meson](https://mesonbuild.com) (0.56 or
later) as an experimental alternative to autotools:

```shell
meson setup build
meson compile -C build
meson test -C build
meson install -C build
```

Use `-Dtests=false` to skip building the tests and `-Ddefault_library=static`
(or `both`) to build a static library.

If you want to compile with Microsoft Visual Studio, you should follow the
instructions in `./src/win32/README.md`.

To compile under Windows, install [MinGW](http://www.mingw.org/) and MSYS then
select the common packages (gcc, automake, libtool, etc). The directory
_./src/win32/_ contains a Visual C project.

To compile under OS X with [homebrew](http://mxcl.github.com/homebrew/), you
will need to install the following dependencies first: `brew install autoconf
automake libtool`.

To build under Embox, you have to use its build system.

## Testing

Some tests are provided in _tests_ directory, you can freely edit the source
code to fit your needs (it's Free Software :).

See _tests/README_ for a description of each program.

For a quick test of libmodbus, you can run the following programs in two shells:

1. ./unit-test-server
2. ./unit-test-client

By default, all TCP unit tests will be executed (see --help for options).

It's also possible to run the unit tests with `make check` (or `meson test -C
build` with Meson).

## To report a bug or to contribute

See [CONTRIBUTING](CONTRIBUTING.md) document.

By contributing to libmodbus, you agree to the [Contributor License Agreement](CLA.txt).

## Documentation

You can serve the local documentation with:

```shell
pip install mkdocs-material
mkdocs serve
```
