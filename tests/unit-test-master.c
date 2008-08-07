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

/* Tests based on PI-MBUS-300 documentation */
#define SLAVE    0x11

int main(void)
{
        uint8_t *tab_rp_status;
        uint16_t *tab_rp_registers;
        modbus_param_t mb_param;
        int i;
        uint8_t value;
        int address;
        int nb_points;
        int ret;

        /* RTU parity : none, even, odd */
/*      modbus_init_rtu(&mb_param, "/dev/ttyS0", 19200, "none", 8, 1); */

        /* TCP */
        modbus_init_tcp(&mb_param, "127.0.0.1", 1502);
/*        modbus_set_debug(&mb_param, TRUE);*/
      
        if (modbus_connect(&mb_param) == -1) {
                printf("ERROR Connection failed\n");
                exit(1);
        }

        /* Allocate and initialize the memory to store the status */
        nb_points = (UT_COIL_STATUS_NB_POINTS > UT_INPUT_STATUS_NB_POINTS) ?
                UT_COIL_STATUS_NB_POINTS : UT_INPUT_STATUS_NB_POINTS;
        tab_rp_status = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
        memset(tab_rp_status, 0, nb_points * sizeof(uint8_t));
        
        /* Allocate and initialize the memory to store the registers */
        nb_points = (UT_HOLDING_REGISTERS_NB_POINTS > 
                     UT_INPUT_REGISTERS_NB_POINTS) ?
                UT_HOLDING_REGISTERS_NB_POINTS : UT_INPUT_REGISTERS_NB_POINTS;
        tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
        memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

        printf("** UNIT TESTING **\n");

        printf("\nTEST WRITE/READ:\n");

        /** COIL STATUS **/

        /* Single */
        ret = force_single_coil(&mb_param, SLAVE, UT_COIL_STATUS_ADDRESS, ON);
        printf("1/2 force_single_coil: ");
        if (ret == 1) {
                printf("OK\n");
        } else {
                printf("FAILED\n");
                goto close;
        }

        ret = read_coil_status(&mb_param, SLAVE, UT_COIL_STATUS_ADDRESS, 1,
                               tab_rp_status);
        printf("2/2 read_coil_status: ");
        if (ret != 1) {
                printf("FAILED (nb points %d)\n", ret); 
                goto close;
        }

        if (tab_rp_status[0] != ON) {
                printf("FAILED (%0X = != %0X)\n", tab_rp_status[0], ON);
                goto close;
        }
        printf("OK\n");
        /* End single */

        /* Multiple coils */
        {
                uint8_t tab_value[UT_COIL_STATUS_NB_POINTS];
                
                set_bits_from_bytes(tab_value, 0, UT_COIL_STATUS_NB_POINTS,
                                    UT_COIL_STATUS_TAB);
                ret = force_multiple_coils(&mb_param, SLAVE,
                                           UT_COIL_STATUS_ADDRESS,
                                           UT_COIL_STATUS_NB_POINTS,
                                           tab_value);
                printf("1/2 force_multiple_coils: ");
                if (ret == UT_COIL_STATUS_NB_POINTS) {
                        printf("OK\n");
                } else {
                        printf("FAILED\n");
                        goto close;
                }
        }

        ret = read_coil_status(&mb_param, SLAVE, UT_COIL_STATUS_ADDRESS,
                               UT_COIL_STATUS_NB_POINTS, tab_rp_status);
        printf("2/2 read_coil_status: ");
        if (ret != UT_COIL_STATUS_NB_POINTS) {
                printf("FAILED (nb points %d)\n", ret); 
                goto close;
        }

        i = 0;
        address = UT_COIL_STATUS_ADDRESS;
        nb_points = UT_COIL_STATUS_NB_POINTS;
        while (nb_points > 0) {
                int nb_bits = (nb_points > 8) ? 8 : nb_points;

                value = get_byte_from_bits(tab_rp_status, i*8, nb_bits);
                if (value != UT_COIL_STATUS_TAB[i]) {
                        printf("FAILED (%0X != %0X)\n",
                               value, UT_COIL_STATUS_TAB[i]);
                        goto close;
                }

                nb_points -= nb_bits;
                i++;
        }
        printf("OK\n");
        /* End of multiple coils */

        /** INPUT STATUS **/
        ret = read_input_status(&mb_param, SLAVE, UT_INPUT_STATUS_ADDRESS,
                                UT_INPUT_STATUS_NB_POINTS, tab_rp_status);
        printf("1/1 read_input_status: ");

        if (ret != UT_INPUT_STATUS_NB_POINTS) {
                printf("FAILED (nb points %d)\n", ret); 
                goto close;
        }

        i = 0;
        address = UT_INPUT_STATUS_ADDRESS;
        nb_points = UT_INPUT_STATUS_NB_POINTS;
        while (nb_points > 0) {
                int nb_bits = (nb_points > 8) ? 8 : nb_points;

                value = get_byte_from_bits(tab_rp_status, i*8, nb_bits);
                if (value != UT_INPUT_STATUS_TAB[i]) {
                        printf("FAILED (%0X != %0X)\n",
                               value, UT_INPUT_STATUS_TAB[i]);
                        goto close;
                }

                nb_points -= nb_bits;
                i++;
        }
        printf("OK\n");

        /** HOLDING REGISTERS **/

        /* Single register */
        ret = preset_single_register(&mb_param, SLAVE,
                                     UT_HOLDING_REGISTERS_ADDRESS, 0x1234);
        printf("1/2 preset_single_register: ");
        if (ret == 1) {
                printf("OK\n");
        } else {
                printf("FAILED\n");
                goto close;
        }

        ret = read_holding_registers(&mb_param, SLAVE,
                                     UT_HOLDING_REGISTERS_ADDRESS,
                                     1, tab_rp_registers);
        printf("2/2 read_holding_registers: ");
        if (ret != 1) {
                printf("FAILED (nb points %d)\n", ret); 
                goto close;
        }

        if (tab_rp_registers[0] != 0x1234) {
                printf("FAILED (%0X != %0X)\n",
                       tab_rp_registers[0], 0x1234);
                goto close;
        }
        printf("OK\n");
        /* End of single register */

        /* Many registers */
        ret = preset_multiple_registers(&mb_param, SLAVE,
                                        UT_HOLDING_REGISTERS_ADDRESS,
                                        UT_HOLDING_REGISTERS_NB_POINTS,
                                        UT_HOLDING_REGISTERS_TAB);
        printf("1/2 preset_multiple_registers: ");
        if (ret == UT_HOLDING_REGISTERS_NB_POINTS) {
                printf("OK\n");
        } else {
                printf("FAILED\n");
                goto close;
        }

        ret = read_holding_registers(&mb_param,
                                     SLAVE, UT_HOLDING_REGISTERS_ADDRESS,
                                     UT_HOLDING_REGISTERS_NB_POINTS,
                                     tab_rp_registers);
        printf("2/2 read_holding_registers: ");
        if (ret != UT_HOLDING_REGISTERS_NB_POINTS) {
                printf("FAILED (nb points %d)\n", ret); 
                goto close;
        }

        for (i=0; i < UT_HOLDING_REGISTERS_NB_POINTS; i++) {
                if (tab_rp_registers[i] != UT_HOLDING_REGISTERS_TAB[i]) {
                        printf("FAILED (%0X != %0X)\n",
                               tab_rp_registers[i],
                               UT_HOLDING_REGISTERS_TAB[i]);
                        goto close;
                }
        }
        printf("OK\n");
        /* End of many registers */


        /** INPUT REGISTERS **/
        ret = read_input_registers(&mb_param,
                                   SLAVE, UT_INPUT_REGISTERS_ADDRESS,
                                   UT_INPUT_REGISTERS_NB_POINTS,
                                   tab_rp_registers);
        printf("1/1 read_input_registers: ");
        if (ret != UT_INPUT_REGISTERS_NB_POINTS) {
                printf("FAILED (nb points %d)\n", ret); 
                goto close;
        }
         
        for (i=0; i < UT_INPUT_REGISTERS_NB_POINTS; i++) {
                if (tab_rp_registers[i] != UT_INPUT_REGISTERS_TAB[i]) {
                        printf("FAILED (%0X != %0X)\n",
                               tab_rp_registers[i], UT_INPUT_REGISTERS_TAB[i]);
                        goto close;
                }
        }
        printf("OK\n");
        

        /** ILLEGAL DATA ADDRESS **/
        printf("\nTEST ILLEGAL DATA ADDRESS:\n");

        /* The mapping begins at 0 and ending at address + nb_points so
         * the addresses below are not valid. */ 
        
        ret = read_coil_status(&mb_param, SLAVE,
                               UT_COIL_STATUS_ADDRESS,
                               UT_COIL_STATUS_NB_POINTS + 1,
                               tab_rp_status);
        printf("* read_coil_status: ");
        if (ret == ILLEGAL_DATA_ADDRESS)
                printf("OK\n");
        else
                printf("FAILED\n");

        ret = read_input_status(&mb_param, SLAVE,
                                UT_INPUT_STATUS_ADDRESS,
                                UT_INPUT_STATUS_NB_POINTS + 1,
                                tab_rp_status);
        printf("* read_input_status: ");
        if (ret == ILLEGAL_DATA_ADDRESS)
                printf("OK\n");
        else
                printf("FAILED\n");

        ret = read_holding_registers(&mb_param, SLAVE,
                                     UT_HOLDING_REGISTERS_ADDRESS,
                                     UT_HOLDING_REGISTERS_NB_POINTS + 1,
                                     tab_rp_registers);
        printf("* read_holding_registers: ");
        if (ret == ILLEGAL_DATA_ADDRESS)
                printf("OK\n");
        else
                printf("FAILED\n");
                
        ret = read_input_registers(&mb_param, SLAVE,
                                   UT_INPUT_REGISTERS_ADDRESS,
                                   UT_INPUT_REGISTERS_NB_POINTS + 1,
                                   tab_rp_registers);
        printf("* read_input_registers: ");
        if (ret == ILLEGAL_DATA_ADDRESS)
                printf("OK\n");
        else
                printf("FAILED\n");

        ret = force_single_coil(&mb_param, SLAVE,
                                UT_COIL_STATUS_ADDRESS + UT_COIL_STATUS_NB_POINTS,
                                ON);
        printf("* force_single_coil: ");
        if (ret == ILLEGAL_DATA_ADDRESS) {
                printf("OK\n");
        } else {
                printf("FAILED\n");
        }

        ret = force_multiple_coils(&mb_param, SLAVE,
                                   UT_COIL_STATUS_ADDRESS + UT_COIL_STATUS_NB_POINTS,
                                   UT_COIL_STATUS_NB_POINTS,
                                   tab_rp_status);
        printf("* force_multiple_coils: ");
        if (ret == ILLEGAL_DATA_ADDRESS) {
                printf("OK\n");
        } else {
                printf("FAILED\n");
        }

        ret = preset_multiple_registers(&mb_param, SLAVE,
                                        UT_HOLDING_REGISTERS_ADDRESS + UT_HOLDING_REGISTERS_NB_POINTS,
                                        UT_HOLDING_REGISTERS_NB_POINTS,
                                        tab_rp_registers);
        printf("* preset_multiple_registers: ");
        if (ret == ILLEGAL_DATA_ADDRESS) {
                printf("OK\n");
        } else {
                printf("FAILED\n");
        }


        /** TOO MANY DATA **/
        printf("\nTEST TOO MANY DATA ERROR:\n");

        ret = read_coil_status(&mb_param, SLAVE,
                               UT_COIL_STATUS_ADDRESS,
                               MAX_STATUS + 1,
                               tab_rp_status);
        printf("* read_coil_status: ");
        if (ret == TOO_MANY_DATA)
                printf("OK\n");
        else
                printf("FAILED\n");

        ret = read_input_status(&mb_param, SLAVE,
                                UT_INPUT_STATUS_ADDRESS,
                                MAX_STATUS + 1,
                                tab_rp_status);
        printf("* read_input_status: ");
        if (ret == TOO_MANY_DATA)
                printf("OK\n");
        else
                printf("FAILED\n");

        ret = read_holding_registers(&mb_param, SLAVE,
                                     UT_HOLDING_REGISTERS_ADDRESS,
                                     MAX_REGISTERS + 1,
                                     tab_rp_registers);
        printf("* read_holding_registers: ");
        if (ret == TOO_MANY_DATA)
                printf("OK\n");
        else
                printf("FAILED\n");
                
        ret = read_input_registers(&mb_param, SLAVE,
                                   UT_INPUT_REGISTERS_ADDRESS,
                                   MAX_REGISTERS + 1,
                                   tab_rp_registers);
        printf("* read_input_registers: ");
        if (ret == TOO_MANY_DATA)
                printf("OK\n");
        else
                printf("FAILED\n");

        ret = force_multiple_coils(&mb_param, SLAVE,
                                   UT_COIL_STATUS_ADDRESS,
                                   MAX_STATUS + 1,
                                   tab_rp_status);
        printf("* force_multiple_coils: ");
        if (ret == TOO_MANY_DATA) {
                printf("OK\n");
        } else {
                printf("FAILED\n");
        }

        ret = preset_multiple_registers(&mb_param, SLAVE,
                                        UT_HOLDING_REGISTERS_ADDRESS,
                                        MAX_REGISTERS + 1,
                                        tab_rp_registers);
        printf("* preset_multiple_registers: ");
        if (ret == TOO_MANY_DATA) {
                printf("OK\n");
        } else {
                printf("FAILED\n");
        }

        /** BAD RESPONSE **/
        printf("\nTEST BAD RESPONSE ERROR:\n");

        /* Allocate only the required space */
        uint16_t *tab_rp_registers_bad = (uint16_t *) malloc(
                UT_HOLDING_REGISTERS_NB_POINTS_SPECIAL * sizeof(uint16_t));
        ret = read_holding_registers(&mb_param,
                                     SLAVE, UT_HOLDING_REGISTERS_ADDRESS,
                                     UT_HOLDING_REGISTERS_NB_POINTS_SPECIAL,
                                     tab_rp_registers_bad);
        printf("* read_holding_registers: ");
        if (ret > 0) {
                /* Error not detected */
                printf("FAILED\n");
        } else {
                printf("OK\n");
        }
        free(tab_rp_registers_bad);

close:
        /* Free the memory */
        free(tab_rp_status);                                           
        free(tab_rp_registers);

        /* Close the connection */
        modbus_close(&mb_param);
        
        return 0;
}
