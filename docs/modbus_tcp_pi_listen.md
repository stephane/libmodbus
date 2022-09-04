# modbus_tcp_pi_listen

## Name

modbus_tcp_pi_listen - create and listen a TCP PI Modbus socket (IPv6)

## Synopsis

```c
int modbus_tcp_pi_listen(modbus_t *ctx, int nb_connection);
```

## Description

The *modbus_tcp_pi_listen()* function shall create a socket and listen to
maximum `nb_connection` incoming connections on the specified nodes.  The
context *ctx* must be allocated and initialized with [modbus_new_tcp_pi](modbus_new_tcp_pi.md)
before to set the node to listen, if node is set to NULL or '0.0.0.0', any addresses will be
listen.

## Return value

The function shall return a new socket if successful. Otherwise it shall return
-1 and set errno.

## Example

For detailed examples, see source files in tests directory:

- unit-test-server.c, simple but handle only one connection

```c
...

ctx = modbus_new_tcp_pi("::0", "502");
s = modbus_tcp_pi_listen(ctx, 1);
modbus_tcp_pi_accept(ctx, &s);

for (;;) {
    rc = modbus_receive(ctx, query);
    modbus_replay(ctx, query, rc, mb_mapping);
}
...

modbus_close(s);
modbus_free(ctx);
```

- bandwidth-server-many-up.c, handles several connections at once

## See also

- [modbus_new_tcp_pi](modbus_new_tcp_pi.md)
- [modbus_tcp_pi_accept](modbus_tcp_pi_accept.md)
- [modbus_tcp_listen](modbus_tcp_listen.md)
