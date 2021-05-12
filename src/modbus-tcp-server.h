/*
 * Copyright © 2016 DEIF A/S
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_TCP_SERVER_H
#define MODBUS_TCP_SERVER_H
#include <stdint.h>
#include "modbus.h"

#define MB_TCP_SRV_BLOCKING_TIMEOUT 0xFFFFFFFF // Use with modbus_tcp_server_set_select_timeout()

typedef struct _modbus_tcp_server modbus_tcp_server_t;

MODBUS_BEGIN_DECLS

MODBUS_API modbus_tcp_server_t* modbus_tcp_server_start(char* ipaddr, uint16_t port, uint16_t max_connections);
MODBUS_API int modbus_tcp_server_stop(modbus_tcp_server_t* mb_srv_ctx);
MODBUS_API int modbus_tcp_server_handle(modbus_tcp_server_t* mb_srv_ctx, modbus_mapping_t* mb_map);

MODBUS_API int modbus_tcp_server_set_select_timeout(modbus_tcp_server_t* mb_srv_ctx, uint32_t to_sec, uint32_t to_usec);

MODBUS_END_DECLS

#endif /* MODBUS_TCP_H */
