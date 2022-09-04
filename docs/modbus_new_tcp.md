# modbus_new_tcp

## Name

modbus_new_tcp - create a libmodbus context for TCP/IPv4

## Synopsis

```c
modbus_t *modbus_new_tcp(const char *ip, int port);
```

## Description

The *modbus_new_tcp()* function shall allocate and initialize a modbus_t
structure to communicate with a Modbus TCP IPv4 server.

The `ip` argument specifies the IP address of the server to which the client
wants to establish a connection. A NULL value can be used to listen any addresses in
server mode.

The `port` argument is the TCP port to use. Set the port to
`MODBUS_TCP_DEFAULT_PORT` to use the default one (502). It’s convenient to use a
port number greater than or equal to 1024 because it’s not necessary to have
administrator privileges.

## Return value

The function shall return a pointer to a *modbus_t* structure if
successful. Otherwise it shall return NULL and set errno to one of the values
defined below.

## Errors

- *EINVAL*, an invalid IP address was given.
- *ENOMEM*, out of memory. Possibly, the application hits its memory limit
  and/or whole system is running out of memory.

## Example

```c
modbus_t *ctx;

ctx = modbus_new_tcp("127.0.0.1", 1502);
if (ctx == NULL) {
    fprintf(stderr, "Unable to allocate libmodbus context\n");
    return -1;
}

if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}
```

## See also

- [modbus_tcp_listen](modbus_tcp_listen.md)
- [modbus_free](modbus_free.md)
