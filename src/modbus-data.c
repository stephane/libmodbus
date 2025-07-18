/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>

// clang-format off
#ifndef _MSC_VER
#  include <stdint.h>
#else
#  include "stdint.h"
#endif

#include <string.h>
#include <assert.h>

#if defined(_WIN32)
#  include <winsock2.h>
#else
#  include <arpa/inet.h>
#endif

#include <config.h>

#include "modbus.h"

// clang-format on

/* Sets many bits from a single byte value (all 8 bits of the byte value are
   set) */
void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value)
{
    int i;

    for (i = 0; i < 8; i++) {
        dest[idx + i] = (value & (1 << i)) ? 1 : 0;
    }
}

/* Sets many bits from a table of bytes (only the bits between idx and
   idx + nb_bits are set) */
void modbus_set_bits_from_bytes(uint8_t *dest,
                                int idx,
                                unsigned int nb_bits,
                                const uint8_t *tab_byte)
{
    unsigned int i;
    int shift = 0;

    for (i = idx; i < idx + nb_bits; i++) {
        dest[i] = tab_byte[(i - idx) / 8] & (1 << shift) ? 1 : 0;
        /* gcc doesn't like: shift = (++shift) % 8; */
        shift++;
        shift %= 8;
    }
}

/* Gets the byte value from many bits.
   To obtain a full byte, set nb_bits to 8. */
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx, unsigned int nb_bits)
{
    unsigned int i;
    uint8_t value = 0;

    if (nb_bits > 8) {
        /* Assert is ignored if NDEBUG is set */
        assert(nb_bits < 8);
        nb_bits = 8;
    }

    for (i = 0; i < nb_bits; i++) {
        value |= (src[idx + i] << i);
    }

    return value;
}

/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float modbus_get_float_abcd(const uint16_t *src)
{
    float f;
    uint32_t i;
    uint8_t a, b, c, d;

    // Mind: src contains 16-bit numbers in processor-endianness, hence
    //       we use shift operations and do not access memory directly
    a = (src[0] >> 8) & 0xFF; // high byte of first word
    b = (src[0] >> 0) & 0xFF; // low byte of first word
    c = (src[1] >> 8) & 0xFF; // high byte of second word
    d = (src[1] >> 0) & 0xFF; // low byte of second word

    // we assemble 32bit integer always in abcd order via shift operations
    i = (a << 24) | (b << 16) | (c << 8) | (d << 0);
    memcpy(&f, &i, 4);

    return f;
}

/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float modbus_get_float_dcba(const uint16_t *src)
{
    float f;
    uint32_t i;
    uint8_t a, b, c, d;

    // byte order is defined when reading from src: dcba
    d = (src[0] >> 8) & 0xFF;
    c = (src[0] >> 0) & 0xFF;
    b = (src[1] >> 8) & 0xFF;
    a = (src[1] >> 0) & 0xFF;

    // we assemble 32bit integer always in abcd order via shift operations
    i = (a << 24) | (b << 16) | (c << 8) | (d << 0);
    memcpy(&f, &i, 4);

    return f;
}

/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float modbus_get_float_badc(const uint16_t *src)
{
    float f;
    uint32_t i;
    uint8_t a, b, c, d;

    // byte order is defined when reading from src: badc
    b = (src[0] >> 8) & 0xFF;
    a = (src[0] >> 0) & 0xFF;
    d = (src[1] >> 8) & 0xFF;
    c = (src[1] >> 0) & 0xFF;

    // we assemble 32bit integer always in abcd order via shift operations
    i = (a << 24) | (b << 16) | (c << 8) | (d << 0);
    memcpy(&f, &i, 4);

    return f;
}

/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float modbus_get_float_cdab(const uint16_t *src)
{
    float f;
    uint32_t i;
    uint8_t a, b, c, d;

    // byte order is defined when reading from src: cdab
    c = (src[0] >> 8) & 0xFF;
    d = (src[0] >> 0) & 0xFF;
    a = (src[1] >> 8) & 0xFF;
    b = (src[1] >> 0) & 0xFF;

    // we assemble 32bit integer always in abcd order via shift operations
    i = (a << 24) | (b << 16) | (c << 8) | (d << 0);
    memcpy(&f, &i, 4);

    return f;
}

/* DEPRECATED - Get a float from 4 bytes in sort of Modbus format */
float modbus_get_float(const uint16_t *src)
{
    return modbus_get_float_cdab(src);
}

/* Set a float to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_float_abcd(float f, uint16_t *dest)
{
    // The straight-forward type conversion won't work because of type-punned pointer aliasing warning
    // uint32_t i = *(uint32_t*)(&f);
    float * fptr = &f;
    uint32_t * iptr = (uint32_t *)fptr;
    uint32_t i = *iptr;
    uint8_t a, b, c, d;

    a = (i >> 24) & 0xFF;
    b = (i >> 16) & 0xFF;
    c = (i >> 8) & 0xFF;
    d = (i >> 0) & 0xFF;

    dest[0] = (a << 8) | b;
    dest[1] = (c << 8) | d;
}

/* Set a float to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_float_dcba(float f, uint16_t *dest)
{
    float * fptr = &f;
    uint32_t * iptr = (uint32_t *)fptr;
    uint32_t i = *iptr;
    uint8_t a, b, c, d;

    a = (i >> 24) & 0xFF;
    b = (i >> 16) & 0xFF;
    c = (i >> 8) & 0xFF;
    d = (i >> 0) & 0xFF;

    dest[0] = (d << 8) | c;
    dest[1] = (b << 8) | a;
}

/* Set a float to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_float_badc(float f, uint16_t *dest)
{
    float * fptr = &f;
    uint32_t * iptr = (uint32_t *)fptr;
    uint32_t i = *iptr;
    uint8_t a, b, c, d;

    a = (i >> 24) & 0xFF;
    b = (i >> 16) & 0xFF;
    c = (i >> 8) & 0xFF;
    d = (i >> 0) & 0xFF;

    dest[0] = (b << 8) | a;
    dest[1] = (d << 8) | c;
}

/* Set a float to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_float_cdab(float f, uint16_t *dest)
{
    float * fptr = &f;
    uint32_t * iptr = (uint32_t *)fptr;
    uint32_t i = *iptr;
    uint8_t a, b, c, d;

    a = (i >> 24) & 0xFF;
    b = (i >> 16) & 0xFF;
    c = (i >> 8) & 0xFF;
    d = (i >> 0) & 0xFF;

    dest[0] = (c << 8) | d;
    dest[1] = (a << 8) | b;
}

/* DEPRECATED - Set a float to 4 bytes in a sort of Modbus format! */
void modbus_set_float(float f, uint16_t *dest)
{
    modbus_set_float_cdab(f, dest);
}
