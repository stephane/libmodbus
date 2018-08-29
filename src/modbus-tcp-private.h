/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef MODBUS_TCP_PRIVATE_H
#define MODBUS_TCP_PRIVATE_H

#define _MODBUS_TCP_HEADER_LENGTH      7
#define _MODBUS_TCP_PRESET_REQ_LENGTH 12
#define _MODBUS_TCP_PRESET_RSP_LENGTH  8

#define _MODBUS_TCP_CHECKSUM_LENGTH    0

/* In both structures, the transaction ID must be placed on first position
   to have a quick access not dependant of the TCP backend */
typedef struct _modbus_tcp {
    /* Extract from MODBUS Messaging on TCP/IP Implementation Guide V1.0b
       (page 23/46):
       The transaction identifier is used to associate the future response
       with the request. This identifier is unique on each TCP connection. */
    uint16_t t_id;
    /* TCP port */
    int port;
    /* IP address */
    char ip[16];
} modbus_tcp_t;

#define _MODBUS_TCP_PI_NODE_LENGTH    1025
#define _MODBUS_TCP_PI_SERVICE_LENGTH   32

typedef struct _modbus_tcp_pi {
    /* Transaction ID */
    uint16_t t_id;
    /* TCP port */
    int port;
    /* Node */
    char node[_MODBUS_TCP_PI_NODE_LENGTH];
    /* Service */
    char service[_MODBUS_TCP_PI_SERVICE_LENGTH];
} modbus_tcp_pi_t;

ssize_t _modbus_tcp_send(modbus_t *ctx, const uint8_t *req, int req_length);
int _modbus_tcp_receive(modbus_t *ctx, uint8_t *req);
ssize_t _modbus_tcp_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length);
int _modbus_tcp_connect(modbus_t *ctx);
int _modbus_tcp_pi_connect(modbus_t *ctx);
void _modbus_tcp_close(modbus_t *ctx);
int _modbus_tcp_select(modbus_t *ctx, fd_set *rfds, struct timeval *tv, int length_to_read);

#endif /* MODBUS_TCP_PRIVATE_H */
