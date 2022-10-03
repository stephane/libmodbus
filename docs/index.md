# libmodbus

A featureful and portable Open Source Modbus library.

## Description

libmodbus is a library to send/receive data with a device which respects the
Modbus protocol. This library contains various backends to communicate over
different networks (eg. serial in RTU mode or Ethernet in TCP IPv4/IPv6). The
<http://www.modbus.org> site provides documentation about the [Modbus
Specifications and Implementation Guides](http://www.modbus.org/specs.php).

libmodbus provides an abstraction of the lower communication layers and offers
the same API on all supported platforms.

This documentation presents an overview of libmodbus concepts, describes how
libmodbus abstracts Modbus communication with different hardware and platforms
and provides a reference manual for the functions provided by the libmodbus
library.

## Use cases

The library can be used to write a:

- **client**, the application reads/writes data from various devices.
- **server**, the application provides data to several clients.

<figure markdown>
  <img src="assets/client-sensors.webp" width="512">
  <figcaption>A libmodbus client that reads only the temperatures from sensors.</figcaption>
</figure>

<figure markdown>
  <img src="assets/server-grafana.webp" width="512">
  <figcaption>A libmodbus server that exposes data to a Grafana service.</figcaption>
</figure>

## Contexts

The Modbus protocol supports several transport protocols (eg. serial RTU,
Ethernet TCP) called backends in *libmodbus*.

The first step is to allocate and set a `modbus_t` context according to the
required backend (RTU or TCP) with a dedicated function, such as
[modbus_new_rtu](modbus_new_rtu.md).
The function will return an opaque structure called `modbus_t` containing all
necessary information to establish a connection with other Modbus devices
according to the selected backend.

Once this context has been created, you can use use the common API provided by
*libmodbus* to read/write or set the various timeouts. With this common API,
it's easy to switch the backend of your application from RTU to TCP IPv6 for
example.

### RTU Context

The RTU backend (Remote Terminal Unit) is used in serial communication and makes
use of a compact, binary representation of the data for protocol communication.
The RTU format follows the commands/data with a cyclic redundancy check checksum
as an error check mechanism to ensure the reliability of data. Modbus RTU is the
most common implementation available for Modbus. A Modbus RTU message must be
transmitted continuously without inter-character hesitations (extract from
Wikipedia, [Modbus](http://en.wikipedia.org/wiki/Modbus) as of Mar. 13, 2011,
20:51 GMT).

The Modbus RTU framing calls a slave, a device/service which handle Modbus
requests, and a master, a client which send requests. The communication is
always initiated by the master.

Many Modbus devices can be connected together on the same physical link so
before sending a message, you must set the slave (receiver) with
[modbus_set_slave](modbus_set_slave.md). If you're running a slave, its slave number
will be used to filter received messages.

The libmodbus implementation of RTU isn't time based as stated in original
Modbus specification, instead all bytes are sent as fast as possible and a
response or an indication is considered complete when all expected characters
have been received. This implementation offers very fast communication but you
must take care to set a response timeout of slaves less than response timeout of
master (ortherwise other slaves may ignore master requests when one of the slave
is not responding).

To create a Modbus RTU context, you should use [modbus_new_rtu](modbus_new_rtu.md).

You can tweak the serial mode with the following functions:

- [modbus_rtu_get_serial_mode](modbus_rtu_get_serial_mode.md)
- [modbus_rtu_set_serial_mode](modbus_rtu_set_serial_mode.md)
- [modbus_rtu_get_rts](modbus_rtu_get_rts.md)
- [modbus_rtu_set_rts](modbus_rtu_set_rts.md)
- [modbus_rtu_set_custom_rts](modbus_rtu_set_custom_rts.md)
- [modbus_rtu_get_rts_delay](modbus_rtu_get_rts_delay.md)
- [modbus_rtu_set_rts_delay](modbus_rtu_set_rts_delay.md)

### TCP (IPv4) Context

The TCP backend implements a Modbus variant used for communications over
TCP/IPv4 networks. It does not require a checksum calculation as lower layer
takes care of the same.

To create a Modbus TCP context, you should use [modbus_new_tcp](modbus_new_tcp.md).

### TCP PI (IPv4 and IPv6) Context

The TCP PI (Protocol Independent) backend implements a Modbus variant used for
communications over TCP IPv4 and IPv6 networks. It does not require a checksum
calculation as lower layer takes care of the same.

Contrary to the TCP IPv4 only backend, the TCP PI backend offers hostname
resolution but it consumes about 1Kb of additional memory.

Create a Modbus TCP PI context, you should use [modbus_new_tcp_pi](modbus_new_tcp_pi.md).

## Connection

The following functions are provided to establish and close a connection with
Modbus devices:

- [modbus_connect](modbus_connect.md) establishes a connection.
- [modbus_close](modbus_close.md) closes a connection.
- [modbus_flush](modbus_flush.md) flushed a connection.

In RTU, you should define the slave ID of your client with
[modbus_set_slave](modbus_set_slave.md).

To analyse the exchanged data, you can enable the debug mode with
[modbus_set_debug](modbus_set_debug.md).

Once you have completed the communication or at the end of your program, you
should free the resources with the common function, [modbus_free](modbus_free.md)

## Reads and writes from the client

The Modbus protocol defines different data types and functions to read and write
them from/to remote devices. The following functions are used by the clients to
send Modbus requests:

To read data:

- [modbus_read_bits](modbus_read_bits.md)
- [modbus_read_input_bits](modbus_read_input_bits.md)
- [modbus_read_registers](modbus_read_registers.md)
- [modbus_read_input_registers](modbus_read_input_registers.md)
- [modbus_report_slave_id](modbus_report_slave_id.md)

To write data:

- [modbus_write_bit](modbus_write_bit.md)
- [modbus_write_register](modbus_write_register.md)
- [modbus_write_bits](modbus_write_bits.md)
- [modbus_write_registers](modbus_write_registers.md)

To write and read data in a single operation:

- [modbus_write_and_read_registers](modbus_write_and_read_registers.md)

To send and receive low-level requests:

- [modbus_send_raw_request](modbus_send_raw_request.md)
- [modbus_receive_confirmation](modbus_receive_confirmation.md)

To reply to an exception:

- [modbus_reply_exception](modbus_reply_exception.md)

## Handling requests from server

The server is waiting for request from clients and must answer when it is
concerned by the request. The libmodbus offers the following functions to
handle requests:

Data mapping:

- [modbus_mapping_new](modbus_mapping_new.md)
- [modbus_mapping_free](modbus_mapping_free.md)

Receive:

- [modbus_receive](modbus_receive.md)

Reply:

- [modbus_reply](modbus_reply.md)
- [modbus_reply_exception](modbus_reply_exception.md)

## Advanced functions

Timeout settings:

- [modbus_get_byte_timeout](modbus_get_byte_timeout.md)
- [modbus_set_byte_timeout](modbus_set_byte_timeout.md)
- [modbus_get_response_timeout](modbus_get_response_timeout.md)
- [modbus_set_response_timeout](modbus_set_response_timeout.md)

Error recovery mode:

- [modbus_set_error_recovery](modbus_set_error_recovery.md)

Setter/getter of internal socket:

- [modbus_set_socket](modbus_set_socket.md)
- [modbus_get_socket](modbus_get_socket.md)

Information about header:

- [modbus_get_header_length](modbus_get_header_length.md)

## Data handling

Macros for data manipulation:

- `MODBUS_GET_HIGH_BYTE(data)`, extracts the high byte from a byte
- `MODBUS_GET_LOW_BYTE(data)`, extracts the low byte from a byte
- `MODBUS_GET_INT64_FROM_INT16(tab_int16, index)`, builds an int64 from the four first int16 starting at tab_int16[index]
- `MODBUS_GET_INT32_FROM_INT16(tab_int16, index)`, builds an int32 from the two first int16 starting at tab_int16[index]
- `MODBUS_GET_INT16_FROM_INT8(tab_int8, index)`, builds an int16 from the two first int8 starting at tab_int8[index]
- `MODBUS_SET_INT16_TO_INT8(tab_int8, index, value)`, set an int16 value into the two first bytes starting at tab_int8[index]
- `MODBUS_SET_INT32_TO_INT16(tab_int16, index, value)`, set an int32 value into the two first int16 starting at tab_int16[index]
- `MODBUS_SET_INT64_TO_INT16(tab_int16, index, value)`, set an int64 value into the four first int16 starting at tab_int16[index]

Handling of bits and bytes:

- [modbus_set_bits_from_byte](modbus_set_bits_from_byte.md)
- [modbus_set_bits_from_bytes](modbus_set_bits_from_bytes.md)
- [modbus_get_byte_from_bits](modbus_get_byte_from_bits.md)

Set or get float numbers:

- [modbus_get_float_abcd](modbus_get_float_abcd.md)
- [modbus_set_float_abcd](modbus_set_float_abcd.md)
- [modbus_get_float_badc](modbus_get_float_badc.md)
- [modbus_set_float_badc](modbus_set_float_badc.md)
- [modbus_get_float_cdab](modbus_get_float_cdab.md)
- [modbus_set_float_cdab](modbus_set_float_cdab.md)
- [modbus_get_float_dcba](modbus_get_float_dcba.md)
- [modbus_set_float_dcba](modbus_set_float_dcba.md)
- [modbus_get_float](modbus_get_float.md) **deprecated**
- [modbus_set_float](modbus_set_float.md) **deprecated**

## Error handling

The libmodbus functions handle errors using the standard conventions found on
POSIX systems. Generally, this means that upon failure a libmodbus function
shall return either a NULL value (if returning a pointer) or a negative value
(if returning an integer), and the actual error code shall be stored in the
`errno` variable.

The *modbus_strerror()* function is provided to translate libmodbus-specific
error codes into error message strings; for details refer to
[modbus_strerror](modbus_strerror.md).

## Miscellaneous

To deviate from the Modbus standard, you can enable or disable quirks with:

- [modbus_disable_quirks](modbus_disable_quirks.md)
- [modbus_enable_quirks](modbus_enable_quirks.md)

The `_LIBMODBUS_VERSION_STRING_` constant indicates the libmodbus version the
program has been compiled against. The variables 'libmodbus_version_major',
'libmodbus_version_minor', 'libmodbus_version_micro' give the version the
program is linked against.

## Copying

Free use of this software is granted under the terms of the GNU Lesser General
Public License (LGPL v2.1+). For details see the file `COPYING.LESSER` included
with the libmodbus distribution.
