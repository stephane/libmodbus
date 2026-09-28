/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Regression test for the RTU multi-slave accept filter
 * (modbus_rtu_set_slave_filter). The API validation runs everywhere; the
 * receive-filtering checks drive modbus_receive() over a socketpair used as a
 * fake serial link and therefore run on POSIX only — on Windows the RTU backend
 * does its I/O through a serial HANDLE rather than ctx->s. */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <modbus.h>

/* Standard Modbus RTU CRC-16 (polynomial 0xA001, init 0xFFFF). */
static uint16_t rtu_crc16(const uint8_t *buf, int len)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= (uint16_t) buf[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/* Build a "read holding registers" indication addressed to `slave`.
 * Returns the frame length. */
static int build_read_request(uint8_t *frame, int slave)
{
    frame[0] = (uint8_t) slave;
    frame[1] = 0x03; /* FC read holding registers */
    frame[2] = 0x00;
    frame[3] = 0x00; /* address 0 */
    frame[4] = 0x00;
    frame[5] = 0x01; /* quantity 1 */
    uint16_t crc = rtu_crc16(frame, 6);
    frame[6] = (uint8_t) (crc & 0xFF); /* RTU sends the CRC low byte first */
    frame[7] = (uint8_t) (crc >> 8);
    return 8;
}

static void test_api_validation(void)
{
    printf("[1] set_slave_filter API validation... ");
    fflush(stdout);

    /* NULL context is rejected. */
    errno = 0;
    assert(modbus_rtu_set_slave_filter(NULL, NULL, 0) == -1 && errno == EINVAL);

    /* A non-RTU context is rejected. */
    modbus_t *tcp = modbus_new_tcp("127.0.0.1", 1502);
    assert(tcp);
    uint8_t ids[] = {5};
    errno = 0;
    assert(modbus_rtu_set_slave_filter(tcp, ids, 1) == -1 && errno == EINVAL);
    modbus_free(tcp);

    modbus_t *ctx = modbus_new_rtu("/dev/null", 9600, 'N', 8, 1);
    assert(ctx);

    /* An out-of-range unit id is rejected and leaves the filter unchanged. */
    uint8_t bad[] = {248};
    errno = 0;
    assert(modbus_rtu_set_slave_filter(ctx, bad, 1) == -1 && errno == EINVAL);

    /* A valid set is accepted, and a NULL/empty list clears it. */
    uint8_t good[] = {1, 5, 247};
    assert(modbus_rtu_set_slave_filter(ctx, good, 3) == 0);
    assert(modbus_rtu_set_slave_filter(ctx, NULL, 0) == 0);

    /* An empty list is also accepted through a non-NULL pointer. */
    assert(modbus_rtu_set_slave_filter(ctx, good, 0) == 0);

    modbus_free(ctx);
    printf("PASS\n");
}

static void test_max_slave_quirk(void)
{
    printf("[2] set_slave_filter MAX_SLAVE quirk... ");
    fflush(stdout);

    modbus_t *ctx = modbus_new_rtu("/dev/null", 9600, 'N', 8, 1);
    assert(ctx);

    /* Ids above 247 are reserved by the Modbus specification and rejected. */
    uint8_t reserved[] = {248, 255};
    errno = 0;
    assert(modbus_rtu_set_slave_filter(ctx, reserved, 2) == -1 && errno == EINVAL);

    /* MODBUS_QUIRK_MAX_SLAVE raises the accepted range to the full byte. */
    assert(modbus_enable_quirks(ctx, MODBUS_QUIRK_MAX_SLAVE) == 0);
    assert(modbus_rtu_set_slave_filter(ctx, reserved, 2) == 0);

    /* Disabling it restores the specification limit. */
    assert(modbus_disable_quirks(ctx, MODBUS_QUIRK_MAX_SLAVE) == 0);
    errno = 0;
    assert(modbus_rtu_set_slave_filter(ctx, reserved, 2) == -1 && errno == EINVAL);

    /* A rejected list leaves the previously accepted filter in place. */
    uint8_t mixed[] = {5, 248};
    errno = 0;
    assert(modbus_rtu_set_slave_filter(ctx, mixed, 2) == -1 && errno == EINVAL);

    modbus_free(ctx);
    printf("PASS\n");
}

#ifndef _WIN32
/* Write a request for `slave` to the master end, then receive it on the server
 * context. Returns the modbus_receive() result (>0 accepted, 0 ignored). */
static int receive_for_slave(modbus_t *srv, int fd_master, int slave)
{
    uint8_t frame[8];
    int len = build_read_request(frame, slave);
    assert(write(fd_master, frame, len) == len);

    uint8_t req[MODBUS_RTU_MAX_ADU_LENGTH];
    return modbus_receive(srv, req);
}

static void test_receive_filtering(void)
{
    printf("[3] RTU receive filtering... ");
    fflush(stdout);

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    modbus_t *srv = modbus_new_rtu("/dev/null", 9600, 'N', 8, 1);
    assert(srv);
    modbus_set_socket(srv, fds[1]);
    /* A bounded timeout so a wrongly-ignored frame fails instead of hanging. */
    modbus_set_response_timeout(srv, 1, 0);

    /* Multi-slave: accept unit ids 5, 10 and 247 only. The last one exercises
       the top of the bitmap (byte 30, bit 7). */
    uint8_t ids[] = {5, 10, 247};
    assert(modbus_rtu_set_slave_filter(srv, ids, 3) == 0);

    /* The filter replaces ctx->slave, it does not extend it: an id set with
       modbus_set_slave() but absent from the filter is ignored. */
    modbus_set_slave(srv, 3);

    assert(receive_for_slave(srv, fds[0], 5) > 0);   /* accepted */
    assert(receive_for_slave(srv, fds[0], 10) > 0);  /* accepted */
    assert(receive_for_slave(srv, fds[0], 247) > 0); /* accepted, high bit */
    assert(receive_for_slave(srv, fds[0], 7) == 0);  /* ignored */
    assert(receive_for_slave(srv, fds[0], 3) == 0);  /* ctx->slave, ignored */

    /* Broadcast is always accepted, even with a filter set. */
    assert(receive_for_slave(srv, fds[0], MODBUS_BROADCAST_ADDRESS) > 0);

    /* A filtered server is the authoritative responder for its own ids, so an
       ignored indication must not arm the confirmation-to-ignore state: the
       very next indication has to be delivered rather than swallowed. */
    assert(receive_for_slave(srv, fds[0], 7) == 0); /* ignored */
    assert(receive_for_slave(srv, fds[0], 5) > 0);  /* still delivered */

    /* Clearing the filter restores single-slave behaviour on ctx->slave. */
    assert(modbus_rtu_set_slave_filter(srv, NULL, 0) == 0);
    modbus_set_slave(srv, 17);
    assert(receive_for_slave(srv, fds[0], 17) > 0); /* ctx->slave */
    assert(receive_for_slave(srv, fds[0], 5) == 0); /* no longer accepted */

    modbus_free(srv);
    close(fds[0]);
    close(fds[1]);
    printf("PASS\n");
}

/* Read whatever the server put on the link, up to `ms` milliseconds. */
static int read_response(int fd, uint8_t *buf, int len, int ms)
{
    fd_set rset;
    struct timeval tv;
    ssize_t n;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (select(fd + 1, &rset, NULL, NULL, &tv) <= 0)
        return 0;

    n = read(fd, buf, len);
    return n < 0 ? 0 : (int) n;
}

/* A server answering for several unit ids must reply as the addressed unit
 * rather than as ctx->slave, and must leave the link silent for an id it does
 * not serve. */
static void test_reply_addressing(void)
{
    printf("[4] multi-slave reply addressing... ");
    fflush(stdout);

    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    modbus_t *srv = modbus_new_rtu("/dev/null", 9600, 'N', 8, 1);
    assert(srv);
    modbus_set_socket(srv, fds[1]);
    modbus_set_response_timeout(srv, 1, 0);

    modbus_mapping_t *map = modbus_mapping_new(0, 0, 10, 0);
    assert(map);
    for (int i = 0; i < 10; i++)
        map->tab_registers[i] = (uint16_t) (900 + i);

    uint8_t ids[] = {5, 10};
    assert(modbus_rtu_set_slave_filter(srv, ids, 2) == 0);
    /* Set on purpose to an id outside the filter. */
    modbus_set_slave(srv, 3);

    uint8_t req[MODBUS_RTU_MAX_ADU_LENGTH];
    uint8_t rsp[MODBUS_RTU_MAX_ADU_LENGTH];
    uint8_t frame[8];
    int len;
    int rc;
    int n;

    for (unsigned int i = 0; i < sizeof(ids); i++) {
        len = build_read_request(frame, ids[i]);
        assert(write(fds[0], frame, len) == len);

        rc = modbus_receive(srv, req);
        assert(rc > 0);
        assert(modbus_reply(srv, req, rc, map) > 0);

        n = read_response(fds[0], rsp, sizeof(rsp), 1000);
        assert(n >= 5);
        /* The response carries the addressed unit, not ctx->slave. */
        assert(rsp[0] == ids[i]);

        uint16_t crc = rtu_crc16(rsp, n - 2);
        assert(rsp[n - 2] == (uint8_t) (crc & 0xFF));
        assert(rsp[n - 1] == (uint8_t) (crc >> 8));
    }

    /* An id outside the filter is ignored and nothing reaches the link. */
    len = build_read_request(frame, 7);
    assert(write(fds[0], frame, len) == len);
    assert(modbus_receive(srv, req) == 0);
    assert(read_response(fds[0], rsp, sizeof(rsp), 200) == 0);

    modbus_mapping_free(map);
    modbus_free(srv);
    close(fds[0]);
    close(fds[1]);
    printf("PASS\n");
}
#endif

int main(void)
{
    printf("=== modbus RTU multi-slave filter tests ===\n");

    test_api_validation();
    test_max_slave_quirk();
#ifndef _WIN32
    test_receive_filtering();
    test_reply_addressing();
#else
    printf("[3] RTU receive filtering... SKIP (no ctx->s serial link on Windows)\n");
    printf("[4] multi-slave reply addressing... SKIP (no ctx->s serial link on Windows)\n");
#endif

    printf("All tests passed.\n");
    return EXIT_SUCCESS;
}
