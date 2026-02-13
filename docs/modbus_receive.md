# modbus_receive

## Name

modbus_receive - receive an indication request

## Synopsis

```c
int modbus_receive(modbus_t *ctx, uint8_t *req);
```

## Description

The *modbus_receive()* function shall receive an indication request from the
socket of the context `ctx`. This function is used by a Modbus slave/server to
receive and analyze indication request sent by the masters/clients.

If you need to use another socket or file descriptor than the one defined in the
context `ctx`, see the function [modbus_set_socket](modbus_set_socket.md).

## Return value

The function shall store the indication request in `req` and return the request
length if successful. The returned request length can be zero if the indication
request is ignored (eg. a query for another slave in RTU mode). Otherwise it
shall return -1 and set errno.

## Buffer size

The maximum size of the request depends on the used backend, in RTU the `req`
array must be `MODBUS_RTU_MAX_ADU_LENGTH` bytes and in TCP it must be
`MODBUS_TCP_MAX_ADU_LENGTH` bytes. If you want to write code compatible with
both, you can use the constant `MODBUS_MAX_ADU_LENGTH` (maximum value of all
libmodbus backends). Take care to allocate enough memory to store requests to
avoid crashes of your server.

## Example

```c
uint8_t req[MODBUS_MAX_ADU_LENGTH];
rc = modbus_receive(ctx, req);
```

## See also

- [modbus_set_socket](modbus_set_socket.md)
- [modbus_reply](modbus_reply.md)
