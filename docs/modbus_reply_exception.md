# modbus_reply_exception

## Name

modbus_reply_exception - send an exception response

## Synopsis

```c
int modbus_reply_exception(modbus_t *ctx, const uint8_t *req, unsigned int exception_code);
```

## Description

The *modbus_reply_exception()* function shall send an exception response based
on the 'exception_code' in argument.

The libmodbus provides the following exception codes:

- MODBUS_EXCEPTION_ILLEGAL_FUNCTION (1)
- MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS (2)
- MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE (3)
- MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE (4)
- MODBUS_EXCEPTION_ACKNOWLEDGE (5)
- MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY (6)
- MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE (7)
- MODBUS_EXCEPTION_MEMORY_PARITY (8)
- MODBUS_EXCEPTION_NOT_DEFINED (9)
- MODBUS_EXCEPTION_GATEWAY_PATH (10)
- MODBUS_EXCEPTION_GATEWAY_TARGET (11)

The initial request `req` is required to build a valid response.

## Return value

The function shall return the length of the response sent if
successful. Otherwise it shall return -1 and set errno.

## Errors

- *EINVAL*, the exception code is invalid

## See also

- [modbus_reply](modbus_reply.md)
