/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_ASCII_H
#define MODBUS_ASCII_H

#include "modbus.h"

MODBUS_BEGIN_DECLS

/* Modbus_over_serial_line_V1_02.pdf - 2.5.2.1 MODBUS Message ASCII Framing
 * Page 17
 * RS232 / RS485 ADU =
 * start (1 byte) + address (2 bytes) + function (2 bytes) + 2x252 bytes + LRC (2 bytes) + end (2 bytes) = 513 bytes
 */
#define MODBUS_ASCII_MAX_ADU_LENGTH  256

MODBUS_API modbus_t* modbus_new_ascii(const char *device, int baud, char parity,
                                      int data_bit, int stop_bit);

/*
 * The get/set serial mode and get/set rts methods should be deprecated,
 * as they just forward to the serial backend.
 * Currently they are kept so the known interface does not change.
 */
#define MODBUS_ASCII_RS232 0
#define MODBUS_ASCII_RS485 1

MODBUS_API int modbus_ascii_set_serial_mode(modbus_t *ctx, int mode);
MODBUS_API int modbus_ascii_get_serial_mode(modbus_t *ctx);

#define MODBUS_ASCII_RTS_NONE   0
#define MODBUS_ASCII_RTS_UP     1
#define MODBUS_ASCII_RTS_DOWN   2

MODBUS_API int modbus_ascii_set_rts(modbus_t *ctx, int mode);
MODBUS_API int modbus_ascii_get_rts(modbus_t *ctx);

MODBUS_END_DECLS

#endif /* MODBUS_ASCII_H */
