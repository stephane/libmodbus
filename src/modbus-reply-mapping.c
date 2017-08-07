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

#include <config.h>

#include "modbus-private.h"
#include "modbus.h"

static int response_io_status(uint8_t *tab_io_status,
                              int address, int nb,
                              uint8_t *rsp, int offset)
{
    int shift = 0;
    /* Instead of byte (not allowed in Win32) */
    int one_byte = 0;
    int i;

    for (i = address; i < address + nb; i++) {
        one_byte |= tab_io_status[i] << shift;
        if (shift == 7) {
            /* Byte is full */
            rsp[offset++] = one_byte;
            one_byte = shift = 0;
        } else {
            shift++;
        }
    }

    if (shift != 0)
        rsp[offset++] = one_byte;

    return offset;
}

static int mb_mapping_accept_rtu_slave(void *user_ctx, int slave)
{
    return TRUE;
}

static int mb_mapping_verify(void *user_ctx, int slave, int function, uint16_t address, int nb)
{
    modbus_mapping_t *mb_mapping = user_ctx;

    unsigned int is_input = 0;

    switch (function) {
    case MODBUS_FC_READ_DISCRETE_INPUTS:
        is_input = 1; /* fall-through */
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_WRITE_SINGLE_COIL:
    case MODBUS_FC_WRITE_MULTIPLE_COILS: {
        int start_bits = is_input ? mb_mapping->start_input_bits : mb_mapping->start_bits;
        int nb_bits = is_input ? mb_mapping->nb_input_bits : mb_mapping->nb_bits;
        int mapping_address = address - start_bits;
        if (mapping_address < 0 || (mapping_address + nb) > nb_bits)
            return EMBXILADD;
    } break;

    case MODBUS_FC_READ_INPUT_REGISTERS:
        is_input = 1; /* fall-through */
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_WRITE_SINGLE_REGISTER:
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
    case MODBUS_FC_MASK_WRITE_REGISTER:
    case MODBUS_FC_WRITE_AND_READ_REGISTERS: {
        int nb_registers = is_input ? mb_mapping->nb_input_registers : mb_mapping->nb_registers;
        int start_registers = is_input ? mb_mapping->start_input_registers : mb_mapping->start_registers;
        int mapping_address = address - start_registers;
        if (mapping_address < 0 || (mapping_address + nb) > nb_registers)
            return EMBXILADD;
    } break;
    }

    return 0;
}

static int mb_mapping_read(void *user_ctx, int slave, int function, uint16_t address, int nb, uint8_t *rsp, int max_len)
{
    modbus_mapping_t *mb_mapping = user_ctx;

    unsigned int is_input = 0;
    int length = 0, i;

    switch (function) {
    case MODBUS_FC_READ_INPUT_REGISTERS:
        is_input = 1; /* fall-through */
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_WRITE_AND_READ_REGISTERS: {
        int start_registers = is_input ? mb_mapping->start_input_registers : mb_mapping->start_registers;
        uint16_t *tab_registers = is_input ? mb_mapping->tab_input_registers : mb_mapping->tab_registers;
        int mapping_address = address - start_registers;

        for (i = mapping_address; i < mapping_address + nb; i++) {
            rsp[length++] = tab_registers[i] >> 8;
            rsp[length++] = tab_registers[i] & 0xFF;
        }
    } break;

    case MODBUS_FC_READ_DISCRETE_INPUTS:
        is_input = 1; /* fall-through */
    case MODBUS_FC_READ_COILS: {
        uint8_t *tab_bits = is_input ? mb_mapping->tab_input_bits : mb_mapping->tab_bits;
        int start_bits = is_input ? mb_mapping->start_input_bits : mb_mapping->start_bits;
        int mapping_address = address - start_bits;
        length = response_io_status(tab_bits, mapping_address, nb, rsp, 0);
    } break;

    default:
        break;
    }

    return length;
}

static int mb_mapping_write(void *user_ctx, int slave, int function, uint16_t address, int nb, const uint8_t *req)
{
    modbus_mapping_t *mb_mapping = user_ctx;

    switch (function) {
    case MODBUS_FC_WRITE_SINGLE_COIL:
    case MODBUS_FC_WRITE_MULTIPLE_COILS: {
        int mapping_address = address - mb_mapping->start_bits;
        modbus_set_bits_from_bytes(mb_mapping->tab_bits, mapping_address, nb, req);
    } break;

    case MODBUS_FC_WRITE_SINGLE_REGISTER:
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
    case MODBUS_FC_WRITE_AND_READ_REGISTERS: {
        uint16_t mapping_address = address - mb_mapping->start_registers;
        int i, j;
        for (i = mapping_address, j = 0; i < mapping_address + nb; i++, j += 2) {
            mb_mapping->tab_registers[i] = (req[j] << 8) + req[j + 1];
        }
    } break;

    case MODBUS_FC_MASK_WRITE_REGISTER: {
        uint16_t mapping_address = address - mb_mapping->start_registers;
        uint16_t data = mb_mapping->tab_registers[mapping_address];
        uint16_t and = (req[0] << 8) + req[1];
        uint16_t or = (req[2] << 8) + req[3];
        data = (data & and) | (or &(~and));
        mb_mapping->tab_registers[mapping_address] = data;
    } break;

    default:
        return -EINVAL;
    }
    return 0;
}

static const modbus_reply_callbacks_t mb_mapping_callbacks = {
    mb_mapping_accept_rtu_slave,
    mb_mapping_verify,
    mb_mapping_read,
    mb_mapping_write,
};

int modbus_reply(modbus_t *ctx, const uint8_t *req,
                 int req_length, modbus_mapping_t *mb_mapping)
{
    modbus_set_reply_callbacks(ctx, &mb_mapping_callbacks, mb_mapping);
    return modbus_reply_callback(ctx, req, req_length);
}
