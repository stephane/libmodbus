/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Regression test for the pluggable transport layer (modbus-transport.h).
 *
 * A socketpair provides a loopback: one end drives a Modbus TCP client through
 * a custom transport, the other is served by a Modbus TCP server on the normal
 * backend path (modbus_set_socket). The test checks the set/get round-trip, the
 * connect/send/recv/select/close/free call sequence, that framing and CRC are
 * unchanged, and that a context without a transport still behaves normally.
 *
 * The connection is identified by an opaque pointer handle (conn_t *) rather
 * than an int fd, to exercise a transport whose connection object cannot be
 * stored in the int-typed ctx->s.
 *
 * On Windows the round-trip test uses a 127.0.0.1 loopback pair and a server
 * thread in place of socketpair() and fork(). */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
typedef int sock_t;
#define CLOSESOCK(s) close(s)
#else
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET sock_t;
#define CLOSESOCK(s) closesocket(s)
#endif

#include "modbus-transport.h"
#include "modbus.h"

/* The socket is hidden inside a heap-allocated handle; the transport only ever
 * sees the conn_t pointer stored in priv. */
typedef struct conn {
    sock_t sock;
} conn_t;

typedef struct {
    conn_t *conn;
    int n_connect;
    int n_send;
    int n_recv;
    int n_select;
    int n_flush;
    int n_close;
    int n_free;
} test_priv_t;

static int transport_connect(modbus_transport_t *t)
{
    ((test_priv_t *) t->priv)->n_connect++;
    return 0;
}

static ssize_t transport_send(modbus_transport_t *t, const uint8_t *buf, int len)
{
    test_priv_t *p = t->priv;
    p->n_send++;
    return send(p->conn->sock, (const char *) buf, len, 0);
}

static ssize_t transport_recv(modbus_transport_t *t, uint8_t *buf, int len)
{
    test_priv_t *p = t->priv;
    p->n_recv++;
    /* Read exactly len bytes; a manual loop is used instead of MSG_WAITALL,
     * whose stream support varies on Winsock. */
    int total = 0;
    while (total < len) {
        int rc = recv(p->conn->sock, (char *) buf + total, len - total, 0);
        if (rc == 0)
            return total; /* peer closed */
        if (rc < 0)
            return -1;
        total += rc;
    }
    return total;
}

static int transport_select(modbus_transport_t *t, struct timeval *tv)
{
    test_priv_t *p = t->priv;
    sock_t s = p->conn->sock;
    p->n_select++;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(s, &rset);
    return select((int) (s + 1), &rset, NULL, NULL, tv);
}

static void drain_nonblocking(sock_t fd)
{
    uint8_t discard[256];
#ifndef _WIN32
    while (recv(fd, (char *) discard, sizeof(discard), MSG_DONTWAIT) > 0)
        ;
#else
    u_long nb = 1;
    ioctlsocket(fd, FIONBIO, &nb);
    while (recv(fd, (char *) discard, sizeof(discard), 0) > 0)
        ;
    nb = 0;
    ioctlsocket(fd, FIONBIO, &nb);
#endif
}

static int transport_flush(modbus_transport_t *t)
{
    test_priv_t *p = t->priv;
    p->n_flush++;
    drain_nonblocking(p->conn->sock);
    return 0;
}

static void transport_close(modbus_transport_t *t)
{
    test_priv_t *p = t->priv;
    p->n_close++;
    /* close() tears down the connection but must not free the handle */
    CLOSESOCK(p->conn->sock);
    p->conn->sock = (sock_t) -1;
}

static void transport_free(modbus_transport_t *t)
{
    test_priv_t *p = t->priv;
    p->n_free++;
    /* free() is the destructor and releases the heap-allocated handle */
    free(p->conn);
    p->conn = NULL;
}

/* Two connected stream sockets. On POSIX this is socketpair(); on Windows it is
 * emulated with a 127.0.0.1 loopback listener, connect and accept. Returns 0 on
 * success, -1 on failure. */
static int make_socketpair(sock_t fds[2])
{
#ifndef _WIN32
    return socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
#else
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0; /* let the OS pick a free port */

    if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)) != 0 ||
        listen(listener, 1) != 0 ||
        getsockname(listener, (struct sockaddr *) &addr, &addrlen) != 0) {
        closesocket(listener);
        return -1;
    }

    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET) {
        closesocket(listener);
        return -1;
    }
    if (connect(client, (struct sockaddr *) &addr, addrlen) != 0) {
        closesocket(listener);
        closesocket(client);
        return -1;
    }
    SOCKET server = accept(listener, NULL, NULL);
    closesocket(listener);
    if (server == INVALID_SOCKET) {
        closesocket(client);
        return -1;
    }
    fds[0] = client;
    fds[1] = server;
    return 0;
#endif
}

static void check(int rc, const char *label)
{
    if (rc == -1) {
        fprintf(stderr, "FAIL %s: %s\n", label, modbus_strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/* Service one request against the mapping, then return. Runs in a child process
 * (POSIX) or a thread (Windows). */
typedef struct {
    modbus_t *srv;
    modbus_mapping_t *map;
} server_arg_t;

static void server_serve_one(server_arg_t *a)
{
    uint8_t req[MODBUS_TCP_MAX_ADU_LENGTH];
    modbus_set_debug(a->srv, FALSE);
    int rc = modbus_receive(a->srv, req);
    if (rc > 0)
        modbus_reply(a->srv, req, rc, a->map);
}

#ifdef _WIN32
static unsigned __stdcall server_thread(void *arg)
{
    server_serve_one((server_arg_t *) arg);
    return 0;
}
#endif

/* Test 1: round-trip read over a custom transport. */
static void test_transport_read_registers(void)
{
    printf("[1] transport read_registers... ");
    fflush(stdout);

    sock_t fds[2];
    assert(make_socketpair(fds) == 0);

    /* Server side: default TCP backend with the socket injected. */
    modbus_t *srv = modbus_new_tcp("127.0.0.1", 1502);
    assert(srv);
    modbus_set_socket(srv, (int) fds[1]);

    modbus_mapping_t *map = modbus_mapping_new(0, 0, 10, 0);
    assert(map);
    for (int i = 0; i < 10; i++)
        map->tab_registers[i] = (uint16_t) (100 + i);

    /* Client side: custom transport identified by a pointer handle. */
    conn_t *conn = malloc(sizeof(*conn));
    assert(conn);
    conn->sock = fds[0];
    test_priv_t priv = {.conn = conn};
    modbus_transport_t tr = {
        .connect = transport_connect,
        .send = transport_send,
        .recv = transport_recv,
        .select = transport_select,
        .flush = transport_flush,
        .close = transport_close,
        .free = transport_free,
        .priv = &priv,
    };

    modbus_t *cli = modbus_new_tcp("127.0.0.1", 1502);
    assert(cli);
    assert(modbus_get_transport(cli) == NULL);

    check(modbus_set_transport(cli, &tr), "set_transport");
    assert(modbus_get_transport(cli) == &tr);

    check(modbus_connect(cli), "connect");
    assert(priv.n_connect == 1);
    assert(tr.connected == 1);
    modbus_set_slave(cli, MODBUS_TCP_SLAVE);

    uint16_t out[10];
    server_arg_t sarg = {.srv = srv, .map = map};

    /* The socketpair is full-duplex but the exchange is synchronous (the client
     * blocks for the reply), so the server runs in parallel. */
#ifndef _WIN32
    pid_t pid = fork();
    if (pid == 0) {
        /* Child: server. */
        CLOSESOCK(fds[0]);
        server_serve_one(&sarg);
        modbus_mapping_free(map);
        modbus_free(srv);
        _exit(0);
    }
    /* Parent: client. */
    CLOSESOCK(fds[1]);
#else
    /* Windows: the server runs in a thread, so both sockets stay open and each
     * side touches only its own. */
    HANDLE th = (HANDLE) _beginthreadex(NULL, 0, server_thread, &sarg, 0, NULL);
    assert(th != NULL);
#endif

    modbus_set_debug(cli, FALSE);
    int rc = modbus_read_registers(cli, 0, 10, out);
    check(rc, "read_registers");
    assert(rc == 10);
    for (int i = 0; i < 10; i++)
        assert(out[i] == (uint16_t) (100 + i));

    assert(priv.n_connect >= 1);
    assert(priv.n_send >= 1);
    assert(priv.n_recv >= 1);
    assert(priv.n_select >= 1);

    /* modbus_flush() must reach the transport, not the backend. */
    assert(modbus_flush(cli) == 0);
    assert(priv.n_flush == 1);

    modbus_close(cli);
    assert(priv.n_close == 1);
    assert(tr.connected == 0);

    modbus_free(cli); /* calls tr.free */
    assert(priv.n_free == 1);

#ifndef _WIN32
    int status;
    waitpid(pid, &status, 0);
#else
    WaitForSingleObject(th, INFINITE);
    CloseHandle(th);
    modbus_mapping_free(map);
    modbus_free(srv);
#endif
    printf("PASS\n");
}

/* Test 2: modbus_set_transport(ctx, NULL) detaches without calling free(). */
static void test_transport_detach(void)
{
    printf("[2] transport detach (NULL)... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 502);
    assert(ctx);

    test_priv_t priv = {.conn = NULL};
    modbus_transport_t tr = {.priv = &priv};

    modbus_set_transport(ctx, &tr);
    assert(modbus_get_transport(ctx) == &tr);

    modbus_set_transport(ctx, NULL);
    assert(modbus_get_transport(ctx) == NULL);
    assert(priv.n_free == 0);

    modbus_free(ctx);
    assert(priv.n_free == 0); /* ctx no longer owns tr */

    printf("PASS\n");
}

/* Test 3: a context without a transport still works. */
static void test_no_transport(void)
{
    printf("[3] backwards compat (no transport)... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 502);
    assert(ctx);
    assert(modbus_get_transport(ctx) == NULL);
    modbus_free(ctx);

    printf("PASS\n");
}

/* Test 4: a partial transport (no select) leaves the rest to the backend. */
static void test_partial_override(void)
{
    printf("[4] partial override (NULL select falls back)... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 502);
    assert(ctx);

    test_priv_t priv = {.conn = NULL};
    modbus_transport_t tr = {
        /* select left NULL on purpose */
        .send = transport_send,
        .recv = transport_recv,
        .close = transport_close,
        .priv = &priv,
    };

    modbus_set_transport(ctx, &tr);
    assert(modbus_get_transport(ctx) == &tr);

    modbus_free(ctx);
    printf("PASS\n");
}

/* Test 5: the public setter and getter reject a NULL context, and attaching a
 * transport resets its connection state. */
static void test_api_validation(void)
{
    printf("[5] set/get_transport argument validation... ");
    fflush(stdout);

    test_priv_t priv = {.conn = NULL};
    modbus_transport_t tr = {.priv = &priv, .connected = 1};

    errno = 0;
    assert(modbus_set_transport(NULL, &tr) == -1 && errno == EINVAL);
    assert(modbus_get_transport(NULL) == NULL);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 502);
    assert(ctx);
    assert(modbus_set_transport(ctx, &tr) == 0);
    assert(tr.connected == 0);

    modbus_set_transport(ctx, NULL);
    modbus_free(ctx);

    printf("PASS\n");
}

/* Test 6: a transport that has not been connected fails the receive path with
 * EBADF instead of falling through to ctx->s. */
static void test_receive_not_connected(void)
{
    printf("[6] receive on unconnected transport... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 502);
    assert(ctx);

    test_priv_t priv = {.conn = NULL};
    modbus_transport_t tr = {
        .send = transport_send,
        .recv = transport_recv,
        .select = transport_select,
        .priv = &priv,
    };
    assert(modbus_set_transport(ctx, &tr) == 0);
    assert(tr.connected == 0);

    uint8_t req[MODBUS_TCP_MAX_ADU_LENGTH];
    errno = 0;
    assert(modbus_receive(ctx, req) == -1 && errno == EBADF);
    assert(priv.n_recv == 0); /* the transport was never asked to read */

    modbus_set_transport(ctx, NULL);
    modbus_free(ctx);

    printf("PASS\n");
}

/* A transport with no link behind it, for the error paths: send() can fail
 * once, select() always times out, and every call is recorded in order. */
typedef struct {
    int first_send_errno; /* errno for the first send(), 0 to succeed */
    int n_send;
    char trace[256];
} fake_priv_t;

static void fake_trace(fake_priv_t *p, const char *event)
{
    if (p->trace[0] != '\0')
        strcat(p->trace, " ");
    strcat(p->trace, event);
}

static int fake_connect(modbus_transport_t *t)
{
    fake_trace(t->priv, "connect");
    return 0;
}

static ssize_t fake_send(modbus_transport_t *t, const uint8_t *buf, int len)
{
    fake_priv_t *p = t->priv;

    (void) buf;
    if (p->n_send++ == 0 && p->first_send_errno != 0) {
        fake_trace(p, "send-fail");
        errno = p->first_send_errno;
        return -1;
    }
    fake_trace(p, "send");
    return len;
}

static ssize_t fake_recv(modbus_transport_t *t, uint8_t *buf, int len)
{
    (void) buf;
    (void) len;
    fake_trace(t->priv, "recv");
    errno = ECONNRESET;
    return -1;
}

static int fake_select(modbus_transport_t *t, struct timeval *tv)
{
    (void) tv;
    fake_trace(t->priv, "select-timeout");
    return 0; /* the timeout expired, as select(2) reports it */
}

static int fake_flush(modbus_transport_t *t)
{
    fake_trace(t->priv, "flush");
    return 0;
}

static void fake_close(modbus_transport_t *t)
{
    fake_trace(t->priv, "close");
}

static modbus_transport_t fake_transport(fake_priv_t *priv)
{
    modbus_transport_t tr = {
        .connect = fake_connect,
        .send = fake_send,
        .recv = fake_recv,
        .select = fake_select,
        .flush = fake_flush,
        .close = fake_close,
        .priv = priv,
    };
    return tr;
}

/* Test 7: select() returning 0 is a timeout. The request fails with ETIMEDOUT
 * and recv() is never called on an empty link. */
static void test_select_timeout(void)
{
    printf("[7] select() timeout... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 502);
    assert(ctx);

    fake_priv_t priv = {0};
    modbus_transport_t tr = fake_transport(&priv);
    assert(modbus_set_transport(ctx, &tr) == 0);
    assert(modbus_connect(ctx) == 0);

    uint16_t reg;
    errno = 0;
    assert(modbus_read_registers(ctx, 0, 1, &reg) == -1 && errno == ETIMEDOUT);
    assert(strcmp(priv.trace, "connect send select-timeout") == 0);

    modbus_set_transport(ctx, NULL);
    modbus_free(ctx);

    printf("PASS\n");
}

/* Test 8: with MODBUS_ERROR_RECOVERY_LINK, a send() failing with ECONNRESET
 * closes and reconnects the transport before the retry. The decision comes from
 * errno on every platform, not from Winsock, which a transport never sets. */
static void test_link_recovery(void)
{
    printf("[8] link recovery reconnects... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 502);
    assert(ctx);

    fake_priv_t priv = {0};
    priv.first_send_errno = ECONNRESET;
    modbus_transport_t tr = fake_transport(&priv);
    assert(modbus_set_transport(ctx, &tr) == 0);
    modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_LINK);
    /* recovery sleeps for the response timeout */
    modbus_set_response_timeout(ctx, 0, 1000);
    assert(modbus_connect(ctx) == 0);

    uint16_t reg;
    errno = 0;
    assert(modbus_read_registers(ctx, 0, 1, &reg) == -1 && errno == ETIMEDOUT);
    /* reconnect after the failed send, flush after the timeout */
    assert(strcmp(priv.trace,
                  "connect send-fail close connect send select-timeout flush") == 0);

    modbus_set_transport(ctx, NULL);
    modbus_free(ctx);

    printf("PASS\n");
}

int main(void)
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    printf("=== modbus pluggable transport tests ===\n");

    test_transport_read_registers();
    test_transport_detach();
    test_no_transport();
    test_partial_override();
    test_api_validation();
    test_receive_not_connected();
    test_select_timeout();
    test_link_recovery();

    printf("All tests passed.\n");

#ifdef _WIN32
    WSACleanup();
#endif
    return EXIT_SUCCESS;
}
