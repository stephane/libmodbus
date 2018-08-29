/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#if defined(_WIN32)
# define OS_WIN32
/* ws2_32.dll has getaddrinfo and freeaddrinfo on Windows XP and later.
 * minwg32 headers check WINVER before allowing the use of these */
# ifndef WINVER
#   define WINVER 0x0501
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#if defined(_WIN32)
/* Already set in modbus-tcp.h but it seems order matters in VS2005 */
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <sys/ioctl.h>

#if defined(__OpenBSD__) || (defined(__FreeBSD__) && __FreeBSD__ < 5)
# define OS_BSD
# include <netinet/in_systm.h>
#endif

# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
#endif

#include "modbus-private.h"

#include "modbus-rtutcp.h"
#include "modbus-rtutcp-private.h"

static int _modbus_set_slave(modbus_t *ctx, int slave)
{
    /* Broadcast address is 0 (MODBUS_BROADCAST_ADDRESS) */
    if (slave >= 0 && slave <= 247) {
        ctx->slave = slave;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

static int _modbus_rtutcp_build_request_basis(modbus_t *ctx, int function,
                                    int addr, int nb,
                                    uint8_t *req)
{
    return _modbus_rtu_build_request_basis(ctx, function, addr, nb, req);
}

static int _modbus_rtutcp_build_response_basis(sft_t *sft, uint8_t *rsp)
{
    return _modbus_rtu_build_response_basis(sft, rsp);
}

static int _modbus_rtutcp_prepare_response_tid(const uint8_t *req, int *req_length)
{
    return _modbus_rtu_prepare_response_tid(req, req_length);
}

static int _modbus_rtutcp_send_msg_pre(uint8_t *req, int req_length)
{
    return _modbus_rtu_send_msg_pre(req, req_length);
}

static ssize_t _modbus_rtutcp_send(modbus_t *ctx, const uint8_t *req, int req_length)
{
    return _modbus_tcp_send(ctx, req, req_length);
}

static int _modbus_rtutcp_receive(modbus_t *ctx, uint8_t *req)
{
    return _modbus_tcp_receive(ctx, req);
}

static ssize_t _modbus_rtutcp_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length)
{
    return _modbus_tcp_recv(ctx, rsp, rsp_length);
}

/* The check_crc16 function shall return the message length if the CRC is
   valid. Otherwise it shall return -1 and set errno to EMBADCRC. */
static int _modbus_rtutcp_check_integrity(modbus_t *ctx, uint8_t *msg,
                                const int msg_length)
{
    return _modbus_rtu_check_integrity(ctx, msg, msg_length);
}

static int _modbus_rtutcp_connect(modbus_t *ctx)
{
    return _modbus_tcp_connect(ctx);
}

static int _modbus_rtutcp_pi_connect(modbus_t *ctx)
{
    return _modbus_tcp_pi_connect(ctx);
}

static void _modbus_rtutcp_close(modbus_t *ctx)
{
    _modbus_tcp_close(ctx);
}

static int _modbus_rtutcp_flush(modbus_t *ctx)
{
    int rc;
    int rc_sum = 0;

    do {
        /* Extract the garbage from the socket */
        char devnull[MODBUS_RTUTCP_MAX_ADU_LENGTH];
#ifndef OS_WIN32
        rc = recv(ctx->s, devnull, MODBUS_RTUTCP_MAX_ADU_LENGTH, MSG_DONTWAIT);
#else
        /* On Win32, it's a bit more complicated to not wait */
        fd_set rfds;
        struct timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(ctx->s, &rfds);
        rc = select(ctx->s+1, &rfds, NULL, NULL, &tv);
        if (rc == -1) {
            return -1;
        }

        if (rc == 1) {
            /* There is data to flush */
            rc = recv(ctx->s, devnull, MODBUS_RTUTCP_MAX_ADU_LENGTH, 0);
        }
#endif
        if (rc > 0) {
            rc_sum += rc;
        }
    } while (rc == MODBUS_RTUTCP_MAX_ADU_LENGTH);

    return rc_sum;
}

int modbus_rtutcp_listen(modbus_t *ctx, int nb_connection)
{
    return modbus_tcp_listen(ctx, nb_connection);
}

int modbus_rtutcp_pi_listen(modbus_t *ctx, int nb_connection)
{
    return modbus_tcp_pi_listen(ctx, nb_connection);
}

/* On success, the function return a non-negative integer that is a descriptor
   for the accepted socket. On error, -1 is returned, and errno is set
   appropriately. */
int modbus_rtutcp_accept(modbus_t *ctx, int *socket)
{
    return modbus_tcp_accept(ctx, socket);
}

int modbus_rtutcp_pi_accept(modbus_t *ctx, int *socket)
{
    return modbus_tcp_pi_accept(ctx, socket);
}

static int _modbus_rtutcp_select(modbus_t *ctx, fd_set *rfds, struct timeval *tv, int length_to_read)
{
    return _modbus_tcp_select(ctx, rfds, tv, length_to_read);
}

static void _modbus_rtutcp_free(modbus_t *ctx) {
    free(ctx->backend_data);
    free(ctx);
}

const modbus_backend_t _modbus_rtutcp_backend = {
    _MODBUS_BACKEND_TYPE_RTUTCP,
    _MODBUS_RTUTCP_HEADER_LENGTH,
    _MODBUS_RTUTCP_CHECKSUM_LENGTH,
    MODBUS_RTUTCP_MAX_ADU_LENGTH,
    _modbus_set_slave,
    _modbus_rtutcp_build_request_basis,
    _modbus_rtutcp_build_response_basis,
    _modbus_rtutcp_prepare_response_tid,
    _modbus_rtutcp_send_msg_pre,
    _modbus_rtutcp_send,
    _modbus_rtutcp_receive,
    _modbus_rtutcp_recv,
    _modbus_rtutcp_check_integrity,
    NULL,
    _modbus_rtutcp_connect,
    _modbus_rtutcp_close,
    _modbus_rtutcp_flush,
    _modbus_rtutcp_select,
    _modbus_rtutcp_free
};

const modbus_backend_t _modbus_rtutcp_pi_backend = {
    _MODBUS_BACKEND_TYPE_RTUTCP,
    _MODBUS_RTUTCP_HEADER_LENGTH,
    _MODBUS_RTUTCP_CHECKSUM_LENGTH,
    MODBUS_RTUTCP_MAX_ADU_LENGTH,
    _modbus_set_slave,
    _modbus_rtutcp_build_request_basis,
    _modbus_rtutcp_build_response_basis,
    _modbus_rtutcp_prepare_response_tid,
    _modbus_rtutcp_send_msg_pre,
    _modbus_rtutcp_send,
    _modbus_rtutcp_receive,
    _modbus_rtutcp_recv,
    _modbus_rtutcp_check_integrity,
    NULL,
    _modbus_rtutcp_pi_connect,
    _modbus_rtutcp_close,
    _modbus_rtutcp_flush,
    _modbus_rtutcp_select,
    _modbus_rtutcp_free
};

modbus_t* modbus_new_rtutcp(const char *ip, int port)
{
    modbus_t *ctx;
    modbus_tcp_t *ctx_tcp;
    size_t dest_size;
    size_t ret_size;

#if defined(OS_BSD)
    /* MSG_NOSIGNAL is unsupported on *BSD so we install an ignore
       handler for SIGPIPE. */
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) < 0) {
        /* The debug flag can't be set here... */
        fprintf(stderr, "Could not install SIGPIPE handler.\n");
        return NULL;
    }
#endif

    ctx = (modbus_t *)malloc(sizeof(modbus_t));
    if (ctx == NULL) {
        return NULL;
    }
    _modbus_init_common(ctx);

    ctx->backend = &_modbus_rtutcp_backend;

    ctx->backend_data = (modbus_rtutcp_t *) malloc(sizeof(modbus_rtutcp_t));
    if (ctx->backend_data == NULL) {
        modbus_free(ctx);
        errno = ENOMEM;
        return NULL;
    }
    ctx_tcp = (modbus_tcp_t *)ctx->backend_data;

    if (ip != NULL) {
        dest_size = sizeof(char) * 16;
        ret_size = strlcpy(ctx_tcp->ip, ip, dest_size);
        if (ret_size == 0) {
            fprintf(stderr, "The IP string is empty\n");
            modbus_free(ctx);
            errno = EINVAL;
            return NULL;
        }

        if (ret_size >= dest_size) {
            fprintf(stderr, "The IP string has been truncated\n");
            modbus_free(ctx);
            errno = EINVAL;
            return NULL;
        }
    } else {
        ctx_tcp->ip[0] = '0';
    }
    ctx_tcp->port = port;
    ctx_tcp->t_id = 0;

    return ctx;
}


modbus_t* modbus_new_rtutcp_pi(const char *node, const char *service)
{
    modbus_t *ctx;
    modbus_tcp_pi_t *ctx_tcp_pi;
    size_t dest_size;
    size_t ret_size;

    ctx = (modbus_t *)malloc(sizeof(modbus_t));
    if (ctx == NULL) {
        return NULL;
    }
    _modbus_init_common(ctx);

    ctx->backend = &_modbus_rtutcp_pi_backend;

    ctx->backend_data = (modbus_rtutcp_pi_t *) malloc(sizeof(modbus_rtutcp_pi_t));
    if (ctx->backend_data == NULL) {
        modbus_free(ctx);
        errno = ENOMEM;
        return NULL;
    }
    ctx_tcp_pi = (modbus_tcp_pi_t *)ctx->backend_data;

    if (node == NULL) {
        /* The node argument can be empty to indicate any hosts */
        ctx_tcp_pi->node[0] = 0;
    } else {
        dest_size = sizeof(char) * _MODBUS_TCP_PI_NODE_LENGTH;
        ret_size = strlcpy(ctx_tcp_pi->node, node, dest_size);
        if (ret_size == 0) {
            fprintf(stderr, "The node string is empty\n");
            modbus_free(ctx);
            errno = EINVAL;
            return NULL;
        }

        if (ret_size >= dest_size) {
            fprintf(stderr, "The node string has been truncated\n");
            modbus_free(ctx);
            errno = EINVAL;
            return NULL;
        }
    }

    if (service != NULL) {
        dest_size = sizeof(char) * _MODBUS_TCP_PI_SERVICE_LENGTH;
        ret_size = strlcpy(ctx_tcp_pi->service, service, dest_size);
    } else {
        /* Empty service is not allowed, error catched below. */
        ret_size = 0;
    }

    if (ret_size == 0) {
        fprintf(stderr, "The service string is empty\n");
        modbus_free(ctx);
        errno = EINVAL;
        return NULL;
    }

    if (ret_size >= dest_size) {
        fprintf(stderr, "The service string has been truncated\n");
        modbus_free(ctx);
        errno = EINVAL;
        return NULL;
    }

    ctx_tcp_pi->t_id = 0;

    return ctx;
}
