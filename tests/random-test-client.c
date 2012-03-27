/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
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

/* The goal of this program is to check all major functions of
   libmodbus:
   - write_coil
   - read_bits
   - write_coils
   - write_register
   - read_registers
   - write_registers
   - read_registers

   All these functions are called with random values on a address
   range defined by the following defines.
*/
#define LOOP             1
#define SERVER_ID       17
#define ADDRESS_START    0
#define ADDRESS_END     99

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */
int main(void)
{
    modbus_t *ctx;
    int rc;
    int nb_fail;
    int nb_loop;
    int addr;
    int nb;
    uint8_t *tab_rq_bits;
    uint8_t *tab_rp_bits;
    uint16_t *tab_rq_registers;
    uint16_t *tab_rw_rq_registers;
    uint16_t *tab_rp_registers;

    /* RTU */
/*
    ctx = modbus_new_rtu("/dev/ttyUSB0", 19200, 'N', 8, 1);
    modbus_set_slave(ctx, SERVER_ID);
*/

    /* TCP */
    ctx = modbus_new_tcp("127.0.0.1", 1502);
    modbus_set_debug(ctx, TRUE);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the different memory spaces */
    nb = ADDRESS_END - ADDRESS_START;

    tab_rq_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
    memset(tab_rq_bits, 0, nb * sizeof(uint8_t));

    tab_rp_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb * sizeof(uint8_t));

    tab_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

    tab_rp_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

    tab_rw_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rw_rq_registers, 0, nb * sizeof(uint16_t));

    nb_loop = nb_fail = 0;
    while (nb_loop++ < LOOP) {
        for (addr = ADDRESS_START; addr <= ADDRESS_END; addr++) {
            int i;

            /* Random numbers (short) */
            for (i=0; i<nb; i++) {
                tab_rq_registers[i] = (uint16_t) (65535.0*rand() / (RAND_MAX + 1.0));
                tab_rw_rq_registers[i] = ~tab_rq_registers[i];
                tab_rq_bits[i] = tab_rq_registers[i] % 2;
            }
            nb = ADDRESS_END - addr;

            /* WRITE BIT */
            rc = modbus_write_bit(ctx, addr, tab_rq_bits[0]);
            if (rc != 1) {
                printf("ERROR modbus_write_bit (%d)\n", rc);
                printf("Address = %d, value = %d\n", addr, tab_rq_bits[0]);
                nb_fail++;
            } else {
                rc = modbus_read_bits(ctx, addr, 1, tab_rp_bits);
                if (rc != 1 || tab_rq_bits[0] != tab_rp_bits[0]) {
                    printf("ERROR modbus_read_bits single (%d)\n", rc);
                    printf("address = %d\n", addr);
                    nb_fail++;
                }
            }

            /* MULTIPLE BITS */
            rc = modbus_write_bits(ctx, addr, nb, tab_rq_bits);
            if (rc != nb) {
                printf("ERROR modbus_write_bits (%d)\n", rc);
                printf("Address = %d, nb = %d\n", addr, nb);
                nb_fail++;
            } else {
                rc = modbus_read_bits(ctx, addr, nb, tab_rp_bits);
                if (rc != nb) {
                    printf("ERROR modbus_read_bits\n");
                    printf("Address = %d, nb = %d\n", addr, nb);
                    nb_fail++;
                } else {
                    for (i=0; i<nb; i++) {
                        if (tab_rp_bits[i] != tab_rq_bits[i]) {
                            printf("ERROR modbus_read_bits\n");
                            printf("Address = %d, value %d (0x%X) != %d (0x%X)\n",
                                   addr, tab_rq_bits[i], tab_rq_bits[i],
                                   tab_rp_bits[i], tab_rp_bits[i]);
                            nb_fail++;
                        }
                    }
                }
            }

            /* SINGLE REGISTER */
            rc = modbus_write_register(ctx, addr, tab_rq_registers[0]);
            if (rc != 1) {
                printf("ERROR modbus_write_register (%d)\n", rc);
                printf("Address = %d, value = %d (0x%X)\n",
                       addr, tab_rq_registers[0], tab_rq_registers[0]);
                nb_fail++;
            } else {
                rc = modbus_read_registers(ctx, addr, 1, tab_rp_registers);
                if (rc != 1) {
                    printf("ERROR modbus_read_registers single (%d)\n", rc);
                    printf("Address = %d\n", addr);
                    nb_fail++;
                } else {
                    if (tab_rq_registers[0] != tab_rp_registers[0]) {
                        printf("ERROR modbus_read_registers single\n");
                        printf("Address = %d, value = %d (0x%X) != %d (0x%X)\n",
                               addr, tab_rq_registers[0], tab_rq_registers[0],
                               tab_rp_registers[0], tab_rp_registers[0]);
                        nb_fail++;
                    }
                }
            }

            /* MULTIPLE REGISTERS */
            rc = modbus_write_registers(ctx, addr, nb, tab_rq_registers);
            if (rc != nb) {
                printf("ERROR modbus_write_registers (%d)\n", rc);
                printf("Address = %d, nb = %d\n", addr, nb);
                nb_fail++;
            } else {
                rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
                if (rc != nb) {
                    printf("ERROR modbus_read_registers (%d)\n", rc);
                    printf("Address = %d, nb = %d\n", addr, nb);
                    nb_fail++;
                } else {
                    for (i=0; i<nb; i++) {
                        if (tab_rq_registers[i] != tab_rp_registers[i]) {
                            printf("ERROR modbus_read_registers\n");
                            printf("Address = %d, value %d (0x%X) != %d (0x%X)\n",
                                   addr, tab_rq_registers[i], tab_rq_registers[i],
                                   tab_rp_registers[i], tab_rp_registers[i]);
                            nb_fail++;
                        }
                    }
                }
            }
            /* R/W MULTIPLE REGISTERS */
            rc = modbus_write_and_read_registers(ctx,
                                                 addr, nb, tab_rw_rq_registers,
                                                 addr, nb, tab_rp_registers);
            if (rc != nb) {
                printf("ERROR modbus_read_and_write_registers (%d)\n", rc);
                printf("Address = %d, nb = %d\n", addr, nb);
                nb_fail++;
            } else {
                for (i=0; i<nb; i++) {
                    if (tab_rp_registers[i] != tab_rw_rq_registers[i]) {
                        printf("ERROR modbus_read_and_write_registers READ\n");
                        printf("Address = %d, value %d (0x%X) != %d (0x%X)\n",
                               addr, tab_rp_registers[i], tab_rw_rq_registers[i],
                               tab_rp_registers[i], tab_rw_rq_registers[i]);
                        nb_fail++;
                    }
                }

                rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
                if (rc != nb) {
                    printf("ERROR modbus_read_registers (%d)\n", rc);
                    printf("Address = %d, nb = %d\n", addr, nb);
                    nb_fail++;
                } else {
                    for (i=0; i<nb; i++) {
                        if (tab_rw_rq_registers[i] != tab_rp_registers[i]) {
                            printf("ERROR modbus_read_and_write_registers WRITE\n");
                            printf("Address = %d, value %d (0x%X) != %d (0x%X)\n",
                                   addr, tab_rw_rq_registers[i], tab_rw_rq_registers[i],
                                   tab_rp_registers[i], tab_rp_registers[i]);
                            nb_fail++;
                        }
                    }
                }
            }
        }

        printf("Test: ");
        if (nb_fail)
            printf("%d FAILS\n", nb_fail);
        else
            printf("SUCCESS\n");
    }

    /* Free the memory */
    free(tab_rq_bits);
    free(tab_rp_bits);
    free(tab_rq_registers);
    free(tab_rp_registers);
    free(tab_rw_rq_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
