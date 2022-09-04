# modbus_connect

## Name

modbus_connect - establish a Modbus connection

## Synopsis

```c
int modbus_connect(modbus_t *ctx);
```

## Description

The *modbus_connect()* function shall establish a connection to a Modbus server,
a network or a bus using the context information of libmodbus context given in
argument.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno to one of the values defined by the system calls of the underlying
platform.

## Example

```c
modbus_t *ctx;

ctx = modbus_new_tcp("127.0.0.1", 502);
if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}
```

## See also

- [modbus_close](modbus_close.md)
