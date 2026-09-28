# modbus_reply_router

## Name

modbus_reply_router - reply to an indication using a per-slave data mapping

## Synopsis

```c
typedef modbus_mapping_t *(*modbus_mapping_resolver_t)(int slave, void *user);

int modbus_reply_router(modbus_t *ctx, const uint8_t *req, int req_length, modbus_mapping_resolver_t resolve, void *user);
```

## Description

The *modbus_reply_router()* function shall answer the indication *req* of length
*req_length* using the data mapping returned by the *resolve* callback for the
addressed unit identifier. It is a convenience over
[modbus_reply](modbus_reply.md) for a server that handles several slaves on one
context, each backed by its own mapping.

The *resolve* callback receives the unit identifier and the opaque *user* pointer
passed to *modbus_reply_router()*, and shall return the *modbus_mapping_t* serving
that slave, or NULL if the slave is not served. When it returns NULL, a gateway
path exception (MODBUS_EXCEPTION_GATEWAY_PATH) is sent so the client learns the
unit is unavailable.

## Return value

The *modbus_reply_router()* function shall return the length of the response sent
if successful. Otherwise it shall return -1 and set errno.

## Example

```c
static modbus_mapping_t *resolve(int slave, void *user)
{
    modbus_mapping_t **maps = user; /* indexed by unit id */
    return maps[slave];
}

for (;;) {
    uint8_t req[MODBUS_TCP_MAX_ADU_LENGTH];
    int rc = modbus_receive(ctx, req);
    if (rc > 0) {
        modbus_reply_router(ctx, req, rc, resolve, maps);
    }
}
```

## See also

- [modbus_reply](modbus_reply.md)
- [modbus_get_request_slave](modbus_get_request_slave.md)
- [modbus_mapping_new](modbus_mapping_new.md)
- [modbus_receive](modbus_receive.md)
