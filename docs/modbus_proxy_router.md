# modbus_proxy_router

## Name

modbus_proxy_router - route a request to the backend serving its unit identifier

## Synopsis

```c
typedef modbus_t *(*modbus_backend_resolver_t)(int slave, void *user);

int modbus_proxy_router(modbus_t *frontend_ctx, const uint8_t *req, int req_length, modbus_backend_resolver_t resolve, void *user);
```

## Description

The *modbus_proxy_router()* function shall forward the request *req* of length
*req_length* received on *frontend_ctx* to the backend context returned by the
*resolve* callback for the addressed unit identifier, and relay the response
back. It is a convenience over [modbus_proxy](modbus_proxy.md) for a gateway that
bridges to several downstream links, for example one serial port per group of
slaves.

The *resolve* callback receives the unit identifier and the opaque *user*
pointer, and shall return the backend *modbus_t* serving that slave, or NULL if
the slave is not routable. When it returns NULL, a gateway path exception
(MODBUS_EXCEPTION_GATEWAY_PATH) is sent to the frontend. Backend failures, such
as a downstream timeout, are reported by *modbus_proxy()* as a gateway target
exception.

## Return value

The *modbus_proxy_router()* function shall return the length of the response
relayed to the frontend if successful. Otherwise it shall return -1 and set
errno. When *resolve* returns NULL, errno is set to EMBXGPATH and a gateway path
exception has been sent to the frontend.

## Example

```c
static modbus_t *route(int slave, void *user)
{
    modbus_t **backends = user; /* indexed by unit id */
    return backends[slave];
}

for (;;) {
    uint8_t req[MODBUS_TCP_MAX_ADU_LENGTH];
    int rc = modbus_receive(frontend, req);
    if (rc > 0) {
        modbus_proxy_router(frontend, req, rc, route, backends);
    }
}
```

## See also

- [modbus_proxy](modbus_proxy.md)
- [modbus_get_request_slave](modbus_get_request_slave.md)
- [modbus_reply_router](modbus_reply_router.md)
