# modbus_rtu_set_custom_rts

## Name

modbus_rtu_set_custom_rts - set a function to be used for custom RTS implementation

## Synopsis

```c
int modbus_rtu_set_custom_rts(modbus_t *ctx, void (*set_rts) (modbus_t *ctx, int on))
```

## Description

The `modbus_rtu_set_custom_rts()` function shall set a custom function to be
called when the RTS pin is to be set before and after a transmission. By default
this is set to an internal function that toggles the RTS pin using an ioctl
call.

Note that this function adheres to the RTS mode, the values `MODBUS_RTU_RTS_UP` or
`MODBUS_RTU_RTS_DOWN` must be used for the function to be called.

This function can only be used with a context using a RTU backend.

## Return value

The `modbus_rtu_set_custom_rts()` function shall return 0 if successful.
Otherwise it shall return -1 and set errno to one of the values defined below.

## Errors

- *EINVAL*, the libmodbus backend is not RTU.
