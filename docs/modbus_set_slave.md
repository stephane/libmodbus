# modbus_set_slave

## Name

modbus_set_slave - set slave number in the context

## Synopsis

```c
int modbus_set_slave(modbus_t *ctx, int slave);
```

## Description

The *modbus_set_slave()* function shall set the slave number in the libmodbus
context.

It is usually only required to set the slave ID in **RTU**. The meaning of this
ID will be different if your program acts as client (master) or server (slave).

As **RTU client**, *modbus_set_slave()* sets the ID of the remote device you
want to communicate. Be sure to set the slave ID before issuing any Modbus
requests on the serial bus. If you communicate with several servers (slaves),
you can set the slave ID of the remote device before each request.

As **RTU server**, the slave ID allows the various clients to reach your
service. You should use a free ID, once set, this ID should be known by the
clients of the network. According to the protocol, a Modbus device must only
accept message holding its slave number or the special broadcast number.

In **TCP**, the slave number is only required if the message must reach a device
on a serial network. Some not compliant devices or software (such as modpoll)
uses the slave ID as unit identifier, that's incorrect (cf page 23 of Modbus
Messaging Implementation Guide v1.0b) but without the slave value, the faulty
remote device or software drops the requests! The special value
`MODBUS_TCP_SLAVE` (0xFF) can be used in TCP mode to restore the default value.

The broadcast address is `MODBUS_BROADCAST_ADDRESS`. This special value must be
use when you want all Modbus devices of the network receive the request.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno to one of the values defined below.

## Errors

- *EINVAL*, the slave number is invalid.

## Example

```c
modbus_t *ctx;

ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
if (ctx == NULL) {
    fprintf(stderr, "Unable to create the libmodbus context\n");
    return -1;
}

rc = modbus_set_slave(ctx, YOUR_DEVICE_ID);
if (rc == -1) {
    fprintf(stderr, "Invalid slave ID\n");
    modbus_free(ctx);
    return -1;
}

if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}
```

## See also

- [modbus_get_slave](modbus_get_slave.md)
