/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Regression test for the gateway proxy router (modbus_proxy_router). The
 * argument validation runs everywhere; the routing check wires a client, a
 * gateway and a downstream server with two socketpairs and routes by unit
 * identifier to the backend context serving it. The failure check reuses that
 * pipeline with a downstream that never answers or drops the link. Both need
 * concurrent processes, so they run on POSIX only. */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <modbus.h>

/* Resolver for the argument checks; the router must fail before calling it. */
static modbus_t *route_none(int slave, void *user)
{
    (void) slave;
    (void) user;
    assert(0 && "resolver called on an invalid request");
    return NULL;
}

static void test_api_validation(void)
{
    printf("[1] proxy_router argument validation... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 1502);
    assert(ctx);

    /* A TCP indication: 7 header bytes then the function code. */
    uint8_t req[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 7, 0x03};

    errno = 0;
    assert(modbus_proxy_router(NULL, req, (int) sizeof(req), route_none, NULL) == -1 &&
           errno == EINVAL);
    errno = 0;
    assert(modbus_proxy_router(ctx, NULL, (int) sizeof(req), route_none, NULL) == -1 &&
           errno == EINVAL);
    errno = 0;
    assert(modbus_proxy_router(ctx, req, (int) sizeof(req), NULL, NULL) == -1 &&
           errno == EINVAL);

    /* A request truncated before the function code is rejected without asking
       the resolver. */
    errno = 0;
    assert(modbus_proxy_router(ctx, req, 7, route_none, NULL) == -1 &&
           errno == EMBBADDATA);

    modbus_free(ctx);
    printf("PASS\n");
}

#ifndef _WIN32
/* Downstream server: answers one request from a single mapping (registers set
 * to 700 + index). */
static void run_downstream(int fd)
{
    modbus_mapping_t *map = modbus_mapping_new(0, 0, 10, 0);
    assert(map);
    for (int i = 0; i < 10; i++)
        map->tab_registers[i] = (uint16_t) (700 + i);

    modbus_t *dn = modbus_new_tcp("127.0.0.1", 1502);
    assert(dn);
    modbus_set_socket(dn, fd);
    modbus_set_debug(dn, FALSE);

    uint8_t req[MODBUS_TCP_MAX_ADU_LENGTH];
    int rc = modbus_receive(dn, req);
    if (rc > 0)
        modbus_reply(dn, req, rc, map);

    modbus_free(dn);
    modbus_mapping_free(map);
}

/* Route unit id 7 to the single downstream backend, everything else nowhere. */
static modbus_t *route(int slave, void *user)
{
    modbus_t *backend = user;
    return (slave == 7) ? backend : NULL;
}

/* Gateway: forwards the frontend indications through the router. The client
 * sends a routable unit then an unroutable one, so the router must relay the
 * first and report EMBXGPATH for the second. Failures abort the child and are
 * caught by the parent through its exit status. */
static void run_gateway(int frontend_fd, int backend_fd, int n)
{
    modbus_t *frontend = modbus_new_tcp("127.0.0.1", 1502);
    assert(frontend);
    modbus_set_socket(frontend, frontend_fd);
    modbus_set_debug(frontend, FALSE);

    modbus_t *backend = modbus_new_tcp("127.0.0.1", 1502);
    assert(backend);
    modbus_set_socket(backend, backend_fd);
    modbus_set_debug(backend, FALSE);
    modbus_set_response_timeout(backend, 2, 0);

    for (int i = 0; i < n; i++) {
        uint8_t req[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc = modbus_receive(frontend, req);
        assert(rc > 0);

        errno = 0;
        rc = modbus_proxy_router(frontend, req, rc, route, backend);
        if (i == 0) {
            assert(rc > 0); /* unit 7, relayed from the downstream server */
        } else {
            assert(rc == -1 && errno == EMBXGPATH); /* unroutable unit */
        }
    }

    modbus_free(frontend);
    modbus_free(backend);
}

static void test_proxy_router(void)
{
    printf("[2] proxy_router routing... ");
    fflush(stdout);

    int fe[2]; /* client <-> gateway frontend */
    int be[2]; /* gateway backend <-> downstream */
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fe) == 0);
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, be) == 0);

    pid_t dn_pid = fork();
    if (dn_pid == 0) {
        close(fe[0]);
        close(fe[1]);
        close(be[0]);
        run_downstream(be[1]);
        _exit(0);
    }

    pid_t gw_pid = fork();
    if (gw_pid == 0) {
        close(fe[0]);
        close(be[1]);
        run_gateway(fe[1], be[0], 2);
        _exit(0);
    }

    /* Client (parent). */
    close(fe[1]);
    close(be[0]);
    close(be[1]);

    modbus_t *cli = modbus_new_tcp("127.0.0.1", 1502);
    assert(cli);
    modbus_set_socket(cli, fe[0]);
    modbus_set_response_timeout(cli, 3, 0);
    modbus_set_debug(cli, FALSE);

    uint16_t out[10];

    /* Unit 7 is routed to the downstream server. */
    modbus_set_slave(cli, 7);
    assert(modbus_read_registers(cli, 0, 10, out) == 10);
    for (int i = 0; i < 10; i++)
        assert(out[i] == (uint16_t) (700 + i));

    /* An unroutable unit gets a gateway path exception. */
    modbus_set_slave(cli, 9);
    errno = 0;
    assert(modbus_read_registers(cli, 0, 10, out) == -1 && errno == EMBXGPATH);

    modbus_free(cli);
    close(fe[0]);

    /* The gateway checks the router return codes itself, so its exit status
       carries those results back here. */
    int status;
    assert(waitpid(gw_pid, &status, 0) == gw_pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    assert(waitpid(dn_pid, &status, 0) == dn_pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    printf("PASS\n");
}

/* Downstream variants for the failure cases: swallow the request without ever
 * answering, or drop the link straight away. */
#define DOWNSTREAM_SILENT 1
#define DOWNSTREAM_DROP   0

static void run_downstream_failing(int fd, int mode)
{
    if (mode == DOWNSTREAM_SILENT) {
        uint8_t buf[MODBUS_TCP_MAX_ADU_LENGTH];
        ssize_t n;

        n = read(fd, buf, sizeof(buf));
        /* Hold the link open until the gateway gives up and closes it, so the
           only wait is the backend response timeout. */
        n = read(fd, buf, sizeof(buf));
        (void) n;
    }
    close(fd);
}

/* Gateway for the failure cases: the router must report the backend failure
 * instead of relaying a response. */
static void run_gateway_expect_failure(int frontend_fd, int backend_fd)
{
    modbus_t *frontend = modbus_new_tcp("127.0.0.1", 1502);
    modbus_t *backend = modbus_new_tcp("127.0.0.1", 1502);
    assert(frontend && backend);
    modbus_set_socket(frontend, frontend_fd);
    modbus_set_socket(backend, backend_fd);
    modbus_set_debug(frontend, FALSE);
    modbus_set_debug(backend, FALSE);
    /* Short enough to keep the test quick, long enough to be unambiguous. */
    modbus_set_response_timeout(backend, 0, 200000);

    uint8_t req[MODBUS_TCP_MAX_ADU_LENGTH];
    int rc = modbus_receive(frontend, req);
    assert(rc > 0);
    assert(modbus_proxy_router(frontend, req, rc, route, backend) == -1);

    modbus_free(frontend);
    modbus_free(backend);
}

/* Drive one failing exchange and check the exception the client ends up with. */
static void check_backend_failure(int mode, int expected_errno)
{
    int fe[2]; /* client <-> gateway frontend */
    int be[2]; /* gateway backend <-> downstream */
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fe) == 0);
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, be) == 0);

    pid_t dn_pid = fork();
    if (dn_pid == 0) {
        close(fe[0]);
        close(fe[1]);
        close(be[0]);
        run_downstream_failing(be[1], mode);
        _exit(0);
    }

    pid_t gw_pid = fork();
    if (gw_pid == 0) {
        close(fe[0]);
        close(be[1]);
        run_gateway_expect_failure(fe[1], be[0]);
        _exit(0);
    }

    /* Client (parent). */
    close(fe[1]);
    close(be[0]);
    close(be[1]);

    modbus_t *cli = modbus_new_tcp("127.0.0.1", 1502);
    assert(cli);
    modbus_set_socket(cli, fe[0]);
    modbus_set_response_timeout(cli, 3, 0);
    modbus_set_debug(cli, FALSE);

    uint16_t out[10];
    modbus_set_slave(cli, 7);
    errno = 0;
    assert(modbus_read_registers(cli, 0, 10, out) == -1);
    assert(errno == expected_errno);

    modbus_free(cli);
    close(fe[0]);

    int status;
    assert(waitpid(gw_pid, &status, 0) == gw_pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    assert(waitpid(dn_pid, &status, 0) == dn_pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/* modbus_proxy() turns a backend failure into a gateway exception and the
 * router passes it through: a silent downstream times out and yields a gateway
 * target exception, a dropped link yields a gateway path exception. */
static void test_backend_failure(void)
{
    printf("[3] proxy_router backend failure... ");
    fflush(stdout);

    check_backend_failure(DOWNSTREAM_SILENT, EMBXGTAR);
    check_backend_failure(DOWNSTREAM_DROP, EMBXGPATH);

    printf("PASS\n");
}
#endif

int main(void)
{
    printf("=== modbus proxy router tests ===\n");

    test_api_validation();
#ifndef _WIN32
    test_proxy_router();
    test_backend_failure();
#else
    printf("[2] proxy_router routing... SKIP (no fork on Windows)\n");
    printf("[3] proxy_router backend failure... SKIP (no fork on Windows)\n");
#endif

    printf("All tests passed.\n");
    return EXIT_SUCCESS;
}
