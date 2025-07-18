# modbus_close

## Name

modbus_close - close a Modbus connection

## Synopsis

```c
void modbus_close(modbus_t *ctx);
```

## Description

The *modbus_close()* function shall close the connection established with the
backend set in the context.

## Return value

There is no return value.

## Example

```c
modbus_t *ctx;

ctx = modbus_new_tcp("127.0.0.1", 502);
if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

modbus_close(ctx);
modbus_free(ctx);
```

## See also

- [modbus_connect](modbus_connect.md)
