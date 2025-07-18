# modbus_set_response_timeout

## Name

modbus_set_response_timeout - set timeout for response

## Synopsis

```c
int modbus_set_response_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);
```

## Description

The *modbus_set_response_timeout()* function shall set the timeout interval used
to wait for a response. When a byte timeout is set, if elapsed time for the
first byte of response is longer than the given timeout, an `ETIMEDOUT` error
will be raised by the function waiting for a response. When byte timeout is
disabled, the full confirmation response must be received before expiration of
the response timeout.

The value of `to_usec` argument must be in the range 0 to 999999.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno.

## Errors

- *EINVAL*, the argument `ctx` is NULL, or both `to_sec` and `to_usec` are zero,
  or `to_usec` is larger than 999999.

## Example

```c
uint32_t old_response_to_sec;
uint32_t old_response_to_usec;

/* Save original timeout */
modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);

/* Define a new timeout of 200ms */
modbus_set_response_timeout(ctx, 0, 200000);
```

## See also

- [modbus_get_response_timeout](modbus_get_response_timeout.md)
- [modbus_get_byte_timeout](modbus_get_byte_timeout.md)
- [modbus_set_byte_timeout](modbus_set_byte_timeout.md)
