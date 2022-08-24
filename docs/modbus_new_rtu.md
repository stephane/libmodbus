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

Once the `modbus_t` structure is initialized, you must set the slave of your
device with [modbus_set_slave](modbus_set_slave) and connect to the serial bus with
[modbus_connect](modbus_connect).

## Return value

The function shall return a pointer to a `modbus_t` structure if
successful. Otherwise it shall return NULL and set errno to one of the values
defined below.

## Errors

- *EINVAL*, an invalid argument was given.
- *ENOMEM*, out of memory. Possibly, the application hits its memory limit
    and/or whole system is running out of memory.

## Example

```c
modbus_t *ctx;

ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
if (ctx == NULL) {
    fprintf(stderr, "Unable to create the libmodbus context\n");
    return -1;
}

modbus_set_slave(ctx, YOUR_DEVICE_ID);

if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}
```

## See also

- [modbus_new_tcp](modbus_new_tcp)
- [modbus_free](modbus_free)
