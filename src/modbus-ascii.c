/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <assert.h>

#include "modbus-private.h"

#include "modbus-ascii.h"
#include "modbus-ascii-private.h"

#include "modbus-serial-private.h"

static char nibble_to_hex_ascii(uint8_t nibble)
{
    char c;

    if (nibble < 10) {
        c = nibble + '0';
    } else {
        c = nibble - 10 + 'A';
    }
    return c;
}

static uint8_t hex_ascii_to_nibble(char digit) {
    if (digit >= '0' && digit <= '9' ) {
        return digit - '0';
    } else if (digit >= 'A' && digit <= 'F' ) {
        return digit - 'A' + 10;
    } else if (digit >= 'a' && digit <= 'f' ) {
        return digit - 'a' + 10;
    }
    return 0xff;
}

/* Build an ASCII request header */
static int _modbus_ascii_build_request_basis(modbus_t *ctx, int function,
                                             int addr, int nb, uint8_t *req)
{
    assert(ctx->slave != -1);

    req[0] = ':';
    req[1] = ctx->slave;
    req[2] = function;
    req[3] = addr >> 8;
    req[4] = addr & 0x00ff;
    req[5] = nb >> 8;
    req[6] = nb & 0x00ff;

    return 7;
}

/* Build an ASCII response header */
static int _modbus_ascii_build_response_basis(sft_t *sft, uint8_t *rsp)
{
    rsp[0] = ':';
    rsp[1] = sft->slave;
    rsp[2] = sft->function;

    return 3;
}

/* calculate the Longitudinal Redundancy Checking (LRC)
 * see http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf -
 * 2.5.2.2 LRC Checking Page 18
 * and
 * 6.2.1 LRC Generation Page 38
 **/
static uint8_t lrc8(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t lrc = 0;
    while (buffer_length--) {
        lrc += *buffer++;
    }
    /* Return two's complementing of the result */
    return -lrc;
}

static int _modbus_ascii_prepare_response_tid(const uint8_t *req, int *req_length)
{
    (*req_length) -= _MODBUS_ASCII_CHECKSUM_LENGTH;

    /* No TID */
    return 0;
}

static int _modbus_ascii_send_msg_pre(uint8_t *req, int req_length)
{
    /* Skip colon */
    uint8_t lrc = lrc8(req + 1, req_length - 1);

    req[req_length++] = lrc;
    req[req_length++] = '\r';
    req[req_length++] = '\n';

    return req_length;
}

static int _modbus_ascii_check_integrity(modbus_t *ctx, uint8_t *msg, const int msg_length)
{
    uint8_t lrc;
    char colon = msg[0];
    int slave = msg[1];

    /* Check for leading colon */
    if (colon != ':') {
        if (ctx->debug) {
            fprintf(stderr, "No leading colon.\n");
        }
        /* Following call to check_confirmation handles this error */
        return 0;
    }

    /* Filter on the Modbus unit identifier (slave) in ASCII mode to avoid useless
     * CRC computing. */
    if (slave != ctx->slave && slave != MODBUS_BROADCAST_ADDRESS) {
        if (ctx->debug) {
            fprintf(stderr, "Request for slave %d ignored (not %d)\n", slave, ctx->slave);
        }
        /* Following call to check_confirmation handles this error */
        return 0;
    }

    /* Strip ":" and "\r\n" */
    lrc = lrc8(msg + 1, msg_length - 3);

    /* Check CRC of msg */
    if (lrc == 0) {
        return msg_length;
    } else {
        if (ctx->debug) {
            fprintf(stderr, "ERROR lrc received %0X != 0\n", lrc);
        }

        if (ctx->error_recovery & MODBUS_ERROR_RECOVERY_PROTOCOL) {
            _modbus_serial_flush(ctx);
        }
        errno = EMBBADCRC;
        return -1;
    }
}

static ssize_t _modbus_ascii_recv_char(modbus_t *ctx, uint8_t *p_char_rsp, uint8_t with_select)
{
    int rc;
    fd_set rset;
    struct timeval tv;

    if (with_select) {
        FD_ZERO(&rset);
        FD_SET(ctx->s, &rset);

        if (ctx->byte_timeout.tv_sec >= 0 && ctx->byte_timeout.tv_usec >= 0) {
            /* Byte timeout can be disabled with negative values */
            tv.tv_sec = ctx->byte_timeout.tv_sec;
            tv.tv_usec = ctx->byte_timeout.tv_usec;
        } else {
            tv.tv_sec = ctx->response_timeout.tv_sec;
            tv.tv_usec = ctx->response_timeout.tv_usec;
        }

        rc = _modbus_serial_select(ctx, &rset, &tv, 1);
        if (rc == -1) {
            return 0;
        }
    }

    return _modbus_serial_recv(ctx, p_char_rsp, 1);
}

/* We're reading character by character (ignoring how many bytes the caller
 * requested), translating between Modbus RTU and Modbus ASCII as needed.
 * Maybe it's better to try to read as many bytes as possible and then convert?
 */
static ssize_t _modbus_ascii_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length)
{
    uint8_t char_resp;
    uint8_t nibble_resp;

    if (_modbus_ascii_recv_char(ctx, &char_resp, 0) != 1) {
        return 0;
    }

    if (char_resp == ':' || char_resp == '\r' || char_resp == '\n') {
        *rsp = char_resp;
    } else {
        nibble_resp = hex_ascii_to_nibble(char_resp);
        *rsp = nibble_resp << 4;
        if (_modbus_ascii_recv_char(ctx, &char_resp, 1) != 1) {
            return 0;
        }
        nibble_resp = hex_ascii_to_nibble(char_resp);
        *rsp |= nibble_resp;
    }
    return 1;
}

static ssize_t _modbus_ascii_send(modbus_t *ctx, const uint8_t *req, int req_length)
{
    /* FIXME Ugly copy! */
    uint8_t ascii_req[MODBUS_ASCII_MAX_ADU_LENGTH];
    ssize_t i, j = 0;

    for (i = 0; i < req_length; i++) {
        if ((i == 0 && req[i] == ':') ||
            (i == req_length - 2 && req[i] == '\r') ||
            (i == req_length - 1 && req[i] == '\n')) {
            ascii_req[j++] = req[i];
        } else {
            ascii_req[j++] = nibble_to_hex_ascii(req[i] >> 4);
            ascii_req[j++] = nibble_to_hex_ascii(req[i] & 0x0f);
        }
    }
    ascii_req[j] = '\0';

    ssize_t size = _modbus_serial_send(ctx, ascii_req, j);
    /* FIXME */
    return ((size - 3) / 2) + 3;
}

static void _modbus_ascii_free(modbus_t *ctx) {
    _modbus_serial_free(ctx->backend_data);
    free(ctx);
}

const modbus_backend_t _modbus_ascii_backend = {
    _MODBUS_BACKEND_TYPE_SERIAL,
    _MODBUS_ASCII_HEADER_LENGTH,
    _MODBUS_ASCII_CHECKSUM_LENGTH,
    MODBUS_ASCII_MAX_ADU_LENGTH,
    _modbus_serial_set_slave,
    _modbus_ascii_build_request_basis,
    _modbus_ascii_build_response_basis,
    _modbus_ascii_prepare_response_tid,
    _modbus_ascii_send_msg_pre,
    _modbus_ascii_send,
    _modbus_serial_receive,
    _modbus_ascii_recv,
    _modbus_ascii_check_integrity,
    _modbus_serial_pre_check_confirmation,
    _modbus_serial_connect,
    _modbus_serial_close,
    _modbus_serial_flush,
    _modbus_serial_select,
    _modbus_ascii_free
};

modbus_t* modbus_new_ascii(const char *device,
                           int baud, char parity, int data_bit, int stop_bit)
{
    return _modbus_serial_new(&_modbus_ascii_backend,
                              device, baud, parity, data_bit, stop_bit);
}
