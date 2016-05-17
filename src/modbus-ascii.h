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
 * RS232/RS485 ADU =
 * start (1 byte) + address (2 bytes) + function (2 bytes) + 2x252 bytes + LRC (2 bytes) + end (2 bytes) = 513 bytes
 */
#define MODBUS_ASCII_MAX_ADU_LENGTH 513

MODBUS_API modbus_t* modbus_new_ascii(const char *device, int baud, char parity,
                                      int data_bit, int stop_bit);

MODBUS_END_DECLS

#endif /* MODBUS_ASCII_H */
