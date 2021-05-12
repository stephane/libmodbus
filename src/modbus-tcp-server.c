/*
 * Copyright © 2016 DEIF A/S
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef _MSC_VER
# include <unistd.h>
#endif
#include <time.h>

#if defined(_WIN32)
# define OS_WIN32
/* ws2_32.dll has getaddrinfo and freeaddrinfo on Windows XP and later.
 * minwg32 headers check WINVER before allowing the use of these */
# ifndef WINVER
# define WINVER 0x0501
# endif
/* Already set in modbus-tcp.h but it seems order matters in VS2005 */
# include <winsock2.h>
# include <ws2tcpip.h>
# define SHUT_RDWR 2
# define TIME_UTC 1
# define close closesocket
#else
# include <sys/select.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
#endif

#include "modbus.h"
#include "modbus-private.h"
#include "modbus-tcp-server.h"
#include <pthread.h>
#include <time.h>

#include <modbus.h>

#define MB_TCP_SRV_IDLE_TIMEOUT 60

/* modbus client data */
struct modbus_tcp_client_t {
    int socket;
    time_t last_update;
    struct modbus_tcp_client_t* next;
};

/* mdbus server data */
struct _modbus_tcp_server {
    int server_socket;
    fd_set refset;
    fd_set rdset;
    int16_t fdmax;
    uint16_t max_connections;
    uint16_t port;
    uint32_t idle_time_sec;
    uint32_t select_to_sec;
    uint32_t select_to_usec;
    uint8_t shutdown;

    modbus_t* ctx;
    /* Linked list of all current connections */
    struct modbus_tcp_client_t* conn;
};

static void _modbus_tcp_srv_rm_cli(modbus_tcp_server_t* data,
        struct modbus_tcp_client_t* link) {
    struct modbus_tcp_client_t* sp = NULL;
    struct modbus_tcp_client_t* tmp = NULL;

    for (sp = data->conn; sp != NULL; sp = sp->next) {
        if (sp == link) {
            close(sp->socket);
            FD_CLR(sp->socket, &data->refset);

            /* If we are pointing to HEAD, remove head */
            if (tmp == NULL) {
                data->conn = sp->next;
                free(sp);
                break;
            } else {
                struct modbus_tcp_client_t* tmp_next = sp->next;

                /* check if last link */
                if (tmp_next == NULL) {
                    tmp->next = NULL;
                    free(sp);
                } else {
                    tmp->next = tmp_next;
                    free(sp);
                }
            }
        }
        tmp = sp;
    }
}

static void _modbus_tcp_srv_del_oldest_cli(modbus_tcp_server_t* data) {
    struct modbus_tcp_client_t* cli_to_remove = NULL;
    time_t oldest_time = 0x7FFFFFFF;

    /* Remove client from list of connected clients*/
    struct modbus_tcp_client_t* sp = NULL;
    for (sp = data->conn; sp != NULL; sp = sp->next) {
        if (sp->last_update < oldest_time) {
            cli_to_remove = sp;
            oldest_time = sp->last_update;
        }
    }

    /* If we found one, remove it */
    if (cli_to_remove != NULL) {
        _modbus_tcp_srv_rm_cli(data, cli_to_remove);
    }
}

static void _modbus_tcp_srv_add_cli(modbus_tcp_server_t* data, int socket) {
    struct modbus_tcp_client_t* sp = NULL;
    int connection_nbr = 0;

    /* Handle HEAD */
    if (data->conn == NULL) {
        data->conn = malloc(sizeof(struct modbus_tcp_client_t));
        data->conn->last_update = time(NULL);
        data->conn->socket = socket;
        data->conn->next = NULL;

        /* add to listener*/
        FD_SET(socket, &data->refset);

        if (socket > data->fdmax) {
            /* Keep track of the maximum */
            data->fdmax = socket;
        }
        return;
    }
    connection_nbr++; /* HEAD link */

    /* When we have HEAD, add new links */
    for (sp = data->conn; sp != NULL; sp = sp->next) {
        connection_nbr++;
        if ((sp->socket == socket) || sp->next == NULL) {

            sp->next = malloc(sizeof(struct modbus_tcp_client_t));
            sp->next->last_update = time(NULL);
            sp->next->socket = socket;
            sp->next->next = NULL;

            /* add to listener*/
            FD_SET(socket, &data->refset);

            if (socket > data->fdmax) {
                /** Keep track of the maximum */
                data->fdmax = socket;
            }
            break;
        }

        /* Modbus specification: if no available slots, remove oldest client when new connects */
        if (connection_nbr >= data->max_connections) {
            _modbus_tcp_srv_del_oldest_cli(data);
        }
    }
}

static void _modbus_tcp_server_stop(modbus_tcp_server_t* srv_ctx) {

    struct modbus_tcp_client_t* tmp = NULL;

    if (srv_ctx != NULL) {
        struct modbus_tcp_client_t* cli = srv_ctx->conn;

        /* Close modbus server sockets*/
        close(srv_ctx->server_socket);

        /* Loop though linked list and close + free all client data */
        while (cli != NULL) {
            close(cli->socket);

            tmp = cli;
            cli = cli->next;
            free(tmp);
            tmp = NULL;
        }

        /* Release main context */
        if (srv_ctx->ctx != NULL) {
            modbus_free(srv_ctx->ctx);
            srv_ctx->ctx = NULL;
        }

        /* Release server data */
        if (srv_ctx != NULL) {
            free(srv_ctx);
            srv_ctx = NULL;
        }
    }
}

modbus_tcp_server_t* modbus_tcp_server_start(char* ipaddr, uint16_t port, uint16_t max_connections) {
    modbus_tcp_server_t* data = malloc(sizeof(modbus_tcp_server_t));
    memset(data, 0, sizeof(modbus_tcp_server_t));

    data->max_connections = max_connections;
    data->port = port;
    data->idle_time_sec = MB_TCP_SRV_IDLE_TIMEOUT;
    data->select_to_sec = MB_TCP_SRV_BLOCKING_TIMEOUT;
    data->select_to_usec = 0;

    /* Check select can support the request connections */
    if (data->max_connections > FD_SETSIZE) {
        errno = EFBIG;
        return NULL;
    }

    /* Create new server */
    data->ctx = modbus_new_tcp(ipaddr, port);
    if (data->ctx == NULL) {
        /* libmodbus has set errno */
        free(data);
        return NULL;
    }

    /* Open socket */
    data->server_socket = modbus_tcp_listen(data->ctx, 5);

    if (data->server_socket < 0) {
        modbus_free(data->ctx);
        /* libmodbus has set errno */
        free(data);
        return NULL;
    }

    /* Clear the reference set of socket */
    FD_ZERO(&data->refset);

    /* Add the server socket */
    FD_SET(data->server_socket, &data->refset);

    /* Keep track of the max file descriptor */
    data->fdmax = data->server_socket;
    return data;
}

int modbus_tcp_server_stop(modbus_tcp_server_t* srv_ctx) {

    if (srv_ctx == NULL) {
        return -1;
    }

    srv_ctx->shutdown = 1;
    /* send shutdown event to accept() in handle function */
    shutdown(srv_ctx->server_socket, SHUT_RDWR);
    return 0;
}

int modbus_tcp_server_handle(modbus_tcp_server_t* srv_ctx,
        modbus_mapping_t* mb_map) {
    int retval, rc;

    if ((srv_ctx == NULL) || (srv_ctx->ctx == NULL)) {
        errno = EBADF;
        return -1;
    }

    /* Reset select set and select timeout */
    srv_ctx->rdset = srv_ctx->refset;

    /* Blocking select waiting for connections */
    if (srv_ctx->select_to_sec == MB_TCP_SRV_BLOCKING_TIMEOUT
            || srv_ctx->select_to_usec == MB_TCP_SRV_BLOCKING_TIMEOUT) {
        retval = select(srv_ctx->fdmax + 1, &srv_ctx->rdset, NULL, NULL, NULL);
    } else {
        /* Non-blocking select with timeout waiting for connections */
        struct timeval scan_ms;
        scan_ms.tv_sec = srv_ctx->select_to_sec;
        scan_ms.tv_usec = srv_ctx->select_to_usec;
        retval = select(srv_ctx->fdmax + 1, &srv_ctx->rdset, NULL, NULL,
                &scan_ms);
    }

    /* If the context has been destroyed, bailout */
    if ((srv_ctx == NULL) || (srv_ctx->ctx == NULL)) {
        errno = EBADF;
        return -1;
    }

    if (srv_ctx->shutdown) {
        _modbus_tcp_server_stop(srv_ctx);
        errno = ECONNRESET;
        return -1;
    }

    if (retval == 0) {
        /* timeout, this is OK */
    } else if (retval == -1) ///< Critical error on select, exit
            {
         /* select has set errno */
        modbus_tcp_server_stop(srv_ctx);
        return -1;
    } else {
        /* New connection request */
        if (FD_ISSET(srv_ctx->server_socket, &srv_ctx->rdset)) {
            socklen_t addrlen;
            struct sockaddr_storage clientaddr;
            int newfd;

            /* Handle new connections */
            addrlen = sizeof(clientaddr);
            memset(&clientaddr, 0, sizeof(clientaddr));
            newfd = accept(srv_ctx->server_socket,
                    (struct sockaddr *) &clientaddr, &addrlen);

            /* Debug if needed */
            if (srv_ctx->ctx->debug) {
                char ipstr[INET6_ADDRSTRLEN + 1] = { 0 };
                int port = 0;

                struct sockaddr_in *s = (struct sockaddr_in *) &clientaddr;
                port = ntohs(s->sin_port);
                getnameinfo((struct sockaddr *) &clientaddr, sizeof(clientaddr),
                        ipstr, sizeof(ipstr), NULL, 0, 0);
                fprintf(stderr,
                        "MB TCP server on port %d, Incoming connection from %s -> %d\n",
                        srv_ctx->port, ipstr, port);
            }

            if (newfd == -1) {
                if (srv_ctx->ctx->debug) {
                    perror("Server accept() error");
                    fprintf(stderr, " Socket: %d on port: %d\r\n",
                            srv_ctx->server_socket, srv_ctx->port);
                }
                /* accept has set errno */
                return -1;
            } else {
                _modbus_tcp_srv_add_cli(srv_ctx, newfd);
            }
        }
        /* Data request */
        else {
            struct modbus_tcp_client_t* sp = NULL;

            for (sp = srv_ctx->conn; sp != NULL; sp = sp->next) {
                if (FD_ISSET(sp->socket, &srv_ctx->rdset)) {
                    /* An already connected master has sent a new query */
                    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

                    modbus_set_socket(srv_ctx->ctx, sp->socket);
                    rc = modbus_receive(srv_ctx->ctx, query);

                    if (rc != -1) {
                        modbus_reply(srv_ctx->ctx, query, rc, mb_map);
                        sp->last_update = time(NULL);;
                        /* considering implementing a callback pointer with a registration function
                           which the user can use for a 'request from IP on port has been handled' event. */
                    } else {
                        _modbus_tcp_srv_rm_cli(srv_ctx, sp);
                    }
                }
            }
        }
    }
    return 0;
}

int modbus_tcp_server_set_select_timeout(modbus_tcp_server_t* srv_ctx, uint32_t to_sec, uint32_t to_usec) {

    if (srv_ctx == NULL) {
        errno = EBADF;
        return -1;
    }
    srv_ctx->select_to_sec = to_sec;
    srv_ctx->select_to_usec = to_usec;
    return 0;
}

