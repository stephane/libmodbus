# modbus_new_tcp_pi

## Name

modbus_new_tcp_pi - create a libmodbus context for TCP Protocol Independent

## Synopsis

```c
*modbus_t *modbus_new_tcp_pi(const char *node, const char *service);
```

## Description

The *modbus_new_tcp_pi()* function shall allocate and initialize a modbus_t
structure to communicate with a Modbus TCP IPv4 or IPv6 server.

The `node` argument specifies the host name or IP address of the host to connect
to, eg. "192.168.0.5" , "::1" or "server.com". A NULL value can be used to
listen any addresses in server mode.

The `service` argument is the service name/port number to connect to. To use the
default Modbus port, you can provide an NULL value or the string "502". On many
Unix systems, it’s convenient to use a port number greater than or equal to 1024
because it’s not necessary to have administrator privileges.

:octicons-tag-24: v3.1.8 handles NULL value for `service` (no *EINVAL* error).

## Return value

The function shall return a pointer to a *modbus_t* structure if
successful. Otherwise it shall return NULL and set errno to one of the values
defined below.

## Errors

- *ENOMEM*, out of memory. Possibly, the application hits its memory limit
  and/or whole system is running out of memory.

## Example

```c
modbus_t *ctx;

ctx = modbus_new_tcp_pi("::1", "1502");
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

- [modbus_new_tcp](modbus_new_tcp.md)
- [modbus_tcp_pi_listen](modbus_tcp_pi_listen.md)
- [modbus_free](modbus_free.md)
