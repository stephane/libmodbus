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
#include <time.h>
#include <sys/time.h>

#include <modbus/modbus.h>

/* Tests based on PI-MBUS-300 documentation */
#define SLAVE     0x11
#define NB_LOOPS  10000

#define G_USEC_PER_SEC 1000000
uint32_t gettime(void)
{
        struct timeval tv;
        gettimeofday (&tv, NULL);
        
        return (uint32_t) tv.tv_sec * G_USEC_PER_SEC + tv.tv_usec;
}

int main(void)
{
        uint8_t *tab_rp_status;
        uint16_t *tab_rp_registers;
        modbus_param_t mb_param;
        int i;
        int ret;
        int nb_points;
        double elapsed;
        uint32_t start;
        uint32_t end;
        uint32_t bytes;
        uint32_t rate;

        /* TCP */
        modbus_init_tcp(&mb_param, "127.0.0.1", 1502);
        if (modbus_connect(&mb_param) == -1) {
                printf("ERROR Connection failed\n");
                exit(1);
        }

        /* Allocate and initialize the memory to store the status */
        tab_rp_status = (uint8_t *) malloc(MAX_STATUS * sizeof(uint8_t));
        memset(tab_rp_status, 0, MAX_STATUS * sizeof(uint8_t));
        
        /* Allocate and initialize the memory to store the registers */
        tab_rp_registers = (uint16_t *) malloc(MAX_REGISTERS * sizeof(uint16_t));
        memset(tab_rp_registers, 0, MAX_REGISTERS * sizeof(uint16_t));

        printf("READ COIL STATUS\n\n");

        nb_points = MAX_STATUS;
        start = gettime();
        for (i=0; i<NB_LOOPS; i++) {
                ret = read_coil_status(&mb_param, SLAVE, 0, nb_points, tab_rp_status);
        }
        end = gettime();
        elapsed = (end - start) / 1000;

        rate = (NB_LOOPS * nb_points) * G_USEC_PER_SEC / (end - start);
        printf("Transfert rate in points/seconds:\n");
        printf("* %'d points/s\n", rate); 
        printf("\n");

        bytes = NB_LOOPS * (nb_points / 8) + ((nb_points % 8) ? 1 : 0);
        rate = bytes / 1024 * G_USEC_PER_SEC / (end - start);
        printf("Values:\n");
        printf("* %d x %d values\n", NB_LOOPS, nb_points);
        printf("* %.3f ms for %d bytes\n", elapsed, bytes);
        printf("* %'d KiB/s\n", rate);
        printf("\n");
        
        /* TCP: Query and reponse header and values */
        bytes = 12 + 9 + (nb_points / 8) + ((nb_points % 8) ? 1 : 0);
        printf("Values and TCP Modbus overhead:\n");
        printf("* %d x %d bytes\n", NB_LOOPS, bytes);
        bytes = NB_LOOPS * bytes;
        rate = bytes / 1024 * G_USEC_PER_SEC / (end - start);
        printf("* %.3f ms for %d bytes\n", elapsed, bytes);
        printf("* %'d KiB/s\n", rate);
        printf("\n\n");

        printf("READ HOLDING REGISTERS\n\n");

        nb_points = MAX_REGISTERS;
        start = gettime();
        for (i=0; i<NB_LOOPS; i++) {
                ret = read_holding_registers(&mb_param, SLAVE, 0, nb_points, tab_rp_registers);
        }
        end = gettime();
        elapsed = (end - start) / 1000;

        rate = (NB_LOOPS * nb_points) * G_USEC_PER_SEC / (end - start);
        printf("Transfert rate in points/seconds:\n");
        printf("* %'d registers/s\n", rate); 
        printf("\n");

        bytes = NB_LOOPS * nb_points * sizeof(uint16_t);
        rate = bytes / 1024 * G_USEC_PER_SEC / (end - start);
        printf("Values:\n");
        printf("* %d x %d values\n", NB_LOOPS, nb_points);
        printf("* %.3f ms for %d bytes\n", elapsed, bytes);
        printf("* %'d KiB/s\n", rate);
        printf("\n");
        
        /* TCP:Query and reponse header and values */
        bytes = 12 + 9 + (nb_points * sizeof(uint16_t));
        printf("Values and TCP Modbus overhead:\n");
        printf("* %d x %d bytes\n", NB_LOOPS, bytes);
        bytes = NB_LOOPS * bytes;
        rate = bytes / 1024 * G_USEC_PER_SEC / (end - start);
        printf("* %.3f ms for %d bytes\n", elapsed, bytes);
        printf("* %'d KiB/s\n", rate);
        printf("\n");

        /* Free the memory */
        free(tab_rp_status);                                           
        free(tab_rp_registers);

        /* Close the connection */
        modbus_close(&mb_param);
        
        return 0;
}
