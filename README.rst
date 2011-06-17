=========================
 A groovy modbus library
=========================

Overview
--------

libmodbus is a free software library to send/receive data with a
device which respects the Modbus protocol. This library can use a
serial port or an Ethernet connection.

The functions included in the library have been derived from the
Modicon Modbus Protocol Reference Guide which can be obtained from
Schneider at www.schneiderautomation.com.

The license of libmodbus is LGPL v2.1 or later and the licence of programs in
tests directory is GPL v3.

The documentation is available under the Creative Commons Attribution-ShareAlike
License 3.0 (Unported) (http://creativecommons.org/licenses/by-sa/3.0/).

The official website is http://www.libmodbus.org.

The library is written in C and designed to run on Linux, Mac OS X, FreeBSD and
QNX and Windows.

Installation
------------

The shell commands are ``./autogen.sh; ./configure; make; make install``.

If you want to compile with Microsoft Visual Studio, you need to install
http://code.google.com/p/msinttypes/ to fill the absence of stdint.h.

Testing
-------

Some tests are provided in the ``tests`` directory, you can
freely edit the source code to fit your needs (it's Free Sofware :).

See ``tests/README`` for a description of each program.

Report a Bug
------------

To report a bug, you can:
 * fill a bug report on the issue tracker
   http://github.com/stephane/libmodbus/issues
 * or send an email to stephane.raimbault@gmail.com
