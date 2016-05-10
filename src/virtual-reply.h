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

typedef enum {
    MODBUS_VMAP_READ,
    MODBUS_VMAP_WRITE
} modbus_vmap_reason;

typedef struct {
    void* app;
    uint8_t*  (*tab_bits)(void* app, int addr, int nb,
                          modbus_vmap_reason what);
    void (*tab_bits_done)(void* app, uint8_t* store, int addr, int nb,
                          modbus_vmap_reason what);

    uint8_t*  (*tab_input_bits)(void* app, int addr, int nb,
                                modbus_vmap_reason what);
    void (*tab_input_bits_done)(void* app, uint8_t* store, int addr, int nb,
                                modbus_vmap_reason what);

    uint16_t* (*tab_input_registers)(void* app, int addr, int nb,
                                     modbus_vmap_reason what);
    void (*tab_input_registers_done)(void* app, uint16_t*store, int addr,
                                     int nb, modbus_vmap_reason what);


    uint16_t* (*tab_registers)(void* app, int addr, int nb,
                               modbus_vmap_reason what);
    void (*tab_registers_done)(void* app, uint16_t* store, int addr, int nb,
                               modbus_vmap_reason what);
} modbus_vmapping_t;

void modbus_virtualize_mapping(modbus_vmapping_t* dest,
                               modbus_mapping_t* source);

#endif
