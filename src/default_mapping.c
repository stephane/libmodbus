#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "modbus.h"

#include "default_mapping.h"

/* functions working on the modbus_mapping_t structure */

static int modbus_mapping_read_coils(void* modbus_mapping_,
                                uint16_t starting_address, uint16_t quantity,
                                uint16_t* byte_count, uint8_t coils[])
{
    modbus_mapping_t* modbus_mapping = (modbus_mapping_t*)modbus_mapping_;
    if (starting_address + quantity > modbus_mapping->nb_bits)
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    int i, shift, out;
    for (i = 0; i < quantity; ++i) {
        shift = i % 8;
        out = i / 8;
        if (shift == 0)
            coils[out]= 0;
        coils[out] |= modbus_mapping->tab_bits[starting_address + i] << shift;
    }
    return 0;
}

static int modbus_mapping_read_inputs(void* modbus_mapping_,
                                uint16_t starting_address, uint16_t quantity,
                                uint16_t* byte_count, uint8_t inputs[])
{
    modbus_mapping_t* modbus_mapping = (modbus_mapping_t*)modbus_mapping_;
    if (starting_address + quantity > modbus_mapping->nb_input_bits)
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    int i, shift, out;
    for (i = 0; i < quantity; ++i) {
        shift = i % 8;
        out = i / 8;
        if (shift == 0)
            inputs[out]= 0;
        inputs[out] |= modbus_mapping->tab_input_bits[starting_address + i] << shift;
    }
    return 0;
}

static int modbus_mapping_read_holding_registers(void* modbus_mapping_,
                                uint16_t starting_address, uint16_t quantity,
                                uint16_t* byte_count, uint16_t values[])
{
    modbus_mapping_t* modbus_mapping = (modbus_mapping_t*)modbus_mapping_;
    if (starting_address + quantity > modbus_mapping->nb_registers)
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    int i;
    for (i = 0; i < quantity; ++i) {
        values[i] = modbus_mapping->tab_registers[starting_address + i];
    }
    return 0;
}

static int modbus_mapping_read_input_registers(void* modbus_mapping_,
                                uint16_t starting_address, uint16_t quantity,
                                uint16_t* byte_count, uint16_t values[])
{
    modbus_mapping_t* modbus_mapping = (modbus_mapping_t*)modbus_mapping_;
    if (starting_address + quantity > modbus_mapping->nb_input_registers)
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    int i;
    for(i = 0; i < quantity; ++i) {
        values[i] = modbus_mapping->tab_input_registers[starting_address + i];
    }
    return 0;
}

static int modbus_mapping_write_single_coil(void* modbus_mapping_,
                                uint16_t address, uint8_t on)
{
    modbus_mapping_t* modbus_mapping = (modbus_mapping_t*)modbus_mapping_;
    if (address >= (modbus_mapping->nb_bits)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    else if (on) {
        modbus_mapping->tab_bits[address] = 1;
    }
    else {
        modbus_mapping->tab_bits[address] = 0;
    }
    return 0;
}

static int modbus_mapping_write_single_register(void* modbus_mapping_,
                                uint16_t address, uint16_t value)
{
    modbus_mapping_t* modbus_mapping = (modbus_mapping_t*)modbus_mapping_;
    if (address >= modbus_mapping->nb_registers)
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    modbus_mapping->tab_registers[address] = value;
    return 0;
}

static int modbus_mapping_write_multiple_coils(void* modbus_mapping_,
                                uint16_t starting_address, uint16_t quantity,
                                const uint8_t values[])
{
    int i;
    modbus_mapping_t* modbus_mapping = (modbus_mapping_t*)modbus_mapping_;
    if (starting_address + quantity > modbus_mapping->nb_bits)
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    for (i = 0; i < quantity; ++i) {
        modbus_mapping->tab_bits[starting_address + i] = (values[i/8] & (1 << i % 8)) ? 1 : 0;
    }
    return 0;
}

static int modbus_mapping_write_multiple_registers(void* modbus_mapping_,
                                uint16_t starting_address, uint16_t quantity,
                                const uint16_t values[])
{
    int i;
    modbus_mapping_t* modbus_mapping = (modbus_mapping_t*)modbus_mapping_;
    if (starting_address + quantity > modbus_mapping->nb_registers)
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    for (i = 0; i < quantity; ++i) {
        modbus_mapping->tab_registers[starting_address + i] = values[i];
    }
    return 0;
}

/* Default vfp table */
static modbus_storage_backend_vfptable default_mapping_vfptable = {
    1,
    &modbus_mapping_read_coils,
    &modbus_mapping_read_inputs,
    &modbus_mapping_read_holding_registers,
    &modbus_mapping_read_input_registers,
    &modbus_mapping_write_single_coil,
    &modbus_mapping_write_single_register,
    &modbus_mapping_write_multiple_coils,
    &modbus_mapping_write_multiple_registers,
};


modbus_mapping_t* modbus_default_mapping_new(int nb_bits, int nb_input_bits,
                                     int nb_registers, int nb_input_registers)
{
    modbus_mapping_t *mb_mapping;

    mb_mapping = (modbus_mapping_t *)malloc(sizeof(modbus_mapping_t));
    if (mb_mapping == NULL) {
        return NULL;
    }

    /* 0X */
    mb_mapping->nb_bits = nb_bits;
    if (nb_bits == 0) {
        mb_mapping->tab_bits = NULL;
    } else {
       mb_mapping->tab_bits = (uint8_t*) malloc(nb_bits * sizeof(uint8_t));
        if (mb_mapping->tab_bits == NULL) {
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->tab_bits, 0, nb_bits * sizeof(uint8_t));
    }

    /* 1X */
    mb_mapping->nb_input_bits = nb_input_bits;
    if (nb_input_bits == 0) {
        mb_mapping->tab_input_bits = NULL;
    } else {
        mb_mapping->tab_input_bits = (uint8_t*) malloc(nb_input_bits * sizeof(uint8_t));
        if (mb_mapping->tab_input_bits == NULL) {
            free(mb_mapping->tab_bits);
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->tab_input_bits, 0, nb_input_bits * sizeof(uint8_t));
    }

    /* 4X */
    mb_mapping->nb_registers = nb_registers;
    if (nb_registers == 0) {
        mb_mapping->tab_registers = NULL;
    } else {
        mb_mapping->tab_registers = (uint16_t*)malloc(nb_registers * sizeof(uint16_t));
        if (mb_mapping->tab_registers == NULL) {
            free(mb_mapping->tab_input_bits);
            free(mb_mapping->tab_bits);
            free(mb_mapping);
            return NULL;
        }
        //memset(mb_mapping->tab_registers, 0, nb_registers * sizeof(uint16_t));
    }

    /* 3X */
    mb_mapping->nb_input_registers = nb_input_registers;
    if (nb_input_registers == 0) {
        mb_mapping->tab_input_registers = NULL;
    } else {
        mb_mapping->tab_input_registers = (uint16_t*)malloc(nb_registers * sizeof(uint16_t));
        if (mb_mapping->tab_input_registers == NULL) {
            free(mb_mapping->tab_registers);
            free(mb_mapping->tab_input_bits);
            free(mb_mapping->tab_bits);
            free(mb_mapping);
            return NULL;
        }
        memset(mb_mapping->tab_input_registers, 0,
               nb_input_registers * sizeof(uint16_t));
    }
    mb_mapping->vfptable = &default_mapping_vfptable;

    return mb_mapping;
}

void modbus_default_mapping_free(void* mb_mapping)
{
    if (mb_mapping) {
        modbus_mapping_t* mapping = (modbus_mapping_t*) mb_mapping;
        free(mapping->tab_bits);
        mapping->tab_bits = NULL;
        free(mapping->tab_input_bits);
        mapping->tab_input_bits = NULL;
        free(mapping->tab_input_registers);
        mapping->tab_input_registers = NULL;
        free(mapping->tab_registers);
        mapping->tab_registers = NULL;
        free(mb_mapping);   
    }
}

