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

The license of libmodbus is LGPL v3 and the licence of programs in tests
directory is GPL v3.

The official website is http://www.libmodbus.org.

The library is written in C and designed to run on Linux, Mac OS X, FreeBSD and
QNX and Windows.

Installation
------------

The shell commands are ``./autogen.sh; ./configure; make; make install``.

Testing
-------

Some tests are provided in the ``tests`` directory, you can
freely edit the source code to fit your needs (it's Free Sofware :).

See ``tests/README`` for a description of each program.

Report a Bug
------------

To report a bug, you can:
 * send an email to stephane.raimbault@gmail.com
 * or fill a bug report on the issue tracker
   http://github.com/stephane/libmodbus/issues
