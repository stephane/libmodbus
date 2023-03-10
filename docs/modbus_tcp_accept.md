# modbus_tcp_accept

## Name

modbus_tcp_accept - accept a new connection on a TCP Modbus socket (IPv4)

## Synopsis

```c
int modbus_tcp_accept(modbus_t *ctx, int *s);
```

## Description

The *modbus_tcp_accept()* function shall extract the first connection on the
queue of pending connections, create a new socket and store it in libmodbus
context given in argument. If available, `accept4()` with `SOCK_CLOEXEC` will be
called instead of `accept()`.

## Return value

The function shall return a new socket if successful.
Otherwise it shall return -1 and set errno.

## Example

For detailed example, see unit-test-server.c source file in tests directory.

```c
...

ctx = modbus_new_tcp("127.0.0.1", 502);
s = modbus_tcp_listen(ctx, 1);
modbus_tcp_accept(ctx, &s);

...

close(s)
modbus_free(ctx);
```

## See also

- [modbus_tcp_pi_accept](modbus_tcp_pi_accept.md)
- [modbus_tcp_listen](modbus_tcp_listen.md)
- [modbus_tcp_pi_listen](modbus_tcp_pi_listen.md)
