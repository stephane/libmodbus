/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MODBUS_RTU_H_
#define _MODBUS_RTU_H_

#include "modbus.h"

/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * RS232 / RS485 ADU = 253 bytes + slave (1 byte) + CRC (2 bytes) = 256 bytes
 */
#define MODBUS_RTU_MAX_ADU_LENGTH  256

modbus_t* modbus_new_rtu(const char *device, int baud, char parity,
                         int data_bit, int stop_bit);

#if defined(linux)
/* On Linux, we can tell the kernel for RS485 communication */
#define MODBUS_RTU_RS232 0
#define MODBUS_RTU_RS485 1

int modbus_rtu_set_serial_mode(modbus_t *ctx, int mode);
int modbus_rtu_get_serial_mode(modbus_t *ctx);
#endif

#endif /* _MODBUS_RTU_H_ */
