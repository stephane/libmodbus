# License
Test programs of this directory are provided under BSD license (see associated
LICENSE file).

# Compilation
After installation, you can use pkg-config to compile these tests.
For example, to compile random-test-server run:

`gcc random-test-server.c -o random-test-server $(pkg-config --libs --cflags libmodbus)`

- `random-test-server` is necessary to launch a server before running
random-test-client. By default, it receives and replies to Modbus query on the
localhost and port 1502.

- `random-test-client` sends many different queries to a large range of
addresses and values to test the communication between the client and the
server.

- `unit-test-server` and `unit-test-client` run a full unit test suite. These
programs are essential to test the Modbus protocol implementation and libmodbus
behavior.

- `bandwidth-server-one`, `bandwidth-server-many-up` and `bandwidth-client`
 return very useful information about the performance of transfer rate between
 the server and the client. `bandwidth-server-one` can only handles one
 connection at once with a client whereas `bandwidth-server-many-up` opens a
 connection for each new clients (with a limit).

- `read-device-identification-client` requests the Modbus Encapsulated Interface
 (MEI) function READ DEVICE IDENTIFICATION from a Modbus slave device. The slave
 id must be given on the command line. The Basic Device Identification
 (read_device_id_code = 1) is requested and comprises the ASCII Strings VendorName,
 ProductCode and MajorMinorRevision (see official Modbus Application Protocol
 Specification V1.1b, Chapter 6.21,
 http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf).
