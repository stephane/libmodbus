/*
 * Copyright © 2009-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
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
int server_socket;
modbus_mapping_t *mb_mapping;

static void close_sigint(int dummy)
{
    close(server_socket);
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}

int main(void)
{
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;

    /* Maximum file descriptor number */
    int fdmax;

    ctx = modbus_new_tcp("127.0.0.1", 1502);

    mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
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

            if (FD_ISSET(master_socket, &rdset)) {
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
                    /* An already connected master has sent a new query */
                    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

                    rc = modbus_receive(ctx, master_socket, query);
                    if (rc != -1) {
                        modbus_reply(ctx, query, rc, mb_mapping);
                    } else {
                        /* Connection closed by the client, end of server */
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
    }

    return 0;
}
