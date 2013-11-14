/*
 * Copyright © 2008-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
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

int test_raw_request(modbus_t *, int);

int main(int argc, char *argv[])
{
    uint8_t *tab_rp_bits = 0;
    uint16_t *tab_rp_registers = 0;
    uint16_t *tab_rp_registers_bad;
    modbus_t *ctx;
    int i;
    uint8_t value;
    int nb_points;
    int rc;
    float real;
    uint32_t ireal;
    uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;
    uint32_t old_byte_to_sec;
    uint32_t old_byte_to_usec;
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
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);

    if (use_backend == RTU) {
          modbus_set_slave(ctx, SERVER_ID);
    }


    /* Save original timeout before/after connect, and ensure it isn't mutated */
    printf("\nTEST TIMEOUT MUTATION (CONNECT):\n");

    modbus_get_response_timeout(ctx, &old_response_timeout);
    {
        struct timeval beg, end;
        gettimeofday( &beg, NULL );
        if (modbus_connect(ctx) == -1) {
            fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;
        }
        gettimeofday( &end, NULL );
        printf( "\n  CONNECTED in %fs\n", (((double)end.tv_sec + (double)end.tv_usec / 1000000)
                                           - ((double)beg.tv_sec + (double)beg.tv_usec / 1000000)));
    }
    modbus_get_response_timeout(ctx, &response_timeout);
    rc = memcmp( &response_timeout, &old_response_timeout, sizeof response_timeout );
    if (rc == 0) {
        printf("OK\n");
    } else {
        printf("FAILED: modbus_t::response_timeout has been permanently modified by %fs (from %d/%d to %d/%d)\n", 
               ((double)response_timeout.tv_sec + (double)response_timeout.tv_usec / 1000000)
               - ((double)old_response_timeout.tv_sec + (double)old_response_timeout.tv_usec / 1000000),
               (int)old_response_timeout.tv_sec, (int)old_response_timeout.tv_usec, 
               (int)response_timeout.tv_sec, (int)response_timeout.tv_usec );
        goto close;
    }

    

    /* Allocate and initialize the memory to store the bits */
    nb_points = (UT_BITS_NB > UT_INPUT_BITS_NB) ? UT_BITS_NB : UT_INPUT_BITS_NB;
    tab_rp_bits = (uint8_t *) malloc(nb_points * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb_points * sizeof(uint8_t));

    /* Allocate and initialize the memory to store the registers */
    nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
        UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
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
        printf("FAILED (%0X != %0X)\n", tab_rp_bits[0], ON);
        goto close;
    }
    printf("OK\n");
    /* End single */

    /* Multiple bits */
    {
        uint8_t tab_value[UT_BITS_NB];

        modbus_set_bits_from_bytes(tab_value, 0, UT_BITS_NB, UT_BITS_TAB);
        rc = modbus_write_bits(ctx, UT_BITS_ADDRESS,
                               UT_BITS_NB, tab_value);
        printf("1/2 modbus_write_bits: ");
        if (rc == UT_BITS_NB) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
            goto close;
        }
    }

    rc = modbus_read_bits(ctx, UT_BITS_ADDRESS, UT_BITS_NB, tab_rp_bits);
    printf("2/2 modbus_read_bits: ");
    if (rc != UT_BITS_NB) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

    i = 0;
    nb_points = UT_BITS_NB;
    while (nb_points > 0) {
        int nb_bits = (nb_points > 8) ? 8 : nb_points;

        value = modbus_get_byte_from_bits(tab_rp_bits, i*8, nb_bits);
        if (value != UT_BITS_TAB[i]) {
            printf("FAILED (%0X != %0X)\n", value, UT_BITS_TAB[i]);
            goto close;
        }

        nb_points -= nb_bits;
        i++;
    }
    printf("OK\n");
    /* End of multiple bits */

    /** DISCRETE INPUTS **/
    rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
                                UT_INPUT_BITS_NB, tab_rp_bits);
    printf("1/1 modbus_read_input_bits: ");

    if (rc != UT_INPUT_BITS_NB) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

    i = 0;
    nb_points = UT_INPUT_BITS_NB;
    while (nb_points > 0) {
        int nb_bits = (nb_points > 8) ? 8 : nb_points;

        value = modbus_get_byte_from_bits(tab_rp_bits, i*8, nb_bits);
        if (value != UT_INPUT_BITS_TAB[i]) {
            printf("FAILED (%0X != %0X)\n", value, UT_INPUT_BITS_TAB[i]);
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
                                UT_REGISTERS_NB, UT_REGISTERS_TAB);
    printf("1/5 modbus_write_registers: ");
    if (rc == UT_REGISTERS_NB) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB, tab_rp_registers);
    printf("2/5 modbus_read_registers: ");
    if (rc != UT_REGISTERS_NB) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

    for (i=0; i < UT_REGISTERS_NB; i++) {
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
    if (rc != -1) {
        printf("FAILED (nb_points %d)\n", rc);
        goto close;
    }
    printf("OK\n");

    nb_points = (UT_REGISTERS_NB >
                 UT_INPUT_REGISTERS_NB) ?
        UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));

    /* Write registers to zero from tab_rp_registers and store read registers
       into tab_rp_registers. So the read registers must set to 0, except the
       first one because there is an offset of 1 register on write. */
    rc = modbus_write_and_read_registers(ctx,
                                         UT_REGISTERS_ADDRESS + 1,
                                         UT_REGISTERS_NB - 1,
                                         tab_rp_registers,
                                         UT_REGISTERS_ADDRESS,
                                         UT_REGISTERS_NB,
                                         tab_rp_registers);
    printf("4/5 modbus_write_and_read_registers: ");
    if (rc != UT_REGISTERS_NB) {
        printf("FAILED (nb points %d != %d)\n", rc, UT_REGISTERS_NB);
        goto close;
    }

    if (tab_rp_registers[0] != UT_REGISTERS_TAB[0]) {
        printf("FAILED (%0X != %0X)\n",
               tab_rp_registers[0], UT_REGISTERS_TAB[0]);
    }

    for (i=1; i < UT_REGISTERS_NB; i++) {
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
                                     UT_INPUT_REGISTERS_NB,
                                     tab_rp_registers);
    printf("1/1 modbus_read_input_registers: ");
    if (rc != UT_INPUT_REGISTERS_NB) {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }

    for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
        if (tab_rp_registers[i] != UT_INPUT_REGISTERS_TAB[i]) {
            printf("FAILED (%0X != %0X)\n",
                   tab_rp_registers[i], UT_INPUT_REGISTERS_TAB[i]);
            goto close;
        }
    }
    printf("OK\n");

    printf("\nTEST FLOATS\n");
    /** FLOAT **/
    printf("1/4 Set float: ");
    modbus_set_float(UT_REAL, tab_rp_registers);
    if (tab_rp_registers[1] == (UT_IREAL >> 16) &&
        tab_rp_registers[0] == (UT_IREAL & 0xFFFF)) {
        printf("OK\n");
    } else {
        /* Avoid *((uint32_t *)tab_rp_registers)
         * https://github.com/stephane/libmodbus/pull/104 */
        ireal = (uint32_t) tab_rp_registers[0] & 0xFFFF;
        ireal |= (uint32_t) tab_rp_registers[1] << 16;
        printf("FAILED (%x != %x)\n", ireal, UT_IREAL);
        goto close;
    }

    printf("2/4 Get float: ");
    real = modbus_get_float(tab_rp_registers);
    if (real == UT_REAL) {
        printf("OK\n");
    } else {
        printf("FAILED (%f != %f)\n", real, UT_REAL);
        goto close;
    }

    printf("3/4 Set float in DBCA order: ");
    modbus_set_float_dcba(UT_REAL, tab_rp_registers);
    if (tab_rp_registers[1] == (UT_IREAL_DCBA >> 16) &&
        tab_rp_registers[0] == (UT_IREAL_DCBA & 0xFFFF)) {
        printf("OK\n");
    } else {
        ireal = (uint32_t) tab_rp_registers[0] & 0xFFFF;
        ireal |= (uint32_t) tab_rp_registers[1] << 16;
        printf("FAILED (%x != %x)\n", ireal, UT_IREAL_DCBA);
        goto close;
    }

    printf("4/4 Get float in DCBA order: ");
    real = modbus_get_float_dcba(tab_rp_registers);
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
                          UT_BITS_NB + 1, tab_rp_bits);
    printf("* modbus_read_bits: ");
    if (rc == -1 && errno == EMBXILADD) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
                                UT_INPUT_BITS_NB + 1, tab_rp_bits);
    printf("* modbus_read_input_bits: ");
    if (rc == -1 && errno == EMBXILADD)
        printf("OK\n");
    else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB + 1, tab_rp_registers);
    printf("* modbus_read_registers: ");
    if (rc == -1 && errno == EMBXILADD)
        printf("OK\n");
    else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_input_registers(ctx, UT_INPUT_REGISTERS_ADDRESS,
                                     UT_INPUT_REGISTERS_NB + 1,
                                     tab_rp_registers);
    printf("* modbus_read_input_registers: ");
    if (rc == -1 && errno == EMBXILADD)
        printf("OK\n");
    else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_write_bit(ctx, UT_BITS_ADDRESS + UT_BITS_NB, ON);
    printf("* modbus_write_bit: ");
    if (rc == -1 && errno == EMBXILADD) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_write_bits(ctx, UT_BITS_ADDRESS + UT_BITS_NB,
                           UT_BITS_NB, tab_rp_bits);
    printf("* modbus_write_coils: ");
    if (rc == -1 && errno == EMBXILADD) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
                               UT_REGISTERS_NB, tab_rp_registers);
    printf("* modbus_write_registers: ");
    if (rc == -1 && errno == EMBXILADD) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /** TOO MANY DATA **/
    printf("\nTEST TOO MANY DATA ERROR:\n");

    rc = modbus_read_bits(ctx, UT_BITS_ADDRESS,
                          MODBUS_MAX_READ_BITS + 1, tab_rp_bits);
    printf("* modbus_read_bits: ");
    if (rc == -1 && errno == EMBMDATA) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_read_input_bits(ctx, UT_INPUT_BITS_ADDRESS,
                                MODBUS_MAX_READ_BITS + 1, tab_rp_bits);
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
                           MODBUS_MAX_WRITE_BITS + 1, tab_rp_bits);
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
    modbus_set_slave(ctx, INVALID_SERVER_ID);
    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB, tab_rp_registers);
    if (use_backend == RTU) {
        const int RAW_REQ_LENGTH = 6;
        uint8_t raw_req[] = { INVALID_SERVER_ID, 0x03, 0x00, 0x01, 0x01, 0x01 };
        /* Too many points */
        uint8_t raw_invalid_req[] = { INVALID_SERVER_ID, 0x03, 0x00, 0x01, 0xFF, 0xFF };
        const int RAW_REP_LENGTH = 7;
        uint8_t raw_rep[] = { INVALID_SERVER_ID, 0x03, 0x04, 0, 0, 0, 0 };
        uint8_t rsp[MODBUS_RTU_MAX_ADU_LENGTH];

        /* No response in RTU mode */
        printf("1/5-A No response from slave %d: ", INVALID_SERVER_ID);

        if (rc == -1 && errno == ETIMEDOUT) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
            goto close;
        }

        /* The slave raises a timeout on a confirmation to ignore because if an
         * indication for another slave is received, a confirmation must follow */


        /* Send a pair of indication/confirmation to the slave with a different
         * slave ID to simulate a communication on a RS485 bus. At first, the
         * slave will see the indication message then the confirmation, and it must
         * ignore both. */
        modbus_send_raw_request(ctx, raw_req, RAW_REQ_LENGTH * sizeof(uint8_t));
        modbus_send_raw_request(ctx, raw_rep, RAW_REP_LENGTH * sizeof(uint8_t));
        rc = modbus_receive_confirmation(ctx, rsp);

        printf("1/5-B No response from slave %d on indication/confirmation messages: ",
               INVALID_SERVER_ID);

        if (rc == -1 && errno == ETIMEDOUT) {
            printf("OK\n");
        } else {
            printf("FAILED (%d)\n", rc);
            goto close;
        }

        /* Send an INVALID request for another slave */
        modbus_send_raw_request(ctx, raw_invalid_req, RAW_REQ_LENGTH * sizeof(uint8_t));
        rc = modbus_receive_confirmation(ctx, rsp);

        printf("1/5-C No response from slave %d with invalid request: ",
               INVALID_SERVER_ID);

        if (rc == -1 && errno == ETIMEDOUT) {
            printf("OK\n");
        } else {
            printf("FAILED (%d)\n", rc);
            goto close;
        }
    } else {
        /* Response in TCP mode */
        printf("1/4 Response from slave %d: ", INVALID_SERVER_ID);

        if (rc == UT_REGISTERS_NB) {
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
                               UT_REGISTERS_NB, tab_rp_registers);
    printf("2/5 Reply after a broadcast query: ");
    if (rc == UT_REGISTERS_NB) {
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

    printf("3/5 Report slave ID: \n");
    /* tab_rp_bits is used to store bytes */
    rc = modbus_report_slave_id(ctx, tab_rp_bits);
    if (rc == -1) {
        printf("FAILED\n");
        goto close;
    }

    /* Slave ID is an arbitraty number for libmodbus */
    if (rc > 0) {
        printf("OK Slave ID is %d\n", tab_rp_bits[0]);
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Run status indicator */
    if (rc > 1 && tab_rp_bits[1] == 0xFF) {
        printf("OK Run Status Indicator is %s\n", tab_rp_bits[1] ? "ON" : "OFF");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Print additional data as string */
    if (rc > 2) {
        printf("Additional data: ");
        for (i=2; i < rc; i++) {
            printf("%c", tab_rp_bits[i]);
        }
        printf("\n");
    }

    printf("5/5 Response with an invalid TID or slave: ");
    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE,
                               1, tab_rp_registers);
    if (rc == -1) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Save original timeout */
    modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);
    modbus_get_byte_timeout(ctx, &old_byte_to_sec, &old_byte_to_usec);

    rc = modbus_set_response_timeout(ctx, 0, 0);
    printf("1/6 Invalid response timeout (zero): ");
    if (rc == -1 && errno == EINVAL) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_set_response_timeout(ctx, 0, 1000000);
    printf("2/6 Invalid response timeout (too large us): ");
    if (rc == -1 && errno == EINVAL) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    rc = modbus_set_byte_timeout(ctx, 0, 1000000);
    printf("3/6 Invalid byte timeout (too large us): ");
    if (rc == -1 && errno == EINVAL) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    modbus_set_response_timeout(ctx, 0, 1);
    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB, tab_rp_registers);
    printf("4/6 1us response timeout: ");
    if (rc == -1 && errno == ETIMEDOUT) {
        printf("OK\n");
    } else {
        printf("FAILED (can fail on some platforms)\n");
    }

    /* A wait and flush operation is done by the error recovery code of
     * libmodbus but after a sleep of current response timeout
     * so 0 can't be too short!
     */
    usleep(old_response_to_sec * 1000000 + old_response_to_usec);
    modbus_flush(ctx);

    /* Trigger a special behaviour on server to wait for 0.5 second before
     * replying whereas allowed timeout is 0.2 second */
    modbus_set_response_timeout(ctx, 0, 200000);
    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS_SLEEP_500_MS,
                               1, tab_rp_registers);
    printf("5/6 Too short response timeout (0.2s < 0.5s): ");
    if (rc == -1 && errno == ETIMEDOUT) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Wait for reply (0.2 + 0.4 > 0.5 s) and flush before continue */
    usleep(400000);
    modbus_flush(ctx);

    modbus_set_response_timeout(ctx, 0, 600000);
    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS_SLEEP_500_MS,
                               1, tab_rp_registers);
    printf("6/6 Adequate response timeout (0.6s > 0.5s): ");
    if (rc == 1) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Disable the byte timeout.
       The full response must be available in the 600ms interval */
    modbus_set_byte_timeout(ctx, 0, 0);
    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS_SLEEP_500_MS,
                               1, tab_rp_registers);
    printf("7/7 Disable byte timeout: ");
    if (rc == 1) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /* Restore original response timeout */
    modbus_set_response_timeout(ctx, old_response_to_sec,
                                old_response_to_usec);

    if (use_backend == TCP) {
        /* Test server is only able to test byte timeout with the TCP backend */

        /* Timeout of 3ms between bytes */
        modbus_set_byte_timeout(ctx, 0, 3000);
        rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS_BYTE_SLEEP_5_MS,
                                   1, tab_rp_registers);
        printf("1/2 Too small byte timeout (3ms < 5ms): ");
        if (rc == -1 && errno == ETIMEDOUT) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
            goto close;
        }

        /* Wait remaing bytes before flushing */
        usleep(11 * 5000);
        modbus_flush(ctx);

        /* Timeout of 10ms between bytes */
        modbus_set_byte_timeout(ctx, 0, 7000);
        rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS_BYTE_SLEEP_5_MS,
                                   1, tab_rp_registers);
        printf("2/2 Adapted byte timeout (7ms > 5ms): ");
        if (rc == 1) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
            goto close;
        }
    }

    /* Restore original byte timeout */
    modbus_set_byte_timeout(ctx, old_byte_to_sec, old_byte_to_usec);

    /** BAD RESPONSE **/
    printf("\nTEST BAD RESPONSE ERROR:\n");

    /* Allocate only the required space */
    tab_rp_registers_bad = (uint16_t *) malloc(
        UT_REGISTERS_NB_SPECIAL * sizeof(uint16_t));

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB_SPECIAL, tab_rp_registers_bad);
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
                               UT_REGISTERS_NB, tab_rp_registers);

    printf("* modbus_read_registers at special address: ");
    if (rc == -1 && errno == EMBXSBUSY) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
        goto close;
    }

    /** RAW REQUEST */
    if (test_raw_request(ctx, use_backend) == -1) {
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

int test_raw_request(modbus_t *ctx, int use_backend)
{
    int rc;
    int i, j;
    const int RAW_REQ_LENGTH = 6;
    uint8_t raw_req[] = {
        /* slave */
        (use_backend == RTU) ? SERVER_ID : 0xFF,
        /* function, addr 1, 5 values */
        MODBUS_FC_READ_HOLDING_REGISTERS, 0x00, 0x01, 0x0, 0x05,
    };
    /* Write and read registers request */
    uint8_t raw_rw_req[] = {
        /* slave */
        (use_backend == RTU) ? SERVER_ID : 0xFF,
        /* function, addr to read, nb to read */
        MODBUS_FC_WRITE_AND_READ_REGISTERS,
        /* Read */
        0, 0,
        (MODBUS_MAX_WR_READ_REGISTERS + 1) >> 8,
        (MODBUS_MAX_WR_READ_REGISTERS + 1) & 0xFF,
        /* Write */
        0, 0,
        0, 1,
        /* Write byte count */
        1 * 2,
        /* One data to write... */
        0x12, 0x34
    };
    /* See issue #143, test with MAX_WR_WRITE_REGISTERS */
    int req_length;
    uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];
    int tab_function[] = {
        MODBUS_FC_READ_COILS,
        MODBUS_FC_READ_DISCRETE_INPUTS,
        MODBUS_FC_READ_HOLDING_REGISTERS,
        MODBUS_FC_READ_INPUT_REGISTERS
    };
    int tab_nb_max[] = {
        MODBUS_MAX_READ_BITS + 1,
        MODBUS_MAX_READ_BITS + 1,
        MODBUS_MAX_READ_REGISTERS + 1,
        MODBUS_MAX_READ_REGISTERS + 1
    };
    int length;
    int offset;
    const int EXCEPTION_RC = 2;

    if (use_backend == RTU) {
        length = 3;
        offset = 1;
    } else {
        length = 7;
        offset = 7;
    }

    printf("\nTEST RAW REQUESTS:\n");

    req_length = modbus_send_raw_request(ctx, raw_req,
                                         RAW_REQ_LENGTH * sizeof(uint8_t));
    printf("* modbus_send_raw_request: ");
    if (req_length == (length + 5)) {
        printf("OK\n");
    } else {
        printf("FAILED (%d)\n", req_length);
        return -1;
    }

    printf("* modbus_receive_confirmation: ");
    rc  = modbus_receive_confirmation(ctx, rsp);
    if (rc == (length + 12)) {
        printf("OK\n");
    } else {
        printf("FAILED (%d)\n", rc);
        return -1;
    }

    /* Try to crash server with raw requests to bypass checks of client. */

    /* Address */
    raw_req[2] = 0;
    raw_req[3] = 0;

    /* Try to read more values than a response could hold for all data
     * types.
     */
    for (i=0; i<4; i++) {
        raw_req[1] = tab_function[i];

        for (j=0; j<2; j++) {
            if (j == 0) {
                /* Try to read zero values on first iteration */
                raw_req[4] = 0x00;
                raw_req[5] = 0x00;
            } else {
                /* Try to read max values + 1 on second iteration */
                raw_req[4] = (tab_nb_max[i] >> 8) & 0xFF;
                raw_req[5] = tab_nb_max[i] & 0xFF;
            }

            req_length = modbus_send_raw_request(ctx, raw_req,
                                                 RAW_REQ_LENGTH * sizeof(uint8_t));
            if (j == 0) {
                printf("* try to read 0 values with function %d: ", tab_function[i]);
            } else {
                printf("* try an exploit with function %d: ", tab_function[i]);
            }
            rc  = modbus_receive_confirmation(ctx, rsp);
            if (rc == (length + EXCEPTION_RC) &&
                rsp[offset] == (0x80 + tab_function[i]) &&
                rsp[offset + 1] == MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE) {
                printf("OK\n");
            } else {
                printf("FAILED\n");
                return -1;
            }
        }
    }

    /* Modbus write and read multiple registers */
    i = 0;
    tab_function[i] = MODBUS_FC_WRITE_AND_READ_REGISTERS;
    for (j=0; j<2; j++) {
        if (j == 0) {
            /* Try to read zero values on first iteration */
            raw_rw_req[4] = 0x00;
            raw_rw_req[5] = 0x00;
        } else {
            /* Try to read max values + 1 on second iteration */
            raw_rw_req[4] = (MODBUS_MAX_WR_READ_REGISTERS + 1) >> 8;
            raw_rw_req[5] = (MODBUS_MAX_WR_READ_REGISTERS + 1) & 0xFF;
        }
        req_length = modbus_send_raw_request(ctx, raw_rw_req, 13 * sizeof(uint8_t));
        if (j == 0) {
            printf("* try to read 0 values with function %d: ", tab_function[i]);
        } else {
            printf("* try an exploit with function %d: ", tab_function[i]);
        }
        rc = modbus_receive_confirmation(ctx, rsp);
        if (rc == length + EXCEPTION_RC &&
            rsp[offset] == (0x80 + tab_function[i]) &&
            rsp[offset + 1] == MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE) {
            printf("OK\n");
        } else {
            printf("FAILED\n");
            return -1;
        }
    }

    return 0;
}
