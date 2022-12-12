# modbus_rtu_set_rts

## Name

modbus_rtu_set_rts - set the RTS mode in RTU

## Synopsis

```c
int modbus_rtu_set_rts(modbus_t *ctx, int mode)
```

## Description

The *modbus_rtu_set_rts()* function shall set the Request To Send mode to
communicate on a RS-485 serial bus. By default, the mode is set to
`MODBUS_RTU_RTS_NONE` and no signal is issued before writing data on the wire.

To enable the RTS mode, the values `MODBUS_RTU_RTS_UP` or `MODBUS_RTU_RTS_DOWN`
must be used, these modes enable the RTS mode and set the polarity at the same
time. When `MODBUS_RTU_RTS_UP` is used, an ioctl call is made with RTS flag
enabled then data is written on the bus after a delay of 1 ms, then another
ioctl call is made with the RTS flag disabled and again a delay of 1 ms occurs.
The `MODBUS_RTU_RTS_DOWN` mode applies the same procedure but with an inverted
RTS flag.

This function can only be used with a context using a RTU backend.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno to one of the values defined below.

## Errors

- *EINVAL*, the libmodbus backend isn't RTU or the mode given in argument is invalid.

## Example

Enable the RTS mode with positive polarity:

```c
modbus_t *ctx;
uint16_t tab_reg[10];

ctx = modbus_new_rtu("/dev/ttyS0", 115200, 'N', 8, 1);
modbus_set_slave(ctx, 1);
modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_UP);

if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

rc = modbus_read_registers(ctx, 0, 7, tab_reg);
if (rc == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return -1;
}

modbus_close(ctx);
modbus_free(ctx);
```

## See also

- [modbus_rtu_get_rts](modbus_rtu_get_rts.md)
