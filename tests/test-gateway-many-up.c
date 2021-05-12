/*
 * Copyright © 2009-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 * Özgür Keleş <ozgurkeles@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <modbus.h>

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define NB_CONNECTION    5

modbus_t *ctx = NULL;
modbus_t *ctx_rtu = NULL;
int server_socket = -1;

static void close_sigint(int dummy)
{
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_free(ctx_rtu);


    exit(dummy);
}

int main(void)
{
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    /* Maximum file descriptor number */
    int fdmax;
    int query_length;
    uint8_t response[MODBUS_RTU_MAX_ADU_LENGTH];
    int data_start_index;
    int raw_length;
    int exception;
    int slave;

    ctx = modbus_new_tcp("127.0.0.1", 1502);

    //ctx_rtu = modbus_new_rtu("/dev/ttyUSB1", 115200, 'N', 8, 1);
    ctx_rtu = modbus_new_rtu("/dev/ttyS0", 9600, 'N', 8, 1);

    if (modbus_connect(ctx_rtu) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx_rtu);
        modbus_free(ctx);
        return -1;
    }

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    for (;;) {
        rdset = refset;
        if (select(fdmax+1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == server_socket) {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
                if (newfd == -1) {
                    perror("Server accept() error");
                } else {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax) {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                }
            } else {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                if (rc > 0) {
                    exception = 0;
                    query_length = rc;
                    data_start_index = modbus_get_header_length(ctx) - 1;
                    slave = query[data_start_index];
                    raw_length = rc - data_start_index - modbus_get_checksum_length(ctx);
                    modbus_flush(ctx_rtu);
                    modbus_set_slave(ctx_rtu, slave);
                    if (modbus_send_raw_request(ctx_rtu, query + data_start_index, raw_length) != -1) {
                        rc = modbus_receive_confirmation(ctx_rtu, response);
                        if ( rc != -1) {
                            /* rc is the response size */
                            data_start_index = modbus_get_header_length(ctx_rtu) - 1;
                            raw_length = rc - data_start_index - modbus_get_checksum_length(ctx_rtu);
                            modbus_reply_raw_response(ctx, query, query_length, response + data_start_index, raw_length);
                        } else {
                            exception = errno;
                        }
                    } else {
                        exception = errno;
                    }

                    if (exception != 0) {
                        if (exception > MODBUS_ENOBASE && MODBUS_ENOBASE < (MODBUS_ENOBASE + MODBUS_EXCEPTION_MAX)) {
                            exception -= MODBUS_ENOBASE;
                        } else {
                            exception = EMBXSFAIL;
                        }
                        modbus_reply_exception(ctx, query, exception);
                    }
                } else if (rc == -1) {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax) {
                        fdmax--;
                    }
                }
            }
        }
    }

    return 0;
}
