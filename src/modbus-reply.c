/*
 * Copyright © 2001-2011 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * This library implements the Modbus protocol.
 * http://libmodbus.org/
 */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <config.h>

#include "modbus-private.h"
#include "modbus.h"

/* Build the exception response */
static int response_exception(modbus_t *ctx, sft_t *sft,
                              int exception_code, uint8_t *rsp,
                              unsigned int to_flush,
                              const char *template, ...)
{
    int rsp_length;

    /* Print debug message */
    if (ctx->debug) {
        va_list ap;

        va_start(ap, template);
        vfprintf(stderr, template, ap);
        va_end(ap);
    }

    /* Flush if required */
    if (to_flush) {
        _sleep_response_timeout(ctx);
        modbus_flush(ctx);
    }

    /* Build exception response */
    sft->function = sft->function + 0x80;
    rsp_length = ctx->backend->build_response_basis(sft, rsp);
    rsp[rsp_length++] = exception_code;

    return rsp_length;
}

static const char *names[] = {
    [MODBUS_FC_READ_COILS] = "read_bits",
    [MODBUS_FC_READ_DISCRETE_INPUTS] = "read_input_bits",
    [MODBUS_FC_READ_HOLDING_REGISTERS] = "read_registers",
    [MODBUS_FC_READ_INPUT_REGISTERS] = "read_input_registers",
    [MODBUS_FC_WRITE_SINGLE_COIL] = "write_bit",
    [MODBUS_FC_WRITE_SINGLE_REGISTER] = "write_register",
    [MODBUS_FC_READ_EXCEPTION_STATUS] = "read_exception_status",
    [MODBUS_FC_WRITE_MULTIPLE_COILS] = "write_multiple_bits",
    [MODBUS_FC_WRITE_MULTIPLE_REGISTERS] = "write_multiple_registers",
    [MODBUS_FC_REPORT_SLAVE_ID] = "report_slave_id",
    [MODBUS_FC_MASK_WRITE_REGISTER] = "mask_write_register",
    [MODBUS_FC_WRITE_AND_READ_REGISTERS] = "write_and_read_registers",
};

int modbus_set_reply_callbacks(modbus_t *ctx, const modbus_reply_callbacks_t *cb, void *user_ctx)
{
    if (ctx == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (cb != NULL &&
        (cb->verify == NULL ||
         cb->read == NULL ||
         cb->write == NULL)) {

        if (ctx->debug)
            printf("callback-structure is not correctly populated\n");

        errno = EINVAL;
        return -1;
    }

    if (ctx->backend->backend_type == _MODBUS_BACKEND_TYPE_RTU &&
        cb->accept_rtu_slave == NULL) {
        if (ctx->debug)
            printf("callback-structure is not correctly populated - missing accept_rtu_slave\n");
        errno = EINVAL;
        return -1;
    }

    ctx->reply_cb = cb;
    ctx->reply_user_ctx = user_ctx;

    return 0;
}

/* Send a response to the received request.
   Analyses the request and constructs a response.

   If an error occurs, this function construct the response
   accordingly.
*/
int modbus_reply_callback(modbus_t *ctx, const uint8_t *req, int req_length)
{
    int offset;
    int slave;
    int function;
    uint16_t address;
    uint8_t rsp[MAX_MESSAGE_LENGTH];
    int rsp_length = 0;
    sft_t sft;

    if (ctx == NULL || ctx->reply_cb == NULL) {
        errno = EINVAL;
        return -1;
    }

    offset = ctx->backend->header_length;
    slave = req[offset - 1];
    function = req[offset];
    address = (req[offset + 1] << 8) + req[offset + 2];

    /* special RTU-cases error checking */
    if (ctx->backend->backend_type == _MODBUS_BACKEND_TYPE_RTU) {
        /* we accept BROADCAST_ADDRESSes */
        if (slave != MODBUS_BROADCAST_ADDRESS) {
            /* check whether this slave is handled by this instance and
             * suppress any responses when the slave-id not accepted by the user */
            if (ctx->reply_cb->accept_rtu_slave(ctx->reply_user_ctx, slave) == FALSE) {
                if (ctx->debug)
                    fprintf(stderr, "slave ID %d is not handled by this instance\n", slave);
                return 0;
            }
        }

        // TODO broadcast responses should use the slave-id, probably
    }

    sft.slave = slave;
    sft.function = function;
    sft.t_id = ctx->backend->prepare_response_tid(req, &req_length);

    /* first do some verifications
     * for read and write this is the first stage only
     * "simple"-function-replies are constructed here */

    int nb = 0;       /* extracted number of values to written or read */
    int max_nb = 0;   /* maximum number of values to written or read */
    int is_read = 0;  /* is this a read-request */
    int verified = 0; /* return-code of intermediate verify-calls */

    switch (function) {
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_READ_DISCRETE_INPUTS:
        is_read = 1;
    /* fall-through */
    case MODBUS_FC_WRITE_MULTIPLE_COILS:
        nb = (req[offset + 3] << 8) + req[offset + 4];
        max_nb = is_read ? MODBUS_MAX_READ_BITS : MODBUS_MAX_WRITE_BITS;
        break;
    case MODBUS_FC_WRITE_SINGLE_COIL:
        nb = 1;
        max_nb = MODBUS_MAX_WRITE_BITS;
        break;
    case MODBUS_FC_MASK_WRITE_REGISTER:
    case MODBUS_FC_WRITE_SINGLE_REGISTER:
        nb = 1;
        max_nb = MODBUS_MAX_WRITE_REGISTERS;
        break;
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_READ_INPUT_REGISTERS:
        is_read = 1;
    /* fall-through */
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        nb = (req[offset + 3] << 8) + req[offset + 4];
        max_nb = is_read ? MODBUS_MAX_READ_REGISTERS : MODBUS_MAX_WRITE_REGISTERS;
        break;
    case MODBUS_FC_WRITE_AND_READ_REGISTERS:
        nb = (req[offset + 3] << 8) + req[offset + 4];
        max_nb = MODBUS_MAX_WR_READ_REGISTERS;

        { /* write-part is verified here */
            uint16_t address_write = (req[offset + 5] << 8) + req[offset + 6];
            int nb_write = (req[offset + 7] << 8) + req[offset + 8];

            /* first the address verification */
            verified = ctx->reply_cb->verify(ctx->reply_user_ctx, slave, MODBUS_FC_WRITE_MULTIPLE_REGISTERS,
                                             address_write, nb_write);

            int nb_write_bytes = req[offset + 9];
            if (nb_write < 1 ||
                MODBUS_MAX_WR_WRITE_REGISTERS < nb_write ||
                nb_write_bytes != nb_write * 2) {

                rsp_length = response_exception(
                    ctx, &sft, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, TRUE,
                    "Illegal nb of values (W%d, R%d) in write_and_read_registers (max W%d, R%d)\n",
                    nb_write, nb, MODBUS_MAX_WR_WRITE_REGISTERS, MODBUS_MAX_WR_READ_REGISTERS);
            }
        }
        break;
    case MODBUS_FC_REPORT_SLAVE_ID: {
        int str_len;
        int byte_count_pos;

        rsp_length = ctx->backend->build_response_basis(&sft, rsp);
        /* Skip byte count for now */
        byte_count_pos = rsp_length++;
        rsp[rsp_length++] = _REPORT_SLAVE_ID;
        /* Run indicator status to ON */
        rsp[rsp_length++] = 0xFF;
        /* LMB + length of LIBMODBUS_VERSION_STRING */
        str_len = 3 + strlen(LIBMODBUS_VERSION_STRING);
        memcpy(rsp + rsp_length, "LMB" LIBMODBUS_VERSION_STRING, str_len);
        rsp_length += str_len;
        rsp[byte_count_pos] = rsp_length - byte_count_pos - 1;
    } break;
    case MODBUS_FC_READ_EXCEPTION_STATUS:
        if (ctx->debug) {
            fprintf(stderr, "FIXME Not implemented\n");
        }
        errno = ENOPROTOOPT;
        return -1;
        break;
    default:
        rsp_length = response_exception(
            ctx, &sft, MODBUS_EXCEPTION_ILLEGAL_FUNCTION, rsp, TRUE,
            "Unknown Modbus function code: 0x%0X\n", function);
        break;
    }

    if (ctx->debug)
        fprintf(stderr, "function %s (%x), %d, %d, max: %d, resp: %d\n",
                names[function], function, address, nb, max_nb, rsp_length);

    /* we already have a response - we are done */
    if (rsp_length > 0)
        goto send_response;

    /* verify this (second part) of the read/write access
     * MODBUS_FC_WRITE_AND_READ_REGISTERS has two verifications to be done - one is aleady done */
    if (verified == 0)
        verified = ctx->reply_cb->verify(ctx->reply_user_ctx, slave, function, address, nb);

    /* out of reply-buffer-range */
    if (nb < 1 || max_nb < nb) {
        /* Maybe the indication has been truncated on reading because of
         * invalid address (eg. nb is 0 but the request contains values to
         * write) so it's necessary to flush. */
        rsp_length = response_exception(
            ctx, &sft, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, TRUE,
            "Illegal nb of values %d in %s (max %d)\n",
            nb, names[function], max_nb);
        goto send_response;
    }

    if (verified == EMBXILADD) { /* verify found an invalid address */
        rsp_length = response_exception(
            ctx, &sft,
            MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp, FALSE,
            "Illegal data address 0x%0X in %s\n",
            address, names[function]);
        goto send_response;
    } else if (verified == EMBXILFUN) { /* verify found an invalid function */
        rsp_length = response_exception(
            ctx, &sft,
            MODBUS_EXCEPTION_ILLEGAL_FUNCTION, rsp, FALSE,
            "Slave/client does not accept Modbus function code: 0x%0X (%s)\n",
            function, names[function]);
        goto send_response;
    } else if (verified != 0) { /* another error has occured */
        errno = EINVAL;
        return -1;
    }

    /* user verification was successful */

    int rc;
    rsp_length = ctx->backend->build_response_basis(&sft, rsp);

    switch (function) {
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_READ_DISCRETE_INPUTS:
        rsp[rsp_length++] = (nb / 8) + ((nb % 8) ? 1 : 0);
        rc = ctx->reply_cb->read(ctx->reply_user_ctx, slave, function,
                                 address, nb, &rsp[rsp_length], sizeof(rsp) - rsp_length);
        if (rc <= 0) {
            rsp_length = 0;
            goto send_response;
        }

        rsp_length += rc;
        break;
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_READ_INPUT_REGISTERS:
        rsp[rsp_length++] = nb * 2; /* number of register x 2 is the number of bytes */
        rc = ctx->reply_cb->read(ctx->reply_user_ctx, slave, function,
                                 address, nb, &rsp[rsp_length], sizeof(rsp) - rsp_length);
        if (rc <= 0) {
            rsp_length = 0;
            goto send_response;
        }

        rsp_length += rc;
        break;

    case MODBUS_FC_WRITE_MULTIPLE_COILS:
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        /* 6 = byte count (and 7 for registers */
        rc = ctx->reply_cb->write(ctx->reply_user_ctx, slave, function, address, nb, &req[offset + 6]);

        if (rc < 0) {
            rsp_length = 0;
            goto send_response;
        }

        /* 4 to copy the reg/bit address (2) and the quantity of bits/regs */
        memcpy(rsp + rsp_length, req + rsp_length, 4);
        rsp_length += 4;
        break;

    case MODBUS_FC_WRITE_SINGLE_COIL: {
        int data = (req[offset + 3] << 8) + req[offset + 4]; /* transform 0xff00/0x0000 to 0x01 in a byte */

        if (data == 0xFF00 || data == 0x0) {
            uint8_t b = data ? ON : OFF;
            rc = ctx->reply_cb->write(ctx->reply_user_ctx, slave, function, address, 1, &b);
            if (rc < 0) {
                rsp_length = 0;
                goto send_response;
            }
            memcpy(rsp, req, req_length);
            rsp_length = req_length;
        } else {
            rsp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp, FALSE,
                "Illegal data value 0x%0X in write_bit request at address %0X\n",
                data, address);
        }
    } break;

    case MODBUS_FC_MASK_WRITE_REGISTER:
    case MODBUS_FC_WRITE_SINGLE_REGISTER:
        rc = ctx->reply_cb->write(ctx->reply_user_ctx, slave, function, address, 1, &req[offset + 3]);
        if (rc < 0) {
            rsp_length = 0;
            goto send_response;
        }
        memcpy(rsp, req, req_length);
        rsp_length = req_length;
        break;

    case MODBUS_FC_WRITE_AND_READ_REGISTERS: {
        uint16_t address_write = (req[offset + 5] << 8) + req[offset + 6];
        int nb_write = (req[offset + 7] << 8) + req[offset + 8];

        rsp_length = ctx->backend->build_response_basis(&sft, rsp);
        rsp[rsp_length++] = nb << 1;

        /* Write first. 10 and 11 are the offset of the first values to write */
        rc = ctx->reply_cb->write(ctx->reply_user_ctx, slave, function, address_write, nb_write, &req[offset + 10]);
        if (rc < 0) {
            rsp_length = 0;
            goto send_response;
        }

        /* and read the data for the response */
        rc = ctx->reply_cb->read(ctx->reply_user_ctx, slave, function,
                                 address, nb, &rsp[rsp_length], sizeof(rsp) - rsp_length);
        if (rc <= 0) {
            rsp_length = 0;
            goto send_response;
        } else
            rsp_length += rc;
    } break;
    default:
        break;
    }

send_response:
    if ((ctx->backend->backend_type == _MODBUS_BACKEND_TYPE_RTU &&
         slave == MODBUS_BROADCAST_ADDRESS) ||
        rsp_length == 0) /* this indicates that the user does not want us to send response,
							probably to trigger a timeout on the other side */
        return 0;
    else
        return _modbus_send_msg(ctx, rsp, rsp_length);
}
