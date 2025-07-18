# modbus_get_socket

## Name

modbus_get_socket - get the current socket of the context

## Synopsis

```c
int modbus_get_socket(modbus_t *'ctx');
```

## Description

The *modbus_get_socket()* function shall return the current socket or file
descriptor of the libmodbus context.

## Return value

The function returns the current socket or file descriptor of the context if
successful. Otherwise it shall return -1 and set errno.

## See also

- [modbus_set_socket](modbus_set_socket.md)
