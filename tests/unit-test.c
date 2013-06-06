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
