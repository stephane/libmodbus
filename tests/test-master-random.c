/*
 * Copyright (C) 2008 Stéphane Raimbault <stephane.raimbault@gmail.com>
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

/* The goal of this program is to check all major functions of
   libmodbus:
   - force_single_coil
   - read_coil_status
   - force_multiple_coils
   - preset_single_register
   - read_holding_registers
   - preset_multiple_registers
   - read_holding_registers

   All these functions are called with random values on a address
   range defined by the following defines. 
*/
#define LOOP             1
#define SLAVE         0x11
#define ADDRESS_START    0
#define ADDRESS_END    499

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */
int main(void)
{
        int ret;
        int nb_fail;
        int nb_loop;
        int addr;
        int nb_points_total;
        uint8_t *tab_rq_status;
        uint8_t *tab_rp_status;
        uint16_t *tab_rq_registers;
        uint16_t *tab_rp_registers;
        modbus_param_t mb_param;

        /* RTU parity : none, even, odd */
        /* modbus_init_rtu(&mb_param, "/dev/ttyS0", 19200, "none", 8, 1); */

        /* TCP */
        modbus_init_tcp(&mb_param, "192.168.0.100", MODBUS_TCP_DEFAULT_PORT);
        modbus_set_debug(&mb_param, TRUE);

        modbus_connect(&mb_param);

        /* Allocate and initialize the different memory spaces */
        nb_points_total = ADDRESS_END - ADDRESS_START;

        tab_rq_status = (uint8_t *) malloc(nb_points_total * sizeof(uint8_t));
        memset(tab_rq_status, 0, nb_points_total * sizeof(uint8_t));
        tab_rp_status = (uint8_t *) malloc(nb_points_total * sizeof(uint8_t));
        memset(tab_rp_status, 0, nb_points_total * sizeof(uint8_t));

        tab_rq_registers = (uint16_t *) malloc(nb_points_total * sizeof(uint16_t));
        memset(tab_rq_registers, 0, nb_points_total * sizeof(uint16_t));
        tab_rp_registers = (uint16_t *) malloc(nb_points_total * sizeof(uint16_t));
        memset(tab_rp_status, 0, nb_points_total * sizeof(uint16_t));

        nb_loop = nb_fail = 0;
        while (nb_loop++ < LOOP) { 
                for (addr = ADDRESS_START; addr <= ADDRESS_END; addr++) {
                        int i;
                        int nb_points;

                        /* Random numbers (short) */
                        for (i=0; i<nb_points_total; i++) {
                                tab_rq_registers[i] = (uint16_t) (65535.0*rand() / (RAND_MAX + 1.0));
                                tab_rq_status[i] = tab_rq_registers[i] % 2;
                        }
                        nb_points = ADDRESS_END - addr;

                        /* SINGLE COIL */
                        ret = force_single_coil(&mb_param, SLAVE, addr, tab_rq_status[0]);
                        if (ret != 1) {
                                printf("ERROR force_single_coil (%d)\n", ret);
                                printf("Slave = %d, address = %d, value = %d\n",
                                       SLAVE, addr, tab_rq_status[0]);
                                nb_fail++;
                        } else {
                                ret = read_coil_status(&mb_param, SLAVE, addr, 1, tab_rp_status);
                                if (ret != 1 || tab_rq_status[0] != tab_rp_status[0]) {
                                        printf("ERROR read_coil_status single (%d)\n", ret);
                                        printf("Slave = %d, address = %d\n", 
                                               SLAVE, addr);
                                        nb_fail++;
                                }
                        }

                        /* MULTIPLE COILS */
                        ret = force_multiple_coils(&mb_param, SLAVE, addr, nb_points, tab_rq_status);
                        if (ret != nb_points) {
                                printf("ERROR force_multiple_coils (%d)\n", ret);
                                printf("Slave = %d, address = %d, nb_points = %d\n",
                                       SLAVE, addr, nb_points);
                                nb_fail++;
                        } else {
                                ret = read_coil_status(&mb_param, SLAVE, addr, nb_points, tab_rp_status);
                                if (ret != nb_points) {
                                        printf("ERROR read_coil_status\n");
                                        printf("Slave = %d, address = %d, nb_points = %d\n",
                                               SLAVE, addr, nb_points);
                                        nb_fail++;
                                } else {
                                        for (i=0; i<nb_points; i++) {
                                                if (tab_rp_status[i] != tab_rq_status[i]) {
                                                        printf("ERROR read_coil_status ");
                                                        printf("(%d != %d)\n", tab_rp_status[i], tab_rq_status[i]);
                                                        printf("Slave = %d, address = %d\n", 
                                                               SLAVE, addr);
                                                        nb_fail++;
                                                }
                                        }
                                }
                        }

                        /* SINGLE REGISTER */
                        ret = preset_single_register(&mb_param, SLAVE, addr, tab_rq_registers[0]);
                        if (ret != 1) {
                                printf("ERROR preset_single_register (%d)\n", ret);
                                printf("Slave = %d, address = %d, value = %d\n",
                                       SLAVE, addr, tab_rq_registers[0]);
                                nb_fail++;
                        } else {
                                ret = read_holding_registers(&mb_param, SLAVE,
                                                            addr, 1, tab_rp_registers);
                                if (ret != 1) {
                                        printf("ERROR read_holding_registers single (%d)\n", ret);
                                        printf("Slave = %d, address = %d\n",
                                               SLAVE, addr);
                                        nb_fail++;
                                } else {
                                        if (tab_rq_registers[0] != tab_rp_registers[0]) {
                                                printf("ERROR read_holding_registers single ");
                                                printf("(%d != %d)\n",
                                                       tab_rq_registers[0], tab_rp_registers[0]);
                                                printf("Slave = %d, address = %d\n", 
                                                       SLAVE, addr);
                                                nb_fail++;
                                        }
                                }
                        }
                        
                        /* MULTIPLE REGISTERS */
                        ret = preset_multiple_registers(&mb_param, SLAVE,
                                                        addr, nb_points, tab_rq_registers);
                        if (ret != nb_points) {
                                printf("ERROR preset_multiple_registers (%d)\n", ret);
                                printf("Slave = %d, address = %d, nb_points = %d\n",
                                               SLAVE, addr, nb_points);
                                nb_fail++;
                        } else {
                                ret = read_holding_registers(&mb_param, SLAVE,
                                                             addr, nb_points, tab_rp_registers);
                                if (ret != nb_points) {
                                        printf("ERROR read_holding_registers (%d)\n", ret);
                                        printf("Slave = %d, address = %d, nb_points = %d\n",
                                               SLAVE, addr, nb_points);
                                        nb_fail++;
                                } else {
                                        for (i=0; i<nb_points; i++) {
                                                if (tab_rq_registers[i] != tab_rp_registers[i]) {
                                                        printf("ERROR read_holding_registers ");
                                                        printf("(%d != %d)\n",
                                                               tab_rq_registers[i], tab_rp_registers[i]);
                                                        printf("Slave = %d, address = %d\n", 
                                                               SLAVE, addr);
                                                        nb_fail++;
                                                }
                                        }
                                }
                        }
                }
                        
                if (nb_fail)
                        printf("Address: %d - nb of fails: %d\n", addr, nb_fail);
                else
                        printf("Address: %d - OK\n", addr);
        }

        /* Free the memory */
        free(tab_rp_status);                                           
        free(tab_rq_status);
        free(tab_rp_registers);
        free(tab_rq_registers);

        /* Close the connection */
        modbus_close(&mb_param);
        
        return 0;
}

