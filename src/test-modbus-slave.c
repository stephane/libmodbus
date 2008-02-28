/*
   Copyright (C) 2001-2007 Stéphane Raimbault <stephane.raimbault@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

   These library of functions are designed to enable a program send and
   receive data from a device that communicates using the Modbus protocol.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <modbus/modbus.h>

int main(void)
{
        const int nb_coil_status = 500;
        const int nb_input_status = 500;
        const int nb_input_registers = 500;
        const int nb_holding_registers = 500;
        int socket;
        modbus_param_t mb_param;
        modbus_mapping_t mb_mapping;
        int ret;
        int i;

        modbus_init_tcp(&mb_param, "127.0.0.1");
        modbus_set_debug(&mb_param, TRUE);

        modbus_mapping_new(&mb_mapping,
                           nb_coil_status, nb_input_status, nb_input_registers, nb_holding_registers);

        /* Examples from PI_MODBUS_300.pdf */

        /* Coil status */
        mb_mapping.tab_coil_status[26] = ON;
        mb_mapping.tab_coil_status[25] = ON;
        mb_mapping.tab_coil_status[24] = OFF;
        mb_mapping.tab_coil_status[23] = OFF;
        mb_mapping.tab_coil_status[22] = ON;
        mb_mapping.tab_coil_status[21] = ON;
        mb_mapping.tab_coil_status[20] = OFF;
        mb_mapping.tab_coil_status[19] = ON;

        /* Input status */
        mb_mapping.tab_input_status[203] = ON;
        mb_mapping.tab_input_status[202] = OFF;
        mb_mapping.tab_input_status[201] = ON;
        mb_mapping.tab_input_status[200] = OFF;
        mb_mapping.tab_input_status[199] = ON;
        mb_mapping.tab_input_status[198] = ON;
        mb_mapping.tab_input_status[197] = OFF;
        mb_mapping.tab_input_status[196] = OFF;

        mb_mapping.tab_input_status[211] = ON;
        mb_mapping.tab_input_status[210] = ON;
        mb_mapping.tab_input_status[209] = OFF;
        mb_mapping.tab_input_status[208] = ON;
        mb_mapping.tab_input_status[207] = ON;
        mb_mapping.tab_input_status[206] = OFF;
        mb_mapping.tab_input_status[205] = ON;
        mb_mapping.tab_input_status[204] = ON;

        /* Incomplete byte */
        mb_mapping.tab_input_status[217] = ON;
        mb_mapping.tab_input_status[216] = ON;
        mb_mapping.tab_input_status[215] = OFF;
        mb_mapping.tab_input_status[214] = ON;
        mb_mapping.tab_input_status[213] = OFF;
        mb_mapping.tab_input_status[212] = ON;

        /* Holding registers */
        mb_mapping.tab_holding_registers[107] = 0x022B;
        mb_mapping.tab_holding_registers[108] = 0x0000;
        mb_mapping.tab_holding_registers[109] = 0x0064;

        /* Input registers */
        mb_mapping.tab_input_registers[8] = 0x000A;
 
        socket = modbus_init_listen_tcp(&mb_param);
        
        i = 0;
        while (i++ < 5) {
                unsigned char query[MAX_PACKET_SIZE];
                int query_size;

                ret = modbus_listen(&mb_param, query, &query_size);
                if (ret == 0)
                        manage_query(&mb_param, query, query_size, &mb_mapping);
                else
                        printf("Error in modbus_listen (%d)\n", ret);
        }

        close(socket);
        modbus_mapping_free(&mb_mapping);
        modbus_close(&mb_param);
        
        return 0;
}
        
