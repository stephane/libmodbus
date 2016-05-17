/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_ASCII_PRIVATE_H
#define MODBUS_ASCII_PRIVATE_H

#define _MODBUS_ASCII_HEADER_LENGTH      2
#define _MODBUS_ASCII_PRESET_REQ_LENGTH  7
#define _MODBUS_ASCII_PRESET_RSP_LENGTH  2

/* LRC8 + \r + \n */
#define _MODBUS_ASCII_CHECKSUM_LENGTH    3

#endif /* MODBUS_ASCII_PRIVATE_H */
