/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_SERIAL_H
#define MODBUS_SERIAL_H

#define MODBUS_SERIAL_RS232 0
#define MODBUS_SERIAL_RS485 1

MODBUS_API int modbus_serial_set_serial_mode(modbus_t *ctx, int mode);
MODBUS_API int modbus_serial_get_serial_mode(modbus_t *ctx);

#define MODBUS_SERIAL_RTS_NONE   0
#define MODBUS_SERIAL_RTS_UP     1
#define MODBUS_SERIAL_RTS_DOWN   2

MODBUS_API int modbus_serial_set_rts(modbus_t *ctx, int mode);
MODBUS_API int modbus_serial_get_rts(modbus_t *ctx);

MODBUS_API int modbus_serial_set_custom_rts(modbus_t *ctx, void (*set_rts) (modbus_t *ctx, int on));

MODBUS_API int modbus_serial_set_rts_delay(modbus_t *ctx, int us);
MODBUS_API int modbus_serial_get_rts_delay(modbus_t *ctx);

#endif /* MODBUS_SERIAL_H */
