#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <modbus.h>

#include "unit-test.h"

const uint16_t UT_BITS_ADDRESS = 0x13;
const uint16_t UT_BITS_NB = 0x25;
const uint8_t UT_BITS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B };

const uint16_t UT_INPUT_BITS_ADDRESS = 0xC4;
const uint16_t UT_INPUT_BITS_NB = 0x16;
const uint8_t UT_INPUT_BITS_TAB[] = { 0xAC, 0xDB, 0x35 };

const uint16_t UT_REGISTERS_ADDRESS = 0x6B;
/* Raise a manual exception when this address is used for the first byte */
const uint16_t UT_REGISTERS_ADDRESS_SPECIAL = 0x6C;
/* The response of the server will contains an invalid TID or slave */
const uint16_t UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE = 0x6D;

const uint16_t UT_REGISTERS_NB = 0x3;
const uint16_t UT_REGISTERS_TAB[] = { 0x022B, 0x0001, 0x0064 };
/* If the following value is used, a bad response is sent.
   It's better to test with a lower value than
   UT_REGISTERS_NB_POINTS to try to raise a segfault. */
const uint16_t UT_REGISTERS_NB_SPECIAL = 0x2;

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x08;
const uint16_t UT_INPUT_REGISTERS_NB = 0x1;
const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x000A };

const float UT_REAL = 916.540649;
const uint32_t UT_IREAL = 0x4465229a;
const uint32_t UT_IREAL_DCBA = 0x9a226544;


/** Initalized the mapping **/
static int mapping_init(modbus_mapping_t* mapping);


int unit_test_server_listen(unit_test_server_t* server)
{
    server->socket = -1;
    if (server->use_backend == TCP) {
        server->ctx = modbus_new_tcp("127.0.0.1", 1502);
    } else if (server->use_backend == TCP_PI) {
        server->ctx = modbus_new_tcp_pi("::0", "1502");
    } else {
        server->ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
        modbus_set_slave(server->ctx, SERVER_ID);
    }

    modbus_set_debug(server->ctx, TRUE);

    server->mb_mapping = modbus_mapping_new(
        UT_BITS_ADDRESS + UT_BITS_NB,
        UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB,
        UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
        UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB);
    if (server->mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(server->ctx);
        return -1;
    }

    /** Listen **/
    if (server->use_backend == TCP) {
        server->socket = modbus_tcp_listen(server->ctx, 1);
    } else if (server->use_backend == TCP_PI) {
        server->socket = modbus_tcp_pi_listen(server->ctx, 1);
    }
    return 0;
}

int unit_test_server_run(unit_test_server_t* server)
{
    int rc = 0;
    uint8_t* query = NULL;
    int header_length = modbus_get_header_length(server->ctx);

    /** Init modbus mapping **/
    rc = mapping_init(server->mb_mapping);
    if(rc) goto error;

    /** accept connection**/
    switch(server->use_backend)
    {
    case TCP:
        modbus_tcp_accept(server->ctx, &server->socket);
        query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
        break;
    case TCP_PI:
        modbus_tcp_pi_accept(server->ctx, &server->socket);
        query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
        break;
    case RTU:
        rc = modbus_connect(server->ctx);
        query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
        if (rc == -1) {
            fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
            goto error;
        }
        break;
    default:
        assert(0);
        break;
    }

    while(rc != -1) {
        do {
            rc = modbus_receive(server->ctx,query);
            /* Filtered queries return 0 */
        } while (rc == 0);
        if (rc == -1) {
            /* Connection closed by the client or error */
            break;
        }

        /* Read holding registers */
        if (query[header_length] == 0x03) {
            if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 3)
                == UT_REGISTERS_NB_SPECIAL) {
                printf("Set an incorrect number of values\n");
                MODBUS_SET_INT16_TO_INT8(query, header_length + 3,
                                         UT_REGISTERS_NB_SPECIAL - 1);
            } else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
                       == UT_REGISTERS_ADDRESS_SPECIAL) {
                printf("Reply to this special register address by an exception\n");
                modbus_reply_exception(server->ctx, query,
                                       MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
                continue;
            } else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
                       == UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE) {
                const int RAW_REQ_LENGTH = 5;
                uint8_t raw_req[] = {
                    (server->use_backend == RTU) ? INVALID_SERVER_ID : 0xFF,
                    0x03,
                    0x02, 0x00, 0x00
                };

                printf("Reply with an invalid TID or slave\n");
                modbus_send_raw_request(server->ctx, raw_req, RAW_REQ_LENGTH * sizeof(uint8_t));
                continue;
            }
        }

        rc = modbus_reply(server->ctx, query, rc, server->mb_mapping);
    }

    error:
    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (server->use_backend == TCP) {
        if (server->socket != -1) {
            close(server->socket);
        }
    }
    modbus_mapping_free(server->mb_mapping);
    free(query);
    /* For RTU */
    modbus_close(server->ctx);
    modbus_free(server->ctx);

    return rc;
}


static int mapping_init(modbus_mapping_t* mb_mapping)
{
    /* Unit tests of modbus_mapping_new (tests would not be sufficient if two nb_* were identical) */
    if (mb_mapping->nb_bits != UT_BITS_ADDRESS + UT_BITS_NB) {
        printf("Invalid nb bits (%d != %d)\n", UT_BITS_ADDRESS + UT_BITS_NB, mb_mapping->nb_bits);
        return -1;
    }

    if (mb_mapping->nb_input_bits != UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB) {
        printf("Invalid nb input bits: %d\n", UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB);
        return -1;
    }

    if (mb_mapping->nb_registers != UT_REGISTERS_ADDRESS + UT_REGISTERS_NB) {
        printf("Invalid nb registers: %d\n", UT_REGISTERS_ADDRESS + UT_REGISTERS_NB);
        return -1;
    }

    if (mb_mapping->nb_input_registers != UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB) {
        printf("Invalid bb input registers: %d\n", UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB);
        return -1;
    }
    /* Examples from PI_MODBUS_300.pdf.
       Only the read-only input values are assigned. */

    /** INPUT STATUS **/
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits,
                               UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
                               UT_INPUT_BITS_TAB);

    /** INPUT REGISTERS **/
    {
        int i;
        for (i=0; i < UT_INPUT_REGISTERS_NB; i++) {
            mb_mapping->tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
                UT_INPUT_REGISTERS_TAB[i];;
        }
    }

    return 0;
}

int unit_test_client_start(int use_backend)
{
    uint8_t *tab_rp_bits;
    uint16_t *tab_rp_registers;
    uint16_t *tab_rp_registers_bad;
    modbus_t *ctx;
    int i;
    uint8_t value;
    int nb_points;
    int rc;
    float real;
    uint32_t ireal;
    struct timeval old_response_timeout;
    struct timeval response_timeout;

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

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
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
    if (rc != 0) {
        printf("FAILED (nb points %d)\n", rc);
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
                                         UT_REGISTERS_ADDRESS + 1, UT_REGISTERS_NB - 1,
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

    rc = modbus_write_registers(ctx, UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
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


        /* Send a pair of indication/confimration to the slave with a different
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
    modbus_get_response_timeout(ctx, &old_response_timeout);

    /* Define a new and too short timeout */
    response_timeout.tv_sec = 0;
    response_timeout.tv_usec = 0;
    modbus_set_response_timeout(ctx, &response_timeout);

    rc = modbus_read_registers(ctx, UT_REGISTERS_ADDRESS,
                               UT_REGISTERS_NB, tab_rp_registers);
    printf("4/4 Too short timeout: ");
    if (rc == -1 && errno == ETIMEDOUT) {
        printf("OK\n");
    } else {
        printf("FAILED (can fail on slow systems or Windows)\n");
    }

    /* Restore original timeout */
    modbus_set_response_timeout(ctx, &old_response_timeout);

    /* A wait and flush operation is done by the error recovery code of
     * libmodbus */

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
    printf("\nTEST RAW REQUEST:\n");
    {
        const int RAW_REQ_LENGTH = 6;
        uint8_t raw_req[] = { (use_backend == RTU) ? SERVER_ID : 0xFF,
                              0x03, 0x00, 0x01, 0x0, 0x05 };
        int req_length;
        uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];

        req_length = modbus_send_raw_request(ctx, raw_req,
                                             RAW_REQ_LENGTH * sizeof(uint8_t));

        printf("* modbus_send_raw_request: ");
        if ((use_backend == RTU && req_length == (RAW_REQ_LENGTH + 2)) ||
            ((use_backend == TCP || use_backend == TCP_PI) &&
             req_length == (RAW_REQ_LENGTH + 6))) {
            printf("OK\n");
        } else {
            printf("FAILED (%d)\n", req_length);
            goto close;
        }

        printf("* modbus_receive_confirmation: ");
        rc  = modbus_receive_confirmation(ctx, rsp);
        if ((use_backend == RTU && rc == 15) ||
            ((use_backend == TCP || use_backend == TCP_PI) &&
             rc == 19)) {
            printf("OK\n");
        } else {
            printf("FAILED (%d)\n", rc);
            goto close;
        }
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
