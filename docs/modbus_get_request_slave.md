# modbus_get_request_slave

## Name

modbus_get_request_slave - get the unit identifier addressed by a request

## Synopsis

```c
int modbus_get_request_slave(modbus_t *ctx, const uint8_t *req);
```

## Description

The *modbus_get_request_slave()* function shall return the Modbus unit identifier
(slave) addressed by the request or indication *req*. The identifier is located
just before the function code, at the end of the backend header (offset 0 in RTU,
6 in TCP). This is useful to dispatch an indication to a per-slave data mapping.

## Return value

The *modbus_get_request_slave()* function shall return the unit identifier.
Otherwise it shall return -1 and set errno to EINVAL if *ctx* or *req* is NULL.

## See also

- [modbus_reply_router](modbus_reply_router.md)
- [modbus_receive](modbus_receive.md)
