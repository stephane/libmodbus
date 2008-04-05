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

/* Tests based on PI-MBUS-300 documentation */
#define SLAVE    0x11
#define FIELDS   500

int main(void)
{
        int *tab_rq;
        int *tab_rq_bits;
        int *tab_rp;
        modbus_param_t mb_param;

        /* RTU parity : none, even, odd */
/*      modbus_init_rtu(&mb_param, "/dev/ttyS0", 19200, "none", 8, 1); */

        /* TCP */
        modbus_init_tcp(&mb_param, "127.0.0.1", 1502);
        modbus_set_debug(&mb_param, TRUE);
      
        modbus_connect(&mb_param);

        /* Allocate and initialize the different memory spaces */
        tab_rq = (int *) malloc(FIELDS * sizeof(int));
        memset(tab_rq, 0, FIELDS * sizeof(int));

        tab_rq_bits = (int *) malloc(FIELDS * sizeof(int));
        memset(tab_rq_bits, 0, FIELDS * sizeof(int));

        tab_rp = (int *) malloc(FIELDS * sizeof(int));
        memset(tab_rp, 0, FIELDS * sizeof(int));
        
        read_coil_status(&mb_param, SLAVE, 0x13, 0x25, tab_rp);
        read_input_status(&mb_param, SLAVE, 0xC4, 0x16, tab_rp);
        read_holding_registers(&mb_param, SLAVE, 0x6B, 3, tab_rp);
        read_input_registers(&mb_param, SLAVE, 0x8, 1, tab_rp);
        force_single_coil(&mb_param, SLAVE, 0xAC, ON);

        /* Free the memory */
        free(tab_rp);                                           
        free(tab_rq);
        free(tab_rq_bits);

        /* Close the connection */
        modbus_close(&mb_param);
        
        return 0;
}
