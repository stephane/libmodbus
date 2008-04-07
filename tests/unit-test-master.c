/*
 * Copyright (C) 2001-2008 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

/* Tests based on PI-MBUS-300 documentation */
#define SLAVE    0x11

int main(void)
{
        uint8_t *tab_rp_status;
        uint16_t *tab_rp_registers;
        modbus_param_t mb_param;
        int i;
        uint8_t value;
        uint16_t address;
        uint16_t nb_points;

        /* RTU parity : none, even, odd */
/*      modbus_init_rtu(&mb_param, "/dev/ttyS0", 19200, "none", 8, 1); */

        /* TCP */
        modbus_init_tcp(&mb_param, "127.0.0.1", 1502);
        modbus_set_debug(&mb_param, TRUE);
      
        modbus_connect(&mb_param);

        /* Allocate and initialize the memory to store the status */
        nb_points = (UT_COIL_STATUS_NB_POINTS > UT_INPUT_STATUS_NB_POINTS) ?
                UT_COIL_STATUS_NB_POINTS : UT_INPUT_STATUS_NB_POINTS;
        tab_rp_status = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
        memset(tab_rp_status, 0, nb_points * sizeof(uint8_t));
        
        /* Allocate and initialize the memory to store the registers */
        nb_points = (UT_HOLDING_REGISTERS_NB_POINTS > UT_INPUT_REGISTERS_NB_POINTS) ?
                UT_HOLDING_REGISTERS_NB_POINTS : UT_INPUT_REGISTERS_NB_POINTS;
        tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
        memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

        printf("Unit testing\n");

        /** COIL STATUS **/
        read_coil_status(&mb_param, SLAVE, UT_COIL_STATUS_ADDRESS,
                         UT_COIL_STATUS_NB_POINTS, tab_rp_status);

        i = 0;
        address = UT_COIL_STATUS_ADDRESS;
        nb_points = UT_COIL_STATUS_NB_POINTS;
        while (nb_points > 0) {
                int nb_bits = (nb_points > 8) ? 8 : nb_points;

                value = get_byte_from_bits(tab_rp_status, i*8, nb_bits);
                if (value != UT_COIL_STATUS_TAB[i]) {
                        printf("Coil status: FAILED (%0X != %0X)\n",
                               value, UT_COIL_STATUS_TAB[i]);
                        goto close;
                }

                nb_points -= nb_bits;
                i++;
        }
        printf("Coil status: OK\n");


        /** INPUT STATUS **/
        read_input_status(&mb_param, SLAVE, UT_INPUT_STATUS_ADDRESS,
                          UT_INPUT_STATUS_NB_POINTS, tab_rp_status);

        i = 0;
        address = UT_INPUT_STATUS_ADDRESS;
        nb_points = UT_INPUT_STATUS_NB_POINTS;
        while (nb_points > 0) {
                int nb_bits = (nb_points > 8) ? 8 : nb_points;

                value = get_byte_from_bits(tab_rp_status, i*8, nb_bits);
                if (value != UT_INPUT_STATUS_TAB[i]) {
                        printf("Input status: FAILED (%0X != %0X)\n",
                               value, UT_INPUT_STATUS_TAB[i]);
                        goto close;
                }

                nb_points -= nb_bits;
                i++;
        }
        printf("Input status: OK\n");

        /** HOLDING REGISTERS **/
        read_holding_registers(&mb_param, SLAVE, UT_HOLDING_REGISTERS_ADDRESS,
                               UT_HOLDING_REGISTERS_NB_POINTS, tab_rp_registers);
        for (i=0; i < UT_HOLDING_REGISTERS_NB_POINTS; i++) {
                if (tab_rp_registers[i] != UT_HOLDING_REGISTERS_TAB[i]) {
                        printf("Holding registers: FAILED (%0X != %0X)\n",
                               tab_rp_registers[i], UT_HOLDING_REGISTERS_TAB[i]);
                        goto close;
                }
        }
        printf("Holding registers: OK\n");

        /** INPUT REGISTERS **/
        read_input_registers(&mb_param, SLAVE, UT_INPUT_REGISTERS_ADDRESS,
                               UT_INPUT_REGISTERS_NB_POINTS, tab_rp_registers);
        for (i=0; i < UT_INPUT_REGISTERS_NB_POINTS; i++) {
                if (tab_rp_registers[i] != UT_INPUT_REGISTERS_TAB[i]) {
                        printf("Input registers: FAILED (%0X != %0X)\n",
                               tab_rp_registers[i], UT_INPUT_REGISTERS_TAB[i]);
                        goto close;
                }
        }
        printf("Input registers: OK\n");


        /** ILLEGAL DATA ADDRESSES */
        read_coil_status(&mb_param, SLAVE, UT_COIL_STATUS_ADDRESS + UT_COIL_STATUS_NB_POINTS + 1,
                         UT_COIL_STATUS_NB_POINTS, tab_rp_status);
        
close:
        /* Free the memory */
        free(tab_rp_status);                                           
        free(tab_rp_registers);

        /* Close the connection */
        modbus_close(&mb_param);
        
        return 0;
/*
        force_single_coil(&mb_param, SLAVE, 0xAC, ON);
*/
}
