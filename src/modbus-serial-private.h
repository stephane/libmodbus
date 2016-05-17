/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_SERIAL_PRIVATE_H
#define MODBUS_SERIAL_PRIVATE_H

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif

#if defined(_WIN32)
#include <windows.h>
#else
#include <termios.h>
#endif

#include "modbus-private.h"
#include "modbus-serial.h"

#if defined(_WIN32)
#if !defined(ENOTSUP)
#define ENOTSUP WSAEOPNOTSUPP
#endif

/* WIN32: struct containing serial handle and a receive buffer */
#define PY_BUF_SIZE 512
struct win32_ser {
    /* File handle */
    HANDLE fd;
    /* Receive buffer */
    uint8_t buf[PY_BUF_SIZE];
    /* Received chars */
    DWORD n_bytes;
};
#endif /* _WIN32 */

typedef struct _modbus_serial {
    /* Device: "/dev/ttyS0", "/dev/ttyUSB0" or "/dev/tty.USA19*" on Mac OS X. */
    char *device;
    /* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;
#if defined(_WIN32)
    struct win32_ser w_ser;
    DCB old_dcb;
#else
    /* Save old termios settings */
    struct termios old_tios;
#endif
#if HAVE_DECL_TIOCSRS485
    int serial_mode;
#endif
#if HAVE_DECL_TIOCM_RTS
    int rts;
    int rts_delay;
    int onebyte_time;
    void (*set_rts) (modbus_t *ctx, int on);
#endif
    /* To handle many slaves on the same link */
    int confirmation_to_ignore;
} modbus_serial_t;

ssize_t _modbus_serial_send(modbus_t *ctx, const uint8_t *req, int req_length);
int _modbus_serial_receive(modbus_t *ctx, uint8_t *req);
ssize_t _modbus_serial_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length);
int _modbus_serial_pre_check_confirmation(modbus_t *ctx, const uint8_t *req, const uint8_t *rsp, int rsp_length);
int _modbus_serial_connect(modbus_t *ctx);
void _modbus_serial_close(modbus_t *ctx);
int _modbus_serial_flush(modbus_t *ctx);
int _modbus_serial_select(modbus_t *ctx, fd_set *rset, struct timeval *tv, int length_to_read);

int _modbus_serial_set_slave(modbus_t *ctx, int slave);
void _modbus_serial_free(modbus_serial_t *serial_ctx);
modbus_t* _modbus_serial_new(const modbus_backend_t *modbus_backend,
                             const char *device, int baud, char parity, int data_bit, int stop_bit);

#endif /* MODBUS_SERIAL_PRIVATE_H */
