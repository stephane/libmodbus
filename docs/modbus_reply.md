# modbus_reply

## Name

modbus_reply - send a response to the received request

## Synopsis

```c
int modbus_reply(modbus_t *ctx, const uint8_t *req, int req_length, modbus_mapping_t *mb_mapping);
```

## Description

The *modbus_reply()* function shall send a response to received request. The
request `req` given in argument is analyzed, a response is then built and sent
by using the information of the modbus context `ctx`.

If the request indicates to read or write a value the operation will done in the
modbus mapping `mb_mapping` according to the type of the manipulated data.

If an error occurs, an exception response will be sent.

This function is designed for Modbus server.

## Return value

The function shall return the length of the response sent if
successful. Otherwise it shall return -1 and set errno.

## Errors

- *EMBMDATA*, sending has failed

See also the errors returned by the syscall used to send the response (eg. send or write).

## See also

- [modbus_reply_exception](modbus_reply_exception.md)
