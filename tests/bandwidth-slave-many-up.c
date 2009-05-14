/*
 * Copyright © 2009 Stéphane Raimbault <stephane.raimbault@gmail.com>
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

#include "modbus.h"

#define SLAVE         0x11
#define NB_CONNECTION    5

int slave_socket;
modbus_mapping_t mb_mapping;

static void close_sigint(int dummy)
{
        shutdown(slave_socket, SHUT_RDWR);
        close(slave_socket);
        modbus_mapping_free(&mb_mapping);

	exit(dummy);
}

int main(void)
{
        int master_socket;
        modbus_param_t mb_param;
        int ret;
        fd_set refset;
        fd_set rdset;

        /* Maximum file descriptor number */
        int fdmax;

        modbus_init_tcp(&mb_param, "127.0.0.1", 1502, SLAVE);

        ret = modbus_mapping_new(&mb_mapping,  MAX_STATUS, 0, MAX_REGISTERS, 0);
        if (ret < 0) {
                printf("Memory allocation failure\n");
                exit(1);
        }

        slave_socket = modbus_slave_listen_tcp(&mb_param, NB_CONNECTION);

        signal(SIGINT, close_sigint);

        /* Clear the reference set of socket */
        FD_ZERO(&refset);
        /* Add the slave socket */
        FD_SET(slave_socket, &refset);

        /* Keep track of the max file descriptor */
        fdmax = slave_socket;

        for (;;) {
                rdset = refset;
                if (select(fdmax+1, &rdset, NULL, NULL, NULL) == -1) {
                        perror("Slave select() failure.");
                        close_sigint(1);
                }

                /* Run through the existing connections looking for data to be
                 * read */
                for (master_socket = 0; master_socket <= fdmax; master_socket++) {

                        if (FD_ISSET(master_socket, &rdset)) {
                                if (master_socket == slave_socket) {
                                        /* A client is asking a new connection */
                                        socklen_t addrlen;
                                        struct sockaddr_in clientaddr;
                                        int newfd;

                                        /* Handle new connections */
                                        addrlen = sizeof(clientaddr);
                                        memset(&clientaddr, 0, sizeof(clientaddr));
                                        newfd = accept(slave_socket, (struct sockaddr *)&clientaddr, &addrlen);
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
                                        uint8_t query[MAX_MESSAGE_LENGTH];
                                        int query_size;

                                        ret = modbus_slave_receive(&mb_param, master_socket, query, &query_size);
                                        if (ret == 0) {
                                                modbus_slave_manage(&mb_param, query, query_size, &mb_mapping);
                                        } else {
                                                /* Connection closed by the client, end of server */
                                                printf("Connection closed on socket %d\n", master_socket);
                                                shutdown(master_socket, SHUT_RDWR);
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
