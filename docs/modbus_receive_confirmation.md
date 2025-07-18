# modbus_receive_confirmation

## Name

modbus_receive_confirmation - receive a confirmation request

## Synopsis

```c
int modbus_receive_confirmation(modbus_t *ctx, uint8_t *rsp);
```

## Description

The *modbus_receive_confirmation()* function shall receive a request via the
socket of the context `ctx`. This function must be used for debugging purposes
because the received response isn't checked against the initial request. This
function can be used to receive request not handled by the library.

The maximum size of the response depends on the used backend, in RTU the `rsp`
array must be `MODBUS_RTU_MAX_ADU_LENGTH` bytes and in TCP it must be
`MODBUS_TCP_MAX_ADU_LENGTH` bytes. If you want to write code compatible with
both, you can use the constant `MODBUS_MAX_ADU_LENGTH` (maximum value of all
libmodbus backends). Take care to allocate enough memory to store responses to
avoid crashes of your server.

## Return value

The function shall store the confirmation request in `rsp` and return the
response length if successful. The returned request length can be zero if the
indication request is ignored (eg. a query for another slave in RTU
mode). Otherwise it shall return -1 and set errno.

## Example

```c
uint8_t rsp[MODBUS_MAX_ADU_LENGTH];
rc = modbus_receive_confirmation(ctx, rsp);
```

## See also

- [modbus_send_raw_request](modbus_send_raw_request.md)
