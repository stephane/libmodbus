/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "modbus.h"
#include "modbus-serial.h"

MODBUS_BEGIN_DECLS

/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * RS232 / RS485 ADU = 253 bytes + slave (1 byte) + CRC (2 bytes) = 256 bytes
 */
#define MODBUS_RTU_MAX_ADU_LENGTH  256

MODBUS_API modbus_t* modbus_new_rtu(const char *device, int baud, char parity,
                                    int data_bit, int stop_bit);

/* Deprecated */
#define MODBUS_RTU_RS232 MODBUS_SERIAL_RS232
#define MODBUS_RTU_RS485 MODBUS_SERIAL_RS485

#define modbus_rtu_set_serial_mode(pctx, mode) modbus_serial_set_serial_mode(pctx, mode)
#define modbus_rtu_get_serial_mode(pctx) modbus_serial_get_serial_mode(pctx)

MODBUS_END_DECLS

#endif /* MODBUS_RTU_H */
