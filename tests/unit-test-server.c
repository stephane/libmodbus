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
#include <modbus.h>

#include "unit-test.h"

/* Copied from modbus-private.h */
#define HEADER_LENGTH_TCP 7

enum {
    TCP,
    RTU
};

int main(int argc, char*argv[])
{
    int socket;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    int rc;
    int i;
    int use_backend;

    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            use_backend = TCP;
        } else if (strcmp(argv[1], "rtu") == 0) {
            use_backend = RTU;
        } else {
            printf("Usage:\n  %s [tcp|rtu] - Modbus server for unit testing\n\n", argv[0]);
            return -1;
        }
    } else {
        /* By default */
        use_backend = TCP;
    }

    if (use_backend == TCP) {
        ctx = modbus_new_tcp("127.0.0.1", 1502);
    } else {
        ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
        modbus_set_slave(ctx, SERVER_ID);
    }

    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx, TRUE);

    mb_mapping = modbus_mapping_new(
        UT_BITS_ADDRESS + UT_BITS_NB_POINTS,
        UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB_POINTS,
        UT_REGISTERS_ADDRESS + UT_REGISTERS_NB_POINTS,
        UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB_POINTS);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Examples from PI_MODBUS_300.pdf.
       Only the read-only input values are assigned. */

    /** INPUT STATUS **/
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits,
                               UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB_POINTS,
                               UT_INPUT_BITS_TAB);

    /** INPUT REGISTERS **/
    for (i=0; i < UT_INPUT_REGISTERS_NB_POINTS; i++) {
        mb_mapping->tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
            UT_INPUT_REGISTERS_TAB[i];;
    }

    if (use_backend == TCP) {
        socket = modbus_tcp_listen(ctx, 1);
        modbus_tcp_accept(ctx, &socket);
    } else {
        rc = modbus_connect(ctx);
        if (rc == -1) {
            fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;
        }
    }

    for (;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

        rc = modbus_receive(ctx, -1, query);
        if (rc > 0) {
            if (((query[HEADER_LENGTH_TCP + 3] << 8) +
                 query[HEADER_LENGTH_TCP + 4])
                == UT_REGISTERS_NB_POINTS_SPECIAL) {
                /* Change the number of values (offset
                   TCP = 6) */
                query[HEADER_LENGTH_TCP + 3] = 0;
                query[HEADER_LENGTH_TCP + 4] = UT_REGISTERS_NB_POINTS;
            }

            rc = modbus_reply(ctx, query, rc, mb_mapping);
            if (rc == -1) {
                return -1;
            }
        } else {
            /* Connection closed by the client or error */
            break;
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (use_backend == TCP) {
        close(socket);
    }
    modbus_mapping_free(mb_mapping);
    modbus_free(ctx);

    return 0;
}
