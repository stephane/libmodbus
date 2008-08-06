/*
 * Copyright © 2008 Stéphane Raimbault <stephane.raimbault@gmail.com>
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

#include <modbus/modbus.h>

#include "unit-test.h"

int main(void)
{
        int socket;
        modbus_param_t mb_param;
        modbus_mapping_t mb_mapping;
        int ret;
        int i;

        modbus_init_tcp(&mb_param, "127.0.0.1", 1502);
        modbus_set_debug(&mb_param, TRUE);

        ret = modbus_mapping_new(&mb_mapping,
                                 UT_COIL_STATUS_ADDRESS + UT_COIL_STATUS_NB_POINTS,
                                 UT_INPUT_STATUS_ADDRESS + UT_INPUT_STATUS_NB_POINTS,
                                 UT_HOLDING_REGISTERS_ADDRESS + UT_HOLDING_REGISTERS_NB_POINTS,
                                 UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB_POINTS);
        if (ret == FALSE) {
                printf("Memory allocation failed\n");
                exit(1);
        }

        /* Examples from PI_MODBUS_300.pdf.
           Only the read-only input values are assigned. */
        
        /** INPUT STATUS **/
        set_bits_from_bytes(mb_mapping.tab_input_status,
                            UT_INPUT_STATUS_ADDRESS, UT_INPUT_STATUS_NB_POINTS,
                            UT_INPUT_STATUS_TAB);

        /** INPUT REGISTERS **/
        for (i=0; i < UT_INPUT_REGISTERS_NB_POINTS; i++) {
                mb_mapping.tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
                        UT_INPUT_REGISTERS_TAB[i];;
        }

        socket = modbus_init_listen_tcp(&mb_param);
        
        while (1) {
                uint8_t query[MAX_MESSAGE_LENGTH];
                int query_size;
                
                ret = modbus_listen(&mb_param, query, &query_size);
                if (ret == 0) {
                        if (((query[HEADER_LENGTH_TCP + 4] << 8) + query[HEADER_LENGTH_TCP + 5])
                            == UT_HOLDING_REGISTERS_NB_POINTS_SPECIAL) {
                                /* Change the number of values (offset
                                   TCP = 6) */
                                query[HEADER_LENGTH_TCP + 4] = 0;
                                query[HEADER_LENGTH_TCP + 5] = UT_HOLDING_REGISTERS_NB_POINTS;
                        }

                        modbus_manage_query(&mb_param, query, query_size, &mb_mapping);
                } else if (ret == CONNECTION_CLOSED) {
                        /* Connection closed by the client, end of server */
                        break;
                } else {
                        printf("Error in modbus_listen (%d)\n", ret);
                }
        }

        close(socket);
        modbus_mapping_free(&mb_mapping);
        modbus_close(&mb_param);
        
        return 0;
}
        
