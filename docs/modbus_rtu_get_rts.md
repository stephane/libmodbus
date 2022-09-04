# modbus_rtu_get_rts

## Name

modbus_rtu_get_rts - get the current RTS mode in RTU

## Synopsis

```c
int modbus_rtu_get_rts(modbus_t *ctx);
```

## Description

The *modbus_rtu_get_rts()* function shall get the current Request To Send mode
of the libmodbus context `ctx`. The possible returned values are:

- `MODBUS_RTU_RTS_NONE`
- `MODBUS_RTU_RTS_UP`
- `MODBUS_RTU_RTS_DOWN`

This function can only be used with a context using a RTU backend.

## Return value

The function shall return the current RTS mode if successful. Otherwise it shall
return -1 and set errno.

## Errors

- *EINVAL*, the libmodbus backend is not RTU.

## See also

- [modbus_rtu_set_rts](modbus_rtu_set_rts.md)
