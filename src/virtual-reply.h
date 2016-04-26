/*
 * Copyright © 2001-2013 Rüdiger Ranft <libmodbus@qzzq.de>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#ifndef MODBUS_VIRTUAL_RESPONSE_H
#define MODBUS_VIRTUAL_RESPONSE_H

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif

typedef struct {
    void* app;
    uint8_t*  (*tab_bits)(void* app, int addr, int nb);
    uint8_t*  (*tab_input_bits)(void* app, int addr, int nb);
    uint16_t* (*tab_input_registers)(void* app, int addr, int nb);
    uint16_t* (*tab_registers)(void* app, int addr, int nb);
} modbus_vmapping_t;

void modbus_virtualize_mapping(modbus_vmapping_t* dest,
                               modbus_mapping_t* source);

#endif
