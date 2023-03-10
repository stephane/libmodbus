# modbus_set_indication_timeout

## Name

modbus_set_indication_timeout - set timeout between indications

## Synopsis

```c
void modbus_set_indication_timeout(modbus_t *ctx, uint32_t to_sec, uint32_t to_usec);
```

## Description

The *modbus_set_indication_timeout()* function shall set the timeout interval used by
a server to wait for a request from a client.

The value of `to_usec` argument must be in the range 0 to 999999.

If both `to_sec` and `to_usec` are zero, this timeout will not be used at all.
In this case, the server will wait forever.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno.

## Errors

- *EINVAL*, the argument `ctx` is NULL or `to_usec` is larger than 1000000.

## See also

- [modbus_get_indication_timeout](modbus_get_indication_timeout.md)
- [modbus_get_response_timeout](modbus_get_response_timeout.md)
- [modbus_set_response_timeout](modbus_set_response_timeout.md)
