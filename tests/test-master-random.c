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
   range defined by following defines.

   This program is also really useful to test your remote target unit.
*/
#define LOOP       1
#define SLAVE   0x11
#define ADDR_MIN   0
#define ADDR_MAX 499
#define FIELDS   500

int main(void)
{
        int ok, fail;
        int loop_nb;
        int addr;
        int field_nb;
        int *tab_rq;
        int *tab_rq_bits;
        int *tab_rp;
        modbus_param_t mb_param;

        /* RTU parity : none, even, odd */
        /* modbus_init_rtu(&mb_param, "/dev/ttyS0", 19200, "none", 8, 1); */

        /* TCP */
        modbus_init_tcp(&mb_param, "192.168.0.100", MODBUS_TCP_DEFAULT_PORT);
        modbus_set_debug(&mb_param, TRUE);

        modbus_connect(&mb_param);./configu

        /* Allocate and initialize the different memory spaces */
        tab_rq = (int *) malloc(FIELDS * sizeof(int));
        memset(tab_rq, 0, FIELDS * sizeof(int));

        tab_rq_bits = (int *) malloc(FIELDS * sizeof(int));
        memset(tab_rq_bits, 0, FIELDS * sizeof(int));

        tab_rp = (int *) malloc(FIELDS * sizeof(int));
        memset(tab_rp, 0, FIELDS * sizeof(int));

        loop_nb = ok = fail = 0;
        while (loop_nb++ < LOOP) { 
                for (addr=ADDR_MIN; addr <= ADDR_MAX; addr++) {
                        for (field_nb=1; field_nb<=FIELDS; field_nb++) {
                                int i;

                                /* Random numbers (short) */
                                for (i=0; i<field_nb; i++) {
                                        tab_rq[i] = (int) (16536.0*rand()/(RAND_MAX+1.0));
                                        tab_rq_bits[i] = (i) % 2;
                                }

                                /* SINGLE COIL */
                                ok = force_single_coil(&mb_param, SLAVE, addr, tab_rq_bits[0]);
                                if (ok != 1) {
                                        printf("ERROR force_single_coil (%d)\n", ok);
                                        printf("Slave = %d, address = %d, value = %d\n",
                                               SLAVE, addr, tab_rq_bits[0]);
                                        fail++;
                                } else {
                                        ok = read_coil_status(&mb_param, SLAVE, addr, 1, tab_rp);
                                        if (ok != 1 || tab_rq_bits[0] != tab_rp[0]) {
                                                printf("ERROR read_coil_status single (%d)\n", ok);
                                                printf("Slave = %d, address = %d\n", 
                                                       SLAVE, addr);
                                                fail++;
                                        }
                                }

                                /* MULTIPLE COILS */
                                ok = force_multiple_coils(&mb_param, SLAVE, addr, field_nb, tab_rq_bits);
                                if (ok != field_nb) {
                                        printf("ERROR force_multiple_coils (%d)\n", ok);
                                        printf("Slave = %d, address = %d, field_nb = %d\n",
                                               SLAVE, addr, field_nb);
                                        fail++;
                                } else {
                                        ok = read_coil_status(&mb_param, SLAVE, addr,
                                                              field_nb, tab_rp);
                                        if (ok != field_nb) {
                                                printf("ERROR read_coil_status\n");
                                                printf("Slave = %d, address = %d, field_nb = %d\n",
                                                       SLAVE, addr, field_nb);
                                                fail++;
                                        } else {
                                                for (i=0; i<field_nb; i++) {
                                                        if (tab_rp[i] != tab_rq_bits[i]) {
                                                                printf("ERROR read_coil_status ");
                                                                printf("(%d != %d)\n", tab_rp[i], tab_rq_bits[i]);
                                                                printf("Slave = %d, address = %d\n", 
                                                                       SLAVE, addr);
                                                                fail++;
                                                        }
                                                }
                                        }
                                }

                                /* SINGLE REGISTER */
                                ok = preset_single_register(&mb_param, SLAVE, addr, tab_rq[0]);
                                if (ok != 1) {
                                        printf("ERROR preset_single_register (%d)\n", ok);
                                        printf("Slave = %d, address = %d, value = %d\n",
                                               SLAVE, addr, tab_rq[0]);
                                        fail++;
                                } else {
                                        ok = read_holding_registers(&mb_param, SLAVE,
                                                                    addr, 1, tab_rp);
                                        if (ok != 1) {
                                                printf("ERROR read_holding_registers single (%d)\n", ok);
                                                printf("Slave = %d, address = %d\n",
                                                       SLAVE, addr);
                                                fail++;
                                        } else {
                                                if (tab_rq[0] != tab_rp[0]) {
                                                        printf("ERROR read_holding_registers single ");
                                                        printf("(%d != %d)\n",
                                                               tab_rq[0], tab_rp[0]);
                                                        printf("Slave = %d, address = %d\n", 
                                                               SLAVE, addr);
                                                        fail++;
                                                }
                                        }
                                }

                                /* MULTIPLE REGISTERS */
                                ok = preset_multiple_registers(&mb_param, SLAVE,
                                                               addr, field_nb, tab_rq);
                                if (ok != field_nb) {
                                        printf("ERROR preset_multiple_registers (%d)\n", ok);
                                        printf("Slave = %d, address = %d, field_nb = %d\n",
                                               SLAVE, addr, field_nb);
                                        fail++;
                                } else {
                                        ok = read_holding_registers(&mb_param, SLAVE,
                                                                    addr, field_nb, tab_rp);
                                        if (ok != field_nb) {
                                                printf("ERROR read_holding_registers (%d)\n", ok);
                                                printf("Slave = %d, address = %d, field_nb = %d\n",
                                                       SLAVE, addr, field_nb);
                                                fail++;
                                        } else {
                                                for (i=0; i<field_nb; i++) {
                                                        if (tab_rq[i] != tab_rp[i]) {
                                                                printf("ERROR read_holding_registers ");
                                                                printf("(%d != %d)\n",
                                                                       tab_rq[i], tab_rp[i]);
                                                                printf("Slave = %d, address = %d\n", 
                                                                       SLAVE, addr);
                                                                fail++;
                                                        }
                                                }
                                        }
                                }
                        }
        
                        if (fail)
                                printf("Address : %d - Fails sum : %d\n", addr, fail);
                        else
                                printf("Address : %d - OK\n", addr);
                }
        }

        /* Free the memory */
        free(tab_rp);                                           
        free(tab_rq);
        free(tab_rq_bits);

        /* Close the connection */
        modbus_close(&mb_param);
        
        return 0;
}

