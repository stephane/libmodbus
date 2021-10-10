# License
Test programs of this directory are provided under BSD license (see associated
LICENSE file).

# Compilation
After installation, you can use pkg-config to compile these tests.
For example, to compile random-test-server run:

gcc random-test-server.c -o random-test-server `pkg-config --libs --cflags libmodbus`

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
