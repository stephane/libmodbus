# modbus_new_rtu

## Name

modbus_new_rtu - create a libmodbus context for RTU

## Synopsis

```c
modbus_t *modbus_new_rtu(const char *device, int baud, char parity, int data_bit, int stop_bit);
```

## Description

The *modbus_new_rtu()* function shall allocate and initialize a `modbus_t`
structure to communicate in RTU mode on a serial line.

The `device` argument specifies the name of the serial port handled by the OS,
eg. "/dev/ttyS0" or "/dev/ttyUSB0". On Windows, it's necessary to prepend COM
name with "\\.\" for COM number greater than 9, eg. "\\\\.\\COM10". See
http://msdn.microsoft.com/en-us/library/aa365247(v=vs.85).aspx for details

The `baud` argument specifies the baud rate of the communication, eg. 9600,
19200, 57600, 115200, etc.

The `parity` argument can have one of the following values:

- `N` for none
- `E` for even
- `O` for odd

The `data_bits` argument specifies the number of bits of data, the allowed
values are 5, 6, 7 and 8.

The `stop_bits` argument specifies the bits of stop, the allowed values are 1
and 2.

Once the `modbus_t` structure is initialized, you can connect to the serial bus
with [modbus_connect](modbus_connect.md).

In RTU, your program can act as server or client:

- **server** is called *slave* in Modbus terminology, your program will expose
  data to the network by processing and answering the requests of one of several
  clients. It up to you to define the slave ID of your service with
  [modbus_set_slave](modbus_set_slave.md), this ID should be used by the client
  to communicate with your program.

- **client** is called *master* in Modbus terminology, your program will send
  requests to servers to read or write data from them. Before issuing the
  requests, you should define the slave ID of the remote device with
  [modbus_set_slave](modbus_set_slave.md). The slave ID is not an argument of
  the read/write functions because it's very frequent to talk with only one
  server so you can set it once and for all. The slave ID it not used in TCP
  communications so this way the API is common to both.

## Return value

The function shall return a pointer to a `modbus_t` structure if
successful. Otherwise it shall return NULL and set errno to one of the values
defined below.

## Errors

- *EINVAL*, an invalid argument was given.
- *ENOMEM*, out of memory. Possibly, the application hits its memory limit
    and/or whole system is running out of memory.

## Example

In this example, the program will open a serial communication on USB. All
subsequent calls such as read or write of registers will be sent on the wire and
the request will be visible to all connected devices. According to the Modbus
protocol, only the master associated to slave ID 10 will process and answer your
requests.

```c
const int REMOTE_ID = 10;
modbus_t *ctx;
uint16_t tab_reg[10];

ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
if (ctx == NULL) {
    fprintf(stderr, "Unable to create the libmodbus context\n");
    return -1;
}

if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

modbus_set_slave(ctx, REMOTE_ID);

// Read 2 registers from address 0 of server ID 10.
modbus_read_registers(ctx, 0, 2, tab_reg);
```

## See also

- [modbus_new_tcp](modbus_new_tcp.md)
- [modbus_free](modbus_free.md)
