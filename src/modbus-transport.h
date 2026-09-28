/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MODBUS_TRANSPORT_H
#define MODBUS_TRANSPORT_H

/* clang-format off */
#ifndef _MSC_VER
# include <sys/time.h>
# include <sys/types.h>
# include <stdint.h>
#else
# include "stdint.h"
# include <time.h>
typedef int ssize_t;
#endif
/* clang-format on */

#include "modbus.h"

MODBUS_BEGIN_DECLS

/* Pluggable I/O transport: replaces the low-level send, recv, select, connect,
 * close and flush calls while leaving the Modbus framing, CRC and protocol
 * logic unchanged. Every function pointer is optional; a NULL pointer falls
 * back to the default backend behaviour for that operation. See
 * modbus_set_transport(3). */
typedef struct modbus_transport {
    int (*connect)(struct modbus_transport *t);
    ssize_t (*send)(struct modbus_transport *t, const uint8_t *buf, int len);
    ssize_t (*recv)(struct modbus_transport *t, uint8_t *buf, int len);
    int (*select)(struct modbus_transport *t, struct timeval *tv);
    int (*flush)(struct modbus_transport *t);
    void (*close)(struct modbus_transport *t);
    void (*free)(struct modbus_transport *t);
    void *priv;    /* Private transport state, not touched by libmodbus */
    int connected; /* Managed by modbus_connect()/modbus_close() */
} modbus_transport_t;

MODBUS_API int modbus_set_transport(modbus_t *ctx, modbus_transport_t *transport);
MODBUS_API modbus_transport_t *modbus_get_transport(modbus_t *ctx);

MODBUS_END_DECLS

#endif /* MODBUS_TRANSPORT_H */
