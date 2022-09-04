# modbus_set_socket

## Name

modbus_set_socket - set socket of the context

## Synopsis

```c
int modbus_set_socket(modbus_t *ctx, int s);
```

## Description

The *modbus_set_socket()* function shall set the socket or file descriptor in
the libmodbus context. This function is useful for managing multiple client
connections to the same server.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set errno.

## Example

```c
ctx = modbus_new_tcp("127.0.0.1", 1502);
server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);

FD_ZERO(&rdset);
FD_SET(server_socket, &rdset);

/* .... */

if (FD_ISSET(master_socket, &rdset)) {
    modbus_set_socket(ctx, master_socket);
    rc = modbus_receive(ctx, query);
    if (rc != -1) {
        modbus_reply(ctx, query, rc, mb_mapping);
    }
}
```

## See also

- [modbus_get_socket](modbus_get_socket.md)
