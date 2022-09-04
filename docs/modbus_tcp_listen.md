# modbus_tcp_listen

## Name

modbus_tcp_listen - create and listen a TCP Modbus socket (IPv4)


## Synopsis

```c
int modbus_tcp_listen(modbus_t *ctx, int nb_connection);
```

## Description

The *modbus_tcp_listen()* function shall create a socket and listen to maximum
`nb_connection` incoming connections on the specified IP address.  The context
`ctx` must be allocated and initialized with [modbus_new_tcp](modbus_new_tcp.md) before to
set the IP address to listen, if IP address is set to NULL or '0.0.0.0', any addresses will be
listen.

## Return value

The function shall return a new socket if successful. Otherwise it shall return
-1 and set errno.

## Example

For detailed examples, see source files in tests directory:

- unit-test-server.c, simple but handle only one connection
- bandwidth-server-many-up.c, handles several connections at once

```c
...

/* To listen any addresses on port 502 */
ctx = modbus_new_tcp(NULL, 502);

/* Handle until 10 established connections */
server_socket = modbus_tcp_listen(ctx, 10);

/* Clear the reference set of socket */
FD_ZERO(&refset);

/* Add the server socket */
FD_SET(server_socket, &refset);

if (select(server_socket + 1, &refset, NULL, NULL, NULL) == -1) {
}

...

close(server_socket);
modbus_free(ctx);
```

## See also

- [modbus_new_tcp](modbus_new_tcp.md)
- [modbus_tcp_accept](modbus_tcp_accept.md)
- [modbus_tcp_pi_listen](modbus_tcp_pi_listen.md)
