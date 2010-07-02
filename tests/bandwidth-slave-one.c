/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
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

#include <modbus/modbus.h>

#define SLAVE 0x11

int main(void)
{
        int socket;
        modbus_param_t mb_param;
        modbus_mapping_t mb_mapping;
        int rc;

        modbus_init_tcp(&mb_param, "127.0.0.1", 1502, SLAVE);

        rc = modbus_mapping_new(&mb_mapping,  MAX_STATUS, 0, MAX_REGISTERS, 0);
        if (rc == -1) {
                fprintf(stderr, "Failed to allocate the mapping: %s\n",
                        modbus_strerror(errno));
                return -1;
        }

        socket = modbus_slave_listen_tcp(&mb_param, 1);
        modbus_slave_accept_tcp(&mb_param, &socket);

        for(;;) {
                uint8_t query[MAX_MESSAGE_LENGTH];

                rc = modbus_slave_receive(&mb_param, -1, query);
                if (rc >= 0) {
                        modbus_slave_manage(&mb_param, query, rc, &mb_mapping);
                } else {
                        /* Connection closed by the client or server */
                        break;
                }
        }

        printf("Quit the loop: %s\n", modbus_strerror(errno));

        close(socket);
        modbus_mapping_free(&mb_mapping);
        modbus_close(&mb_param);

        return 0;
}
