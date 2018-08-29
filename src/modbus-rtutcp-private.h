/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_RTUTCP_PRIVATE_H
#define MODBUS_RTUTCP_PRIVATE_H

#include "modbus-rtu-private.h"
#include "modbus-tcp-private.h"

#define _MODBUS_RTUTCP_HEADER_LENGTH      _MODBUS_RTU_HEADER_LENGTH

#define _MODBUS_RTUTCP_CHECKSUM_LENGTH     _MODBUS_RTU_CHECKSUM_LENGTH

typedef struct _modbus_rtutcp {
    modbus_tcp_t base;
} modbus_rtutcp_t;

typedef struct _modbus_rtutcp_pi {
    modbus_tcp_pi_t base;
} modbus_rtutcp_pi_t;

#endif /* MODBUS_RTUTCP_PRIVATE_H */