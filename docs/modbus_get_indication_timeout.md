# modbus_get_indication_timeout

## Name

modbus_get_indication_timeout - get timeout used to wait for an indication (request received by a server).

## Synopsis

```c
int modbus_get_indication_timeout(modbus_t *ctx, uint32_t *to_sec, uint32_t *to_usec);
```

## Description

The *modbus_get_indication_timeout()* function shall store the timeout interval
used to wait for an indication in the `to_sec` and `to_usec` arguments.
Indication is the term used by the Modbus protocol to designate a request
received by the server.

The default value is zero, it means the server will wait forever.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno.

```c
uint32_t to_sec;
uint32_t to_usec;

/* Save original timeout */
modbus_get_indication_timeout(ctx, &to_sec, &to_usec);
```

## See also

- [modbus_set_indication_timeout](modbus_set_indication_timeout.md)
- [modbus_get_response_timeout](modbus_get_response_timeout.md)
- [modbus_set_response_timeout](modbus_set_response_timeout.md)
