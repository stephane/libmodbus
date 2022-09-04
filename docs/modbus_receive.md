# modbus_receive

## Name

modbus_receive - receive an indication request

## Synopsis

```c
int modbus_receive(modbus_t *'ctx', uint8_t *'req');
```

## Description

The *modbus_receive()* function shall receive an indication request from the
socket of the context `ctx`. This function is used by Modbus slave/server to
receive and analyze indication request sent by the masters/clients.

If you need to use another socket or file descriptor than the one defined in the
context `ctx`, see the function [modbus_set_socket](modbus_set_socket.md).

## Return value

The function shall store the indication request in `req` and return the request
length if successful. The returned request length can be zero if the indication
request is ignored (eg. a query for another slave in RTU mode). Otherwise it
shall return -1 and set errno.

## See also

- [modbus_set_socket](modbus_set_socket.md)
- [modbus_reply](modbus_reply.md)
