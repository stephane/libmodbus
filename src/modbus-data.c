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
    /* Suppose the modbus byte stream contained the 32-bit float 0x10203040  - abcd.
       On big endian systems, the memory starting at src contains two 16-bit registers 0x1020  and 0x3040
       On little endian system, the 16-bit registers memory holds 0x2010 and 0x4030

       To convert the data to float32 on big-endian we only need to cast the pointer and we are done.
       On little endian systems, we need to swap the bytes in each word again and then assemble
       an integer via shift operations and finally cast to float32.

       A portable way is to retrieve low and high bytes of both words using shift operations, then assemble
       the 32-bit integer.
    */

    float f;
    uint8_t a, b, c, d;

    // access buffer memory byte-wise ABCD
    a = src[0] >> 8;     // high byte if first word
    b = src[0] & 0xFF;   // low byte if first word
    c = src[1] >> 8;     // high byte if second word
    d = src[1] & 0xFF;   // low byte if second word

    // assemble in memory location of float
    // from right to left: get address of float, interpret as address to uint32, dereference and write uint32
    *(uint32_t *)&f = (a << 24) | (b << 16) | (c << 8) | (d << 0);

    return f;
}

/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float modbus_get_float_dcba(const uint16_t *src)
{
    float f;
    uint8_t a, b, c, d;

    // access buffer memory byte-wise DCBA
    d = src[0] >> 8;     // high byte if first word
    c = src[0] & 0xFF;   // low byte if first word
    b = src[1] >> 8;     // high byte if second word
    a = src[1] & 0xFF;   // low byte if second word

    // assemble in memory location of float
    // from right to left: get address of float, interpret as address to uint32, dereference and write uint32
    *(uint32_t *)&f = (a << 24) | (b << 16) | (c << 8) | (d << 0);

    return f;
}

/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float modbus_get_float_badc(const uint16_t *src)
{
    float f;
    uint8_t a, b, c, d;

    // access buffer memory byte-wise BADC
    b = src[0] >> 8;     // high byte if first word
    a = src[0] & 0xFF;   // low byte if first word
    d = src[1] >> 8;     // high byte if second word
    c = src[1] & 0xFF;   // low byte if second word

    // assemble in memory location of float
    // from right to left: get address of float, interpret as address to uint32, dereference and write uint32
    *(uint32_t *)&f = (a << 24) | (b << 16) | (c << 8) | (d << 0);

    return f;
}

/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float modbus_get_float_cdab(const uint16_t *src)
{
    float f;
    uint8_t a, b, c, d;

    // access buffer memory byte-wise CDAB
    c = src[0] >> 8;     // high byte if first word
    d = src[0] & 0xFF;   // low byte if first word
    a = src[1] >> 8;     // high byte if second word
    b = src[1] & 0xFF;   // low byte if second word

    // assemble in memory location of float
    // from right to left: get address of float, interpret as address to uint32, dereference and write uint32
    *(uint32_t *)&f = (a << 24) | (b << 16) | (c << 8) | (d << 0);

    return f;
}

/* DEPRECATED - Get a float from 4 bytes in sort of Modbus format */
float modbus_get_float(const uint16_t *src)
{
    return modbus_get_float_dcba(src);
}

/* Set a float to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_float_abcd(float f, uint16_t *dest)
{
    uint32_t i = *(uint32_t*)(&f);
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
    uint32_t i = *(uint32_t*)(&f);
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
    uint32_t i = *(uint32_t*)(&f);
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
    uint32_t i = *(uint32_t*)(&f);
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
    return modbus_set_float_dcba(f, dest);
}
