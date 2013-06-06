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
#include <assert.h>
#include <modbus.h>

#include "unit-test.h"



int main(int argc, char*argv[])
{
    int rc = 0;
    int backend = -1;
    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            backend = TCP;
        } else if (strcmp(argv[1], "tcppi") == 0) {
            backend = TCP_PI;
        } else if (strcmp(argv[1], "rtu") == 0) {
            backend = RTU;
        } else {
            printf("Usage:\n  %s [tcp|tcppi|rtu] - Modbus server for unit testing\n\n", argv[0]);
            return -1;
        }
    } else {
        /* By default */
        backend = TCP;
    }

    {
        unit_test_server_t server;
        server.use_backend = backend;
        rc = unit_test_server_listen(&server);
        if(rc)goto error;
        rc = unit_test_server_run(&server);

    }
error:
    return rc;
}


