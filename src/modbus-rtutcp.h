/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_RTUTCP_H
#define MODBUS_RTUTCP_H

#include "modbus.h"

MODBUS_BEGIN_DECLS

#if defined(_WIN32) && !defined(__CYGWIN__)
/* Win32 with MinGW, supplement to <errno.h> */
#include <winsock2.h>
#define ECONNRESET   WSAECONNRESET
#define ECONNREFUSED WSAECONNREFUSED
#define ETIMEDOUT    WSAETIMEDOUT
#define ENOPROTOOPT  WSAENOPROTOOPT
#endif

#define MODBUS_RTUTCP_DEFAULT_PORT   502

/* Modbus_Application_Protocol_V1_1b.pdf Chapter 4 Section 1 Page 5
 * RS232 / RS485 ADU = 253 bytes + slave (1 byte) + CRC (2 bytes) = 256 bytes
 */
#define MODBUS_RTUTCP_MAX_ADU_LENGTH  256

MODBUS_API modbus_t* modbus_new_rtutcp(const char *ip_address, int port);
MODBUS_API int modbus_rtutcp_listen(modbus_t *ctx, int nb_connection);
MODBUS_API int modbus_rtutcp_accept(modbus_t *ctx, int *socket);

MODBUS_API modbus_t* modbus_new_rtutcp_pi(const char *node, const char *service);
MODBUS_API int modbus_rtutcp_pi_listen(modbus_t *ctx, int nb_connection);
MODBUS_API int modbus_rtutcp_pi_accept(modbus_t *ctx, int *socket);

MODBUS_END_DECLS

#endif /* MODBUS_RTUTCP_H */
