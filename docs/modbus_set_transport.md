# modbus_set_transport

## Name

modbus_set_transport - register a pluggable I/O transport

## Synopsis

```c
int modbus_set_transport(modbus_t *ctx, modbus_transport_t *transport);
```

## Description

The *modbus_set_transport()* function shall register a custom I/O *transport* on
the libmodbus context *ctx*. The transport replaces the low-level send, recv,
select, connect, close and flush calls while leaving the Modbus framing, CRC and
protocol logic unchanged. It is useful to route Modbus over a custom or
userspace IP stack, to drive a loopback for simulation, or to instrument the I/O
path in tests.

This function must be called after *modbus_new_tcp()* or *modbus_new_rtu()* and
before *modbus_connect()*. The context takes ownership of the transport:
*modbus_free()* shall call its *free()* member if non-NULL. Passing NULL detaches
a previously registered transport without calling *free()*, so a transport may be
stack-allocated or shared.

The transport is described by the *modbus_transport_t* structure. Every function
pointer is optional; a NULL member falls back to the default backend behaviour
for that operation.

```c
typedef struct modbus_transport {
    int     (*connect)(struct modbus_transport *t);
    ssize_t (*send)(struct modbus_transport *t, const uint8_t *buf, int len);
    ssize_t (*recv)(struct modbus_transport *t, uint8_t *buf, int len);
    int     (*select)(struct modbus_transport *t, struct timeval *tv);
    int     (*flush)(struct modbus_transport *t);
    void    (*close)(struct modbus_transport *t);
    void    (*free)(struct modbus_transport *t);
    void   *priv;      /* private transport state, not touched by libmodbus */
    int     connected; /* managed by modbus_connect()/modbus_close() */
} modbus_transport_t;
```

The *connect()* member is called by *modbus_connect()*, *send()* transmits a
fully framed ADU, *recv()* reads up to *len* bytes and returns the number read or
0 when the peer closes, *select()* waits until data can be read or the timeout
*tv* expires (NULL waits indefinitely) and returns a positive value when data is
available or 0 on timeout, *flush()* discards pending input, *close()* tears the
connection down without freeing the struct, and *free()* releases all resources.
On error, members return -1 and set errno. With *MODBUS_ERROR_RECOVERY_LINK* (see
[modbus_set_error_recovery](modbus_set_error_recovery.md)), that errno decides
the recovery on every platform: errors such as ECONNRESET or EBADF mean the link
is lost, so libmodbus calls *close()* and *connect()* before retrying.
Connection parameters such as the address and port must be stored in *priv*
before the transport is registered, since the members receive only the
transport pointer.

## Return value

The *modbus_set_transport()* function shall return 0 if successful. Otherwise it
shall return -1 and set errno to EINVAL if *ctx* is NULL.

## Example

```c
typedef struct {
    mystack_conn_t *conn;
} my_priv_t;

static int my_connect(modbus_transport_t *t)
{
    my_priv_t *p = t->priv;
    p->conn = mystack_connect("192.168.1.10", 502);
    return p->conn ? 0 : -1;
}

static ssize_t my_send(modbus_transport_t *t, const uint8_t *buf, int len)
{
    return mystack_send(((my_priv_t *) t->priv)->conn, buf, len);
}

static ssize_t my_recv(modbus_transport_t *t, uint8_t *buf, int len)
{
    return mystack_recv(((my_priv_t *) t->priv)->conn, buf, len);
}

static int my_select(modbus_transport_t *t, struct timeval *tv)
{
    return mystack_wait_rx(((my_priv_t *) t->priv)->conn, tv);
}

static void my_close(modbus_transport_t *t)
{
    mystack_close(((my_priv_t *) t->priv)->conn);
}

my_priv_t priv = {0};
modbus_transport_t tr = {
    .connect = my_connect,
    .send = my_send,
    .recv = my_recv,
    .select = my_select,
    .close = my_close,
    .priv = &priv,
};

ctx = modbus_new_tcp("192.168.1.10", 502);
modbus_set_transport(ctx, &tr);
modbus_connect(ctx);

modbus_read_registers(ctx, 0, 10, regs);

modbus_close(ctx);
modbus_free(ctx);
```

## See also

- [modbus_get_transport](modbus_get_transport.md)
- [modbus_new_tcp](modbus_new_tcp.md)
- [modbus_new_rtu](modbus_new_rtu.md)
- [modbus_connect](modbus_connect.md)
- [modbus_set_socket](modbus_set_socket.md)
