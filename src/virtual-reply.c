#include "modbus.h"

#include <stdlib.h>
#include <string.h>

static uint8_t* get_bit_bucket(uint8_t *base, int address, int nb, int offset,
                              int count)
{
    if(!base)
        return NULL;

    const int addr = address - offset;
    if ((address < offset) || ((addr + nb) > count))
        return NULL;

    return base + addr;
}

static uint8_t* get_discrete_inputs(void* app, int address, int nb)
{
    if(!app)
        return NULL;

    modbus_mapping_t* mb_mapping = app;
    return get_bit_bucket(mb_mapping->tab_input_bits, address, nb,
                          mb_mapping->offset_input_bits,
                          mb_mapping->nb_input_bits);
}

static uint8_t* get_coils(void* app, int address, int nb)
{
    if(!app)
        return NULL;

    modbus_mapping_t* mb_mapping = app;
    return get_bit_bucket(mb_mapping->tab_bits, address, nb,
                          mb_mapping->offset_bits,
                          mb_mapping->nb_bits);
}

static uint16_t* get_specific_register(uint16_t *base, int address, int nb,
                                       int offset, int count)
{
    if(!base)
        return NULL;

    const int addr = address - offset;
    if ((address < offset) || ((addr + nb) > count))
        return NULL;

    return base + addr;
}

static uint16_t* get_register(void* app, int address, int nb)
{
    if(!app)
        return NULL;

    modbus_mapping_t* mb_mapping = app;
    return get_specific_register(mb_mapping->tab_registers, address, nb,
                                 mb_mapping->offset_registers,
                                 mb_mapping->nb_registers);
}

static uint16_t* get_input_register(void* app, int address, int nb)
{
    if(!app)
        return NULL;

    modbus_mapping_t* mb_mapping = app;
    return get_specific_register(mb_mapping->tab_input_registers, address, nb,
                                 mb_mapping->offset_input_registers,
                                 mb_mapping->nb_input_registers);
}

void modbus_virtualize_mapping(modbus_vmapping_t* dest,
                               modbus_mapping_t* source)
{
    memset(dest, 0, sizeof(*dest));
    dest->app = source;
    dest->tab_input_bits = get_discrete_inputs;
    dest->tab_bits = get_coils;
    dest->tab_registers = get_register;
    dest->tab_input_registers = get_input_register;
}
