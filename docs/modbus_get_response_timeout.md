# modbus_get_response_timeout

## Name

modbus_get_response_timeout - get timeout for response

## Synopsis

```c
int modbus_get_response_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);
```

## Description

The *modbus_get_response_timeout()* function shall return the timeout interval
used to wait for a response in the `to_sec` and `to_usec` arguments.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno.

Example:

```c
uint32_t old_response_to_sec;
uint32_t old_response_to_usec;

/* Save original timeout */
modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);

/* Define a new and too short timeout! */
modbus_set_response_timeout(ctx, 0, 0);
```

## See also

- [modbus_set_response_timeout](modbus_set_response_timeout.md)
- [modbus_get_byte_timeout](modbus_get_byte_timeout.md)
- [modbus_set_byte_timeout](modbus_set_byte_timeout.md)
