/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Regression test for the per-slave reply router (modbus_reply_router) and the
 * modbus_get_request_slave() helper. The helper and argument checks run
 * everywhere; the
 * router dispatch drives a TCP client and server over a socketpair with the
 * server in a child process, so it runs on POSIX only. */

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

typedef struct {
    modbus_mapping_t *map1;
    modbus_mapping_t *map2;
} maps_t;

/* Serve unit 1 from map1 and unit 2 from map2; anything else is unavailable. */
static modbus_mapping_t *resolve(int slave, void *user)
{
    maps_t *maps = user;
    if (slave == 1)
        return maps->map1;
    if (slave == 2)
        return maps->map2;
    return NULL;
}

static void test_get_request_slave(void)
{
    printf("[1] get_request_slave... ");
    fflush(stdout);

    /* TCP header is 7 bytes, so the unit id is at offset 6. */
    modbus_t *tcp = modbus_new_tcp("127.0.0.1", 1502);
    assert(tcp);
    uint8_t tcp_req[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 42, 0x03};
    assert(modbus_get_request_slave(tcp, tcp_req) == 42);

    errno = 0;
    assert(modbus_get_request_slave(tcp, NULL) == -1 && errno == EINVAL);
    modbus_free(tcp);

    /* RTU header is 1 byte, so the unit id is at offset 0. */
    modbus_t *rtu = modbus_new_rtu("/dev/null", 9600, 'N', 8, 1);
    assert(rtu);
    uint8_t rtu_req[] = {17, 0x03, 0x00, 0x00};
    assert(modbus_get_request_slave(rtu, rtu_req) == 17);
    modbus_free(rtu);

    errno = 0;
    assert(modbus_get_request_slave(NULL, tcp_req) == -1 && errno == EINVAL);

    printf("PASS\n");
}

/* Resolver for the argument checks; the router must fail before calling it. */
static modbus_mapping_t *resolve_none(int slave, void *user)
{
    (void) slave;
    (void) user;
    assert(0 && "resolver called on an invalid request");
    return NULL;
}

static void test_reply_router_validation(void)
{
    printf("[2] reply_router argument validation... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 1502);
    assert(ctx);

    /* A TCP indication: 7 header bytes then the function code. */
    uint8_t req[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 1, 0x03};

    errno = 0;
    assert(modbus_reply_router(NULL, req, (int) sizeof(req), resolve_none, NULL) == -1 &&
           errno == EINVAL);
    errno = 0;
    assert(modbus_reply_router(ctx, NULL, (int) sizeof(req), resolve_none, NULL) == -1 &&
           errno == EINVAL);
    errno = 0;
    assert(modbus_reply_router(ctx, req, (int) sizeof(req), NULL, NULL) == -1 &&
           errno == EINVAL);

    /* A request truncated before the function code is rejected without asking
       the resolver. */
    errno = 0;
    assert(modbus_reply_router(ctx, req, 7, resolve_none, NULL) == -1 &&
           errno == EMBBADDATA);

    modbus_free(ctx);
    printf("PASS\n");
}

#ifndef _WIN32
static void serve_requests(int fd, int n)
{
    maps_t maps;
    maps.map1 = modbus_mapping_new(0, 0, 10, 0);
    maps.map2 = modbus_mapping_new(0, 0, 10, 0);
    assert(maps.map1 && maps.map2);
    for (int i = 0; i < 10; i++) {
        maps.map1->tab_registers[i] = (uint16_t) (100 + i);
        maps.map2->tab_registers[i] = (uint16_t) (200 + i);
    }

    modbus_t *srv = modbus_new_tcp("127.0.0.1", 1502);
    assert(srv);
    modbus_set_socket(srv, fd);
    modbus_set_debug(srv, FALSE);

    /* Every reply is sent, including the gateway path exception raised for the
       unserved unit, so the router returns the response length each time.
       Failures abort the child and the parent catches them in its exit status. */
    for (int i = 0; i < n; i++) {
        uint8_t req[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc = modbus_receive(srv, req);
        assert(rc > 0);
        assert(modbus_reply_router(srv, req, rc, resolve, &maps) > 0);
    }

    modbus_free(srv);
    modbus_mapping_free(maps.map1);
    modbus_mapping_free(maps.map2);
}

static void test_reply_router(void)
{
    printf("[3] reply_router dispatch... ");
    fflush(stdout);

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    pid_t pid = fork();
    if (pid == 0) {
        close(fds[0]);
        serve_requests(fds[1], 3);
        _exit(0);
    }
    close(fds[1]);

    modbus_t *cli = modbus_new_tcp("127.0.0.1", 1502);
    assert(cli);
    modbus_set_socket(cli, fds[0]);
    modbus_set_response_timeout(cli, 2, 0);
    modbus_set_debug(cli, FALSE);

    uint16_t out[10];

    /* Unit 1 is served from map1. */
    modbus_set_slave(cli, 1);
    assert(modbus_read_registers(cli, 0, 10, out) == 10);
    for (int i = 0; i < 10; i++)
        assert(out[i] == (uint16_t) (100 + i));

    /* Unit 2 is served from map2. */
    modbus_set_slave(cli, 2);
    assert(modbus_read_registers(cli, 0, 10, out) == 10);
    for (int i = 0; i < 10; i++)
        assert(out[i] == (uint16_t) (200 + i));

    /* An unserved unit gets a gateway path exception. */
    modbus_set_slave(cli, 9);
    errno = 0;
    assert(modbus_read_registers(cli, 0, 10, out) == -1 && errno == EMBXGPATH);

    modbus_free(cli);
    close(fds[0]);

    /* The server checks the router return codes itself, so its exit status
       carries those results back here. */
    int status;
    assert(waitpid(pid, &status, 0) == pid);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    printf("PASS\n");
}
#endif

int main(void)
{
    printf("=== modbus reply router tests ===\n");

    test_get_request_slave();
    test_reply_router_validation();
#ifndef _WIN32
    test_reply_router();
#else
    printf("[3] reply_router dispatch... SKIP (no fork on Windows)\n");
#endif

    printf("All tests passed.\n");
    return EXIT_SUCCESS;
}
