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

enum {
    TCP,
    TCP_PI,
    RTU
};

int main(int argc, char *argv[])
{
    uint8_t *tab_rp_bits;
    uint16_t *tab_rp_registers;
    uint16_t *tab_rp_registers_bad;
    modbus_t *ctx;
    int i;
    uint8_t value;
    int address;
    int nb_points;
    int rc;
    float real;
    struct timeval timeout_begin_old;
    struct timeval timeout_begin_new;
    int use_backend;

    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            use_backend = TCP;
	} else if (strcmp(argv[1], "tcppi") == 0) {
            use_backend = TCP_PI;
        } else if (strcmp(argv[1], "rtu") == 0) {
            use_backend = RTU;
        } else {
            printf("Usage:\n  %s [tcp|tcppi|rtu] - Modbus client for unit testing\n\n", argv[0]);
            exit(1);
        }
    } else {
        /* By default */
        use_backend = TCP;
    }

    if (use_backend == TCP) {
        ctx = modbus_new_tcp("127.0.0.1", 1502);
    } else if (use_backend == TCP_PI) {
        ctx = modbus_new_tcp_pi("::1", "1502");
    } else {
        ctx = modbus_new_rtu("/dev/ttyUSB1", 115200, 'N', 8, 1);
    }
    if (ctx == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }
    modbus_set_debug(ctx, TRUE);

    if (use_backend == RTU) {
          modbus_set_slave(ctx, SERVER_ID);
    }

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the memory to store the bits */
    nb_points = (UT_BITS_NB_POINTS > UT_INPUT_BITS_NB_POINTS) ?
        UT_BITS_NB_POINTS : UT_INPUT_BITS_NB_POINTS;
    tab_rp_bits = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

    /* Allocate and initialize the memory to store the registers */
    nb_points = (UT_REGISTERS_NB_POINTS >
                 UT_INPUT_REGISTERS_NB_POINTS) ?
        UT_REGISTERS_NB_POINTS : UT_INPUT_REGISTERS_NB_POINTS;
    tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

    printf("** UNIT TESTING **\n");

    printf("\nTEST WRITE/READ:\n");

    /** COIL BITS **/

    /* Single */
    rc = modbus_write_bit(ctx, UT_BITS_ADDRESS, ON);
    printf("1/2 modbus_write_bit: ");
    if (rc == 1) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_bits(ctx, UT_BITS_ADDRESS, 1, tab_rp_bits);
    printf("2/2 modbus_read_bits: ");
    if (rc != 1) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

    if (tab_rp_bits[0] != ON) {
        printf("FAILED (%0X = != %0X)\n", tab_rp_bits[0], ON);
        goto close;
    }
    printf("OK\n");
    /* End single */

    /* Multiple bits */
    {
        uint8_t tab_value[UT_BITS_NB_POINTS];

        modbus_set_bits_from_bytes(tab_value, 0, UT_BITS_NB_POINTS,
                                   UT_BITS_TAB);
        rc = modbus_write_bits(ctx, UT_BITS_ADDRESS,
                               UT_BITS_NB_POINTS, tab_value);
        printf("1/2 modbus_write_bits: ");
        if (rc == UT_BITS_NB_POINTS) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
            goto close;
        }
    }

    rc = modbus_read_bits(ctx, UT_BITS_ADDRESS,
                          UT_BITS_NB_POINTS, tab_rp_bits);
    printf("2/2 modbus_read_bits: ");
    if (rc != UT_BITS_NB_POINTS) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

    i = 0;
    address = UT_BITS_ADDRESS;
    nb_points = UT_BITS_NB_POINTS;
    while (nb_points > 0) {
        int nb_bits = (nb_points > 8) ? 8 : nb_points;

        value = modbus_get_byte_from_bits(tab_rp_bits, i*8, nb_bits);
        if (value != UT_BITS_TAB[i]) {
            printf("FAILED (%0X != %0X)\n",
                   value, UT_BITS_TAB[i]);
            goto close;
        }

        nb_points -= nb_bits;
        i++;
    }
    printf("OK\n");
    /* End of multiple bits */

    /** DISCRETE INPUTS **/
    rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
                                UT_INPUT_BITS_NB_POINTS, tab_rp_bits);
    printf("1/1 modbus_read_input_bits: ");

    if (rc != UT_INPUT_BITS_NB_POINTS) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

    i = 0;
    address = UT_INPUT_BITS_ADDRESS;
    nb_points = UT_INPUT_BITS_NB_POINTS;
    while (nb_points > 0) {
        int nb_bits = (nb_points > 8) ? 8 : nb_points;

        value = modbus_get_byte_from_bits(tab_rp_bits, i*8, nb_bits);
        if (value != UT_INPUT_BITS_TAB[i]) {
            printf("FAILED (%0X != %0X)\n",
                   value, UT_INPUT_BITS_TAB[i]);
            goto close;
        }

        nb_points -= nb_bits;
        i++;
    }
    printf("OK\n");

    /** HOLDING REGISTERS **/

    /* Single register */
    rc = modbus_write_register(ctx, UT_REGISTERS_ADDRESS, 0x1234);
    printf("1/2 modbus_write_register: ");
    if (rc == 1) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               1, tab_rp_registers);
    printf("2/2 modbus_read_registers: ");
    if (rc != 1) {
        printf("FAILED (nb points %d)\n", rc);
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
    rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS,
                                UT_REGISTERS_NB_POINTS,
                                UT_REGISTERS_TAB);
    printf("1/5 modbus_write_registers: ");
    if (rc == UT_REGISTERS_NB_POINTS) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB_POINTS,
                               tab_rp_registers);
    printf("2/5 modbus_read_registers: ");
    if (rc != UT_REGISTERS_NB_POINTS) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

    for (i=0; i < UT_REGISTERS_NB_POINTS; i++) {
        if (tab_rp_registers[i] != UT_REGISTERS_TAB[i]) {
            printf("FAILED (%0X != %0X)\n",
                   tab_rp_registers[i],
                   UT_REGISTERS_TAB[i]);
            goto close;
        }
    }
    printf("OK\n");

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               0, tab_rp_registers);
    printf("3/5 modbus_read_registers (0): ");
    if (rc != 0) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }
    printf("OK\n");

    nb_points = (UT_REGISTERS_NB_POINTS >
                 UT_INPUT_REGISTERS_NB_POINTS) ?
        UT_REGISTERS_NB_POINTS : UT_INPUT_REGISTERS_NB_POINTS;
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

    /* Write registers to zero from tab_rp_registers and store read registers
       into tab_rp_registers. So the read registers must set to 0, except the
       first one because there is an offset of 1 register on write. */
    rc = modbus_read_and_write_registers(ctx,
                                         UT_REGISTERS_ADDRESS,
                                         UT_REGISTERS_NB_POINTS,
                                         tab_rp_registers,
                                         UT_REGISTERS_ADDRESS + 1,
                                         UT_REGISTERS_NB_POINTS - 1,
                                         tab_rp_registers);
    printf("4/5 modbus_read_and_write_registers: ");
    if (rc != UT_REGISTERS_NB_POINTS) {
        printf("FAILED (nb points %d != %d)\n", rc, UT_REGISTERS_NB_POINTS);
        goto close;
    }

    if (tab_rp_registers[0] != UT_REGISTERS_TAB[0]) {
        printf("FAILED (%0X != %0X)\n",
               tab_rp_registers[0], UT_REGISTERS_TAB[0]);
    }

    for (i=1; i < UT_REGISTERS_NB_POINTS; i++) {
        if (tab_rp_registers[i] != 0) {
            printf("FAILED (%0X != %0X)\n",
                   tab_rp_registers[i], 0);
            goto close;
        }
    }
    printf("OK\n");

    /* End of many registers */


    /** INPUT REGISTERS **/
    rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
                                     UT_INPUT_REGISTERS_NB_POINTS,
                                     tab_rp_registers);
    printf("1/1 modbus_read_input_registers: ");
    if (rc != UT_INPUT_REGISTERS_NB_POINTS) {
        printf("FAILED (nb points %d)\n", rc);
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

    printf("\nTEST FLOATS\n");
    /** FLOAT **/
    printf("1/2 Set float: ");
    modbus_set_float(UT_REAL, tab_rp_registers);
    if (tab_rp_registers[1] == (UT_IREAL >> 16) &&
        tab_rp_registers[0] == (UT_IREAL & 0xFFFF)) {
        printf("OK\n");
    } else {
        printf("FAILED (%x != %x)\n",
               *((uint32_t *)tab_rp_registers), UT_IREAL);
        goto close;
    }

    printf("2/2 Get float: ");
    real = modbus_get_float(tab_rp_registers);
    if (real == UT_REAL) {
        printf("OK\n");
    } else {
        printf("FAILED (%f != %f)\n", real, UT_REAL);
        goto close;
    }

    printf("\nAt this point, error messages doesn't mean the test has failed\n");

    /** ILLEGAL DATA ADDRESS **/
    printf("\nTEST ILLEGAL DATA ADDRESS:\n");

    /* The mapping begins at 0 and ends at address + nb_points so
     * the addresses are not valid. */

    rc = modbus_read_bits(ctx, UT_BITS_ADDRESS,
                          UT_BITS_NB_POINTS + 1,
                          tab_rp_bits);
    printf("* modbus_read_bits: ");
    if (rc == -1 && errno == EMBXILADD) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
                                UT_INPUT_BITS_NB_POINTS + 1,
                                tab_rp_bits);
    printf("* modbus_read_input_bits: ");
    if (rc == -1 && errno == EMBXILADD)
        printf("OK\n");
    else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB_POINTS + 1,
                               tab_rp_registers);
    printf("* modbus_read_registers: ");
    if (rc == -1 && errno == EMBXILADD)
        printf("OK\n");
    else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
                                     UT_INPUT_REGISTERS_NB_POINTS + 1,
                                     tab_rp_registers);
    printf("* modbus_read_input_registers: ");
    if (rc == -1 && errno == EMBXILADD)
        printf("OK\n");
    else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_write_bit(ctx, UT_BITS_ADDRESS + UT_BITS_NB_POINTS, ON);
    printf("* modbus_write_bit: ");
    if (rc == -1 && errno == EMBXILADD) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_write_bits(ctx, UT_BITS_ADDRESS + UT_BITS_NB_POINTS,
                           UT_BITS_NB_POINTS,
                           tab_rp_bits);
    printf("* modbus_write_coils: ");
    if (rc == -1 && errno == EMBXILADD) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS +
                                UT_REGISTERS_NB_POINTS,
                                UT_REGISTERS_NB_POINTS,
                                tab_rp_registers);
    printf("* modbus_write_registers: ");
    if (rc == -1 && errno == EMBXILADD) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }


    /** TOO MANY DATA **/
    printf("\nTEST TOO MANY DATA ERROR:\n");

    rc = modbus_read_bits(ctx, UT_BITS_ADDRESS, MODBUS_MAX_READ_BITS + 1,
                          tab_rp_bits);
    printf("* modbus_read_bits: ");
    if (rc == -1 && errno == EMBMDATA) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
                                MODBUS_MAX_READ_BITS + 1,
                                tab_rp_bits);
    printf("* modbus_read_input_bits: ");
    if (rc == -1 && errno == EMBMDATA) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               MODBUS_MAX_READ_REGISTERS + 1,
                               tab_rp_registers);
    printf("* modbus_read_registers: ");
    if (rc == -1 && errno == EMBMDATA) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
                                     MODBUS_MAX_READ_REGISTERS + 1,
                                     tab_rp_registers);
    printf("* modbus_read_input_registers: ");
    if (rc == -1 && errno == EMBMDATA) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_write_bits(ctx, UT_BITS_ADDRESS,
                           MODBUS_MAX_WRITE_BITS + 1,
                           tab_rp_bits);
    printf("* modbus_write_bits: ");
    if (rc == -1 && errno == EMBMDATA) {
        printf("OK\n");
    } else {
        goto close;
        printf("FAILED\n");
    }

    rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS,
                                MODBUS_MAX_WRITE_REGISTERS + 1,
                                tab_rp_registers);
    printf("* modbus_write_registers: ");
    if (rc == -1 && errno == EMBMDATA) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /** SLAVE REPLY **/
    printf("\nTEST SLAVE REPLY:\n");
    modbus_set_slave(ctx, 18);
    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB_POINTS,
                               tab_rp_registers);
    if (use_backend == RTU) {
        /* No response in RTU mode */
        printf("1/4 No response from slave %d: ", 18);

        if (rc == -1 && errno == ETIMEDOUT) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
            goto close;
        }
    } else {
        /* Response in TCP mode */
        printf("1/4 Response from slave %d: ", 18);

        if (rc == UT_REGISTERS_NB_POINTS) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
            goto close;
        }
    }

    rc = modbus_set_slave(ctx, MODBUS_BROADCAST_ADDRESS);
    if (rc == -1) {
        printf("Invalid broacast address\n");
        goto close;
    }

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB_POINTS,
                               tab_rp_registers);
    printf("2/4 Reply after a broadcast query: ");
    if (rc == UT_REGISTERS_NB_POINTS) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Restore slave */
    if (use_backend == RTU) {
        modbus_set_slave(ctx, SERVER_ID);
    } else {
        modbus_set_slave(ctx, MODBUS_TCP_SLAVE);
    }

    printf("3/4 Report slave ID: \n");
    /* tab_rp_bits is used to store bytes */
    rc = modbus_report_slave_id(ctx, tab_rp_bits);
    if (rc == -1) {
        printf("FAILED\n");
        goto close;
    }

    if (((use_backend == RTU) && (tab_rp_bits[0] == SERVER_ID))
        || tab_rp_bits[0] == 0xFF) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Save original timeout */
    modbus_get_timeout_begin(ctx, &timeout_begin_old);

    /* Define a new and too short timeout */
    timeout_begin_new.tv_sec = 0;
    timeout_begin_new.tv_usec = 0;
    modbus_set_timeout_begin(ctx, &timeout_begin_new);

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB_POINTS,
                               tab_rp_registers);
    printf("4/4 Too short timeout: ");
    if (rc == -1 && errno == ETIMEDOUT) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Restore original timeout */
    modbus_set_timeout_begin(ctx, &timeout_begin_old);

    /* Wait for data before flushing */
    usleep(100);
    modbus_flush(ctx);

    /** BAD RESPONSE **/
    printf("\nTEST BAD RESPONSE ERROR:\n");

    /* Allocate only the required space */
    tab_rp_registers_bad = (uint16_t *) malloc(
        UT_REGISTERS_NB_POINTS_SPECIAL * sizeof(uint16_t));
    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB_POINTS_SPECIAL,
                               tab_rp_registers_bad);
    printf("* modbus_read_registers: ");
    if (rc == -1 && errno == EMBBADDATA) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    free(tab_rp_registers_bad);

    /** MANUAL EXCEPTION **/
    printf("\nTEST MANUAL EXCEPTION:\n");

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS_SPECIAL,
                               UT_REGISTERS_NB_POINTS,
                               tab_rp_registers);
    printf("* modbus_read_registers at special address: ");
    if (rc == -1 && errno == EMBXSBUSY) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    printf("\nALL TESTS PASS WITH SUCCESS.\n");

close:
    /* Free the memory */
    free(tab_rp_bits);
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
