/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_SERIAL_H
#define MODBUS_SERIAL_H

typedef struct _modbus_serial modbus_serial_t;

#define MODBUS_SERIAL_RS232 0
#define MODBUS_SERIAL_RS485 1

int modbus_serial_set_serial_mode(modbus_t *ctx, int mode);
int modbus_serial_get_serial_mode(modbus_t *ctx);

#define MODBUS_SERIAL_RTS_NONE   0
#define MODBUS_SERIAL_RTS_UP     1
#define MODBUS_SERIAL_RTS_DOWN   2

int modbus_serial_set_rts(modbus_t *ctx, int mode);
int modbus_serial_get_rts(modbus_t *ctx);

#if defined(_WIN32)

void win32_ser_init(struct win32_ser *ws);
int win32_ser_select(struct win32_ser *ws, int max_len, const struct timeval *tv);
int win32_ser_read(struct win32_ser *ws, uint8_t *p_msg, unsigned int max_len);

#endif

#if HAVE_DECL_TIOCM_RTS
void _modbus_serial_ioctl_rts(int fd, int on);
#endif

ssize_t _modbus_serial_send(modbus_t *ctx, const uint8_t *req, int req_length);
int _modbus_serial_receive(modbus_t *ctx, uint8_t *req);
ssize_t _modbus_serial_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length);
int _modbus_serial_flush(modbus_t *);
int _modbus_serial_pre_check_confirmation(modbus_t *ctx, const uint8_t *req, const uint8_t *rsp, int rsp_length);
int _modbus_serial_connect(modbus_t *ctx);
void _modbus_serial_close(modbus_t *ctx);
int _modbus_serial_flush(modbus_t *ctx);
int _modbus_serial_select(modbus_t *ctx, fd_set *rset, struct timeval *tv, int length_to_read);

void _modbus_serial_free(modbus_serial_t *serial_ctx);
modbus_serial_t* modbus_serial_init(const char *device, int baud, char parity, int data_bit, int stop_bit);

#endif /* MODBUS_SERIAL_H */
