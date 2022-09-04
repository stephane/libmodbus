# modbus_set_byte_timeout

## Name

modbus_set_byte_timeout - set timeout between bytes

## Synopsis

```c
void modbus_set_byte_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);
```

## Description

The *modbus_set_byte_timeout()* function shall set the timeout interval between
two consecutive bytes of the same message. The timeout is an upper bound on the
amount of time elapsed before *select()* returns, if the time elapsed is longer
than the defined timeout, an `ETIMEDOUT` error will be raised by the
function waiting for a response.

The value of `to_usec` argument must be in the range 0 to 999999.

If both `to_sec` and `to_usec` are zero, this timeout will not be used at all.
In this case, *modbus_set_response_timeout()* governs the entire handling of the
response, the full confirmation response must be received before expiration of
the response timeout. When a byte timeout is set, the response timeout is only
used to wait for until the first byte of the response.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno.

## Errors

- *EINVAL*, The argument `ctx` is NULL or `to_usec` is larger than 999999.

## See also

- [modbus_get_byte_timeout](modbus_get_byte_timeout.md)
- [modbus_get_response_timeout](modbus_get_response_timeout.md)
- [modbus_set_response_timeout](modbus_set_response_timeout.md)
w
