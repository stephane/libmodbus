# modbus_get_transport

## Name

modbus_get_transport - get the registered I/O transport

## Synopsis

```c
modbus_transport_t *modbus_get_transport(modbus_t *ctx);
```

## Description

The *modbus_get_transport()* function shall return the pluggable I/O transport
registered on the libmodbus context *ctx* with *modbus_set_transport()*, or NULL
if no transport is set.

## Return value

The function shall return the registered transport, or NULL if none is set.

## See also

- [modbus_set_transport](modbus_set_transport.md)
