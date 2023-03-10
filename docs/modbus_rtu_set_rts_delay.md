# modbus_rtu_set_rts_delay

## Name

modbus_rtu_set_rts_delay - set the RTS delay in RTU

## Synopsis

```c
int modbus_rtu_set_rts_delay(modbus_t *ctx, int us);
```

## Description

The `modbus_rtu_set_rts_delay()` function shall set the Request To Send delay
period of the libmodbus context 'ctx'.

This function can only be used with a context using a RTU backend.

## Return value

The `modbus_rtu_set_rts_delay()` function shall return 0 if successful.
Otherwise it shall return -1 and set errno.

## Errors

- *EINVAL*, the libmodbus backend is not RTU or a negative delay was specified.

## See also

- [modbus_rtu_get_rts_delay](modbus_rtu_get_rts_delay.md)
