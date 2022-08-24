# modbus_set_client_context


## Name

`modbus_set_client_context` - set context information (usually a pointer) for use
by the client


## Synopsis

```c
int modbus_set_client_context(modbus_t *ctx, void *cctx)
```


## Description

The `modbus_set_client_context()` function can be used to set arbitrary client
context information. The actual value is not used by libmodbus, but instead
is available to client callback functions via `modbus_get_client_context()`.

Do not be confused by the overloaded use of “client” and “context” in these
functions. Here, “client” refers to the code that makes use of libmodbus and
“client context” is the data used by the client code.

## Return Value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno to one of the values defined below.


## Errors

- *EINVAL*, The libmodbus context is NULL.


## Example

```cpp
modbus_t *ctx;
uint16_t tab_reg[10];
struct ClientContext {
    int clientData1;
    int clientData2;
};
ClientContext *cctx = new ClientContext();

ctx = modbus_new_rtu("/dev/ttyS0", 115200, 'N', 8, 1);
modbus_set_slave(ctx, 1);
modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
modbus_set_client_context(ctx, cctx);

ClientContext *myCCTX = reinterpret_cast<ClientContext*>(modbus_get_client_context(ctx));

modbus_free(ctx);
```

## See also

[`modbus_get_client_context()`](modbus_get_client_context)
