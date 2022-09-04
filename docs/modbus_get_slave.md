# modbus_get_slave

## Name

modbus_get_slave - get slave number in the context

## Synopsis

```c
int modbus_get_slave(modbus_t *ctx);
```

## Description

The *modbus_get_slave()* function shall get the slave number in the libmodbus
context.

## Return value

The function shall return the slave number if successful. Otherwise it shall
return -1 and set errno to one of the values defined below.

## Errors

- *EINVAL*, the libmodbus context is undefined.

## See also

- [modbus_set_slave](modbus_set_slave.md)
