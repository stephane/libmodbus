# modbus_get_byte_timeout

## Name

modbus_get_byte_timeout - get timeout between bytes

## Synopsis

```c
int modbus_get_byte_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);
```

## Description

The *modbus_get_byte_timeout()* function shall store the timeout interval
between two consecutive bytes of the same message in the `to_sec` and `to_usec`
arguments.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno.

## Example

```c
uint32_t to_sec;
uint32_t to_usec;

/* Save original timeout */
modbus_get_byte_timeout(ctx, &to_sec, &to_usec);
```

## See also

- [modbus_set_byte_timeout](modbus_set_byte_timeout.md)
- [modbus_get_response_timeout](modbus_get_response_timeout.md)
- [modbus_set_response_timeout](modbus_set_response_timeout.md)
