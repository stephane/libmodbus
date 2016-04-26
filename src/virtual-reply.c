#include "modbus.h"

#include <stdlib.h>
#include <string.h>

static uint8_t* get_discrete_inputs(void* app, int address, int nb)
{
    if(!app)
        return NULL;

    modbus_mapping_t* mb_mapping = app;
    int addr = address - mb_mapping->offset_input_bits;
    if (   (address < mb_mapping->offset_input_bits)
        || ((addr + nb) > mb_mapping->nb_input_bits)) {
        return NULL;
    }
    return mb_mapping->tab_input_bits + addr;
}


void modbus_virtualize_mapping(modbus_vmapping_t* dest,
                               modbus_mapping_t* source)
{
    memset(dest, 0, sizeof(*dest));
    dest->app = source;
    dest->tab_input_bits = get_discrete_inputs;
}
