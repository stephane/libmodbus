/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 * Copyright © 2010,2011 noris network AG <http://noris.net/>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#if defined(_WIN32)
# define OS_WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# define SHUT_RDWR 2
#else
# include <sys/socket.h>
# include <sys/ioctl.h>

#if defined(OpenBSD) || (defined(__FreeBSD__) && __FreeBSD__ < 5)
# define OS_BSD
# include <netinet/in_systm.h>
#endif

# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
#endif

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

/* The "MODBUS_LEGACY" define is required so the header file declares
 * modbus_new_tcp(), which is implemented below. */
#define MODBUS_LEGACY 1

#include "modbus-private.h"

#include "modbus-tcp.h"
#include "modbus-tcp-private.h"

#ifdef OS_WIN32
static int _modbus_tcp_init_win32(void)
{
    // Initialise Win32 Socket API
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 0);
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        fprintf (stderr, "WSAStartup() returned error code %d\n",
                 GetLastError());
        errno = EIO;
        return -1;
    }
    return 0;
}
#endif

static int _modbus_set_slave(modbus_t *ctx, int slave)
{
    /* Broadcast address is 0 (MODBUS_BROADCAST_ADDRESS) */
    if (slave >= 0 && slave <= 247) {
        ctx->slave = slave;
    } else if (slave == MODBUS_TCP_SLAVE) {
        /* The special value MODBUS_TCP_SLAVE (0xFF) can be used in TCP mode to
         * restore the default value. */
        ctx->slave = slave;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

/* Builds a TCP request header */
int _modbus_tcp_build_request_basis(modbus_t *ctx, int function,
                                    int addr, int nb,
                                    uint8_t *req)
{

    /* Extract from MODBUS Messaging on TCP/IP Implementation Guide V1.0b
       (page 23/46):
       The transaction identifier is used to associate the future response
       with the request. So, at a time, on a TCP connection, this identifier
       must be unique. */
    static uint16_t t_id = 0;

    /* Transaction ID */
    if (t_id < UINT16_MAX)
        t_id++;
    else
        t_id = 0;
    req[0] = t_id >> 8;
    req[1] = t_id & 0x00ff;

    /* Protocol Modbus */
    req[2] = 0;
    req[3] = 0;

    /* Length will be defined later by set_req_length_tcp at offsets 4
       and 5 */

    req[6] = ctx->slave;
    req[7] = function;
    req[8] = addr >> 8;
    req[9] = addr & 0x00ff;
    req[10] = nb >> 8;
    req[11] = nb & 0x00ff;

    return _MODBUS_TCP_PRESET_REQ_LENGTH;
}

/* Builds a TCP response header */
int _modbus_tcp_build_response_basis(sft_t *sft, uint8_t *rsp)
{
    /* Extract from MODBUS Messaging on TCP/IP Implementation
       Guide V1.0b (page 23/46):
       The transaction identifier is used to associate the future
       response with the request. */
    rsp[0] = sft->t_id >> 8;
    rsp[1] = sft->t_id & 0x00ff;

    /* Protocol Modbus */
    rsp[2] = 0;
    rsp[3] = 0;

    /* Length will be set later by send_msg (4 and 5) */

    /* The slave ID is copied from the indication */
    rsp[6] = sft->slave;
    rsp[7] = sft->function;

    return _MODBUS_TCP_PRESET_RSP_LENGTH;
}


int _modbus_tcp_prepare_response_tid(const uint8_t *req, int *req_length)
{
    return (req[0] << 8) + req[1];
}

int _modbus_tcp_send_msg_pre(uint8_t *req, int req_length)
{
    /* Substract the header length to the message length */
    int mbap_length = req_length - 6;

    req[4] = mbap_length >> 8;
    req[5] = mbap_length & 0x00FF;

    return req_length;
}

ssize_t _modbus_tcp_send(modbus_t *ctx, const uint8_t *req, int req_length)
{
    /* MSG_NOSIGNAL
       Requests not to send SIGPIPE on errors on stream oriented
       sockets when the other end breaks the connection.  The EPIPE
       error is still returned. */
    return send(ctx->s, (const char*)req, req_length, MSG_NOSIGNAL);
}

ssize_t _modbus_tcp_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length) {
    return recv(ctx->s, (char *)rsp, rsp_length, 0);
}

int _modbus_tcp_check_integrity(modbus_t *ctx, uint8_t *msg, const int msg_length)
{
    return msg_length;
}

/* Establishes a modbus TCP connection with a Modbus server. */
static int _modbus_tcp_connect(modbus_t *ctx)
{
    modbus_tcp_t *ctx_tcp = ctx->backend_data;
    struct addrinfo *ai_list;
    struct addrinfo *ai_ptr;
    struct addrinfo  ai_hints;
    int rc;

    if (ctx_tcp->node == NULL) {
        return -1;
    }

#ifdef OS_WIN32
    if (_modbus_tcp_init_win32() == -1) {
        return -1;
    }
#endif

    memset (&ai_hints, 0, sizeof (ai_hints));
#ifdef AI_ADDRCONFIG
    ai_hints.ai_flags |= AI_ADDRCONFIG;
#endif
    ai_hints.ai_family = AF_UNSPEC;
    ai_hints.ai_socktype = SOCK_STREAM;
    ai_hints.ai_protocol = IPPROTO_TCP;

    ai_list = NULL;
    rc = getaddrinfo (ctx_tcp->node, ctx_tcp->service,
            &ai_hints, &ai_list);
    if (rc != 0)
        return rc;

    ctx->s = -1;
    for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next)
    {
        int option;

        ctx->s = socket (ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
        if (ctx->s < 0) {
            continue;
        }

        /* Set the TCP no delay flag */
        /* SOL_TCP = IPPROTO_TCP */
        option = 1;
        rc = setsockopt(ctx->s, IPPROTO_TCP, TCP_NODELAY,
                (const void *)&option, sizeof(int));
        if (rc != 0) {
            close(ctx->s);
            ctx->s = -1;
            continue;
        }

#ifndef OS_WIN32
        /**
         * Cygwin defines IPTOS_LOWDELAY but can't handle that flag so it's
         * necessary to workaround that problem.
         **/
        /* Set the IP low delay option */
        option = IPTOS_LOWDELAY;
        rc = setsockopt(ctx->s, IPPROTO_IP, IP_TOS,
                (const void *)&option, sizeof(int));
        if (rc != 0) {
            close(ctx->s);
            ctx->s = -1;
            continue;
        }
#endif

        if (ctx->debug) {
            printf("Connecting to %s:%s\n",
                    ctx_tcp->node, ctx_tcp->service);
        }

        rc = connect(ctx->s, ai_ptr->ai_addr, ai_ptr->ai_addrlen);
        if (rc != 0) {
            close(ctx->s);
            ctx->s = -1;
            continue;
        }

        break;
    } /* for (ai_list) */

    freeaddrinfo (ai_list);

    if (ctx->s < 0) {
        return (-1);
    }

    return 0;
}

/* Closes the network connection and socket in TCP mode */
void _modbus_tcp_close(modbus_t *ctx)
{
    shutdown(ctx->s, SHUT_RDWR);
    close(ctx->s);
}

int _modbus_tcp_flush(modbus_t *ctx)
{
    int rc;

    do {
        /* Extract the garbage from the socket */
        char devnull[MODBUS_TCP_MAX_ADU_LENGTH];
#ifndef OS_WIN32
        rc = recv(ctx->s, devnull, MODBUS_TCP_MAX_ADU_LENGTH, MSG_DONTWAIT);
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

        rc = recv(ctx->s, devnull, MODBUS_TCP_MAX_ADU_LENGTH, 0);
#endif
        if (ctx->debug && rc != -1) {
            printf("\n%d bytes flushed\n", rc);
        }
    } while (rc > 0);

    return rc;
}

/* Listens for any request from one or many modbus masters in TCP */
int modbus_tcp_listen(modbus_t *ctx, int nb_connection)
{
    modbus_tcp_t *ctx_tcp = ctx->backend_data;
    struct addrinfo *ai_list;
    struct addrinfo *ai_ptr;
    struct addrinfo  ai_hints;
    int new_socket;
    int yes;
    int status;

#ifdef OS_WIN32
    if (_modbus_tcp_init_win32() == -1) {
        return -1;
    }
#endif

    memset (&ai_hints, 0, sizeof (ai_hints));
    ai_hints.ai_flags = AI_PASSIVE;
#ifdef AI_ADDRCONFIG
    ai_hints.ai_flags |= AI_ADDRCONFIG;
#endif
    ai_hints.ai_family = AF_UNSPEC;
    ai_hints.ai_socktype = SOCK_STREAM;
    ai_hints.ai_protocol = IPPROTO_TCP;

    ai_list = NULL;
    status = getaddrinfo (ctx_tcp->node,
            (ctx_tcp->service != NULL) ? ctx_tcp->service : "502",
            &ai_hints, &ai_list);
    if (status != 0) {
        return -1;
    }

    new_socket = -1;
    for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next)
    {
        new_socket = socket (ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
        if (new_socket < 0) {
            continue;
        }

        yes = 1;
        status = setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR,
                    (char *) &yes, sizeof(yes));
        if (status != 0) {
            close(new_socket);
            new_socket = -1;
            continue;
        }

        status = bind(new_socket, ai_ptr->ai_addr, ai_ptr->ai_addrlen);
        if (status != 0) {
            close(new_socket);
            new_socket = -1;
            continue;
        }

        status = listen(new_socket, nb_connection);
        if (status != 0) {
            close(new_socket);
            new_socket = -1;
            continue;
        }

        break;
    } /* for (ai_list) */

    freeaddrinfo (ai_list);

    if (new_socket < 0) {
        return -1;
    }

    return new_socket;
}

/* On success, the function return a non-negative integer that is a descriptor
   for the accepted socket. On error, -1 is returned, and errno is set
   appropriately. */
int modbus_tcp_accept(modbus_t *ctx, int *socket)
{
    struct sockaddr_in addr;
    socklen_t addrlen;

    addrlen = sizeof(struct sockaddr_in);
    ctx->s = accept(*socket, (struct sockaddr *)&addr, &addrlen);
    if (ctx->s == -1) {
        close(*socket);
        *socket = 0;
        return -1;
    }

    if (ctx->debug) {
        printf("The client %s is connected\n", inet_ntoa(addr.sin_addr));
    }

    return ctx->s;
}

int _modbus_tcp_select(modbus_t *ctx, fd_set *rfds, struct timeval *tv, int length_to_read)
{
    int s_rc;
    while ((s_rc = select(ctx->s+1, rfds, NULL, NULL, tv)) == -1) {
        if (errno == EINTR) {
            if (ctx->debug) {
                fprintf(stderr, "A non blocked signal was caught\n");
            }
            /* Necessary after an error */
            FD_ZERO(rfds);
            FD_SET(ctx->s, rfds);
        } else {
            _error_print(ctx, "select");
            if (ctx->error_recovery && (errno == EBADF)) {
                modbus_close(ctx);
                modbus_connect(ctx);
                errno = EBADF;
                return -1;
            } else {
                return -1;
            }
        }
    }

    if (s_rc == 0) {
        errno = ETIMEDOUT;
        _error_print(ctx, "select");
        return -1;
    }

    return s_rc;
}

int _modbus_tcp_filter_request(modbus_t *ctx, int slave)
{
    return 0;
}

const modbus_backend_t _modbus_tcp_backend = {
    _MODBUS_BACKEND_TYPE_TCP,
    _MODBUS_TCP_HEADER_LENGTH,
    _MODBUS_TCP_CHECKSUM_LENGTH,
    MODBUS_TCP_MAX_ADU_LENGTH,
    _modbus_set_slave,
    _modbus_tcp_build_request_basis,
    _modbus_tcp_build_response_basis,
    _modbus_tcp_prepare_response_tid,
    _modbus_tcp_send_msg_pre,
    _modbus_tcp_send,
    _modbus_tcp_recv,
    _modbus_tcp_check_integrity,
    _modbus_tcp_connect,
    _modbus_tcp_close,
    _modbus_tcp_flush,
    _modbus_tcp_select,
    _modbus_tcp_filter_request
};


/* Allocates and initializes the modbus_t structure for TCP.
   - ip : "192.168.0.5"
   - port : 1099

   Set the port to MODBUS_TCP_DEFAULT_PORT to use the default one
   (502). It's convenient to use a port number greater than or equal
   to 1024 because it's not necessary to be root to use this port
   number.
*/
modbus_t* modbus_new_tcp(const char *ip_address, int port)
{
    char service[8];

    if ((port <= 0) || (port > 65535))
        port = MODBUS_TCP_DEFAULT_PORT;

    snprintf (service, sizeof (service), "%i", port);
    service[sizeof (service) - 1] = 0;

    return (modbus_new_tcp_pi (ip_address, service));
}

modbus_t* modbus_new_tcp_pi(const char *node, const char *service)
{
    modbus_t *ctx;
    modbus_tcp_t *ctx_tcp;

    if (service == NULL)
        service = MODBUS_TCP_DEFAULT_SERVICE;

    ctx = (modbus_t *) malloc(sizeof(modbus_t));
    _modbus_init_common(ctx);

    /* Could be changed after to reach a remote serial Modbus device */
    ctx->slave = MODBUS_TCP_SLAVE;

    ctx->backend = &(_modbus_tcp_backend);

    ctx->backend_data = (modbus_tcp_t *) malloc(sizeof(modbus_tcp_t));
    ctx_tcp = (modbus_tcp_t *)ctx->backend_data;

    if (node != NULL) {
        ctx_tcp->node = strdup (node);
        if (ctx_tcp->node == NULL) {
            free (ctx->backend_data);
            free (ctx);
            return NULL;
        }
    } else {
        ctx_tcp->node = NULL;
    }

    ctx_tcp->service = strdup (service);
    if (ctx_tcp->service == NULL) {
        free (ctx_tcp->node);
        free (ctx->backend_data);
        free (ctx);
        return NULL;
    }

    return ctx;
}
