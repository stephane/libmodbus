/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>

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

#if defined(HAVE_BYTESWAP_H)
#  include <byteswap.h>
#endif

#if defined(__APPLE__)
#  include <libkern/OSByteOrder.h>
#  define bswap_16 OSSwapInt16
#  define bswap_32 OSSwapInt32
#  define bswap_64 OSSwapInt64
#endif

#if defined(__GNUC__)
#  define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
#  if GCC_VERSION >= 430
// Since GCC >= 4.30, GCC provides __builtin_bswapXX() alternatives so we switch to them
#    undef bswap_32
#    define bswap_32 __builtin_bswap32
#  endif
#  if GCC_VERSION >= 480
#    undef bswap_16
#    define bswap_16 __builtin_bswap16
#  endif
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  define bswap_32 _byteswap_ulong
#  define bswap_16 _byteswap_ushort
#endif

#if !defined(bswap_16)
#  warning "Fallback on C functions for bswap_16"
static inline uint16_t bswap_16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}
#endif

#if !defined(bswap_32)
#  warning "Fallback on C functions for bswap_32"
static inline uint32_t bswap_32(uint32_t x)
{
    return (bswap_16(x & 0xffff) << 16) | (bswap_16(x >> 16));
}
#endif

/* Sets many bits from a single byte value (all 8 bits of the byte value are
   set) */
void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value)
{
    int i;

    for (i=0; i < 8; i++) {
        dest[idx+i] = (value & (1 << i)) ? 1 : 0;
    }
}

/* Sets many bits from a table of bytes (only the bits between idx and
   idx + nb_bits are set) */
void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits,
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
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx,
                                  unsigned int nb_bits)
{
    unsigned int i;
    uint8_t value = 0;

    if (nb_bits > 8) {
        /* Assert is ignored if NDEBUG is set */
        assert(nb_bits < 8);
        nb_bits = 8;
    }

    for (i=0; i < nb_bits; i++) {
        value |= (src[idx+i] << i);
    }

    return value;
}

/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float modbus_get_float_abcd(const uint16_t *src)
{
    float f;
    uint32_t i;
    uint8_t a, b, c, d;

    a = (src[0] >> 8) & 0xFF;
    b = (src[0] >> 0) & 0xFF;
    c = (src[1] >> 8) & 0xFF;
    d = (src[1] >> 0) & 0xFF;

    i = (a << 24) |
        (b << 16) |
        (c << 8) |
        (d << 0);
    memcpy(&f, &i, 4);

    return f;
}

/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float modbus_get_float_dcba(const uint16_t *src)
{
    float f;
    uint32_t i;
    uint8_t a, b, c, d;

    a = (src[0] >> 8) & 0xFF;
    b = (src[0] >> 0) & 0xFF;
    c = (src[1] >> 8) & 0xFF;
    d = (src[1] >> 0) & 0xFF;

    i = (d << 24) |
        (c << 16) |
        (b << 8) |
        (a << 0);
    memcpy(&f, &i, 4);

    return f;
}

/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float modbus_get_float_badc(const uint16_t *src)
{
    float f;
    uint32_t i;
    uint8_t a, b, c, d;

    a = (src[0] >> 8) & 0xFF;
    b = (src[0] >> 0) & 0xFF;
    c = (src[1] >> 8) & 0xFF;
    d = (src[1] >> 0) & 0xFF;

    i = (b << 24) |
        (a << 16) |
        (d << 8) |
        (c << 0);
    memcpy(&f, &i, 4);

    return f;
}

/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float modbus_get_float_cdab(const uint16_t *src)
{
    float f;
    uint32_t i;
    uint8_t a, b, c, d;

    a = (src[0] >> 8) & 0xFF;
    b = (src[0] >> 0) & 0xFF;
    c = (src[1] >> 8) & 0xFF;
    d = (src[1] >> 0) & 0xFF;

    i = (c << 24) |
        (d << 16) |
        (a << 8) |
        (b << 0);
    memcpy(&f, &i, 4);

    return f;
}

/* DEPRECATED - Get a float from 4 bytes in sort of Modbus format */
float modbus_get_float(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = (((uint32_t)src[1]) << 16) + src[0];
    memcpy(&f, &i, sizeof(float));

    return f;

}

/* Set a float to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_float_abcd(float f, uint16_t *dest)
{
    uint32_t i;
    uint8_t *out = (uint8_t*) dest;
    uint8_t a, b, c, d;

    memcpy(&i, &f, sizeof(uint32_t));
    a = (i >> 24) & 0xFF;
    b = (i >> 16) & 0xFF;
    c = (i >> 8) & 0xFF;
    d = (i >> 0) & 0xFF;

    out[0] = a;
    out[1] = b;
    out[2] = c;
    out[3] = d;
}

/* Set a float to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_float_dcba(float f, uint16_t *dest)
{
    uint32_t i;
    uint8_t *out = (uint8_t*) dest;
    uint8_t a, b, c, d;

    memcpy(&i, &f, sizeof(uint32_t));
    a = (i >> 24) & 0xFF;
    b = (i >> 16) & 0xFF;
    c = (i >> 8) & 0xFF;
    d = (i >> 0) & 0xFF;

    out[0] = d;
    out[1] = c;
    out[2] = b;
    out[3] = a;

}

/* Set a float to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_float_badc(float f, uint16_t *dest)
{
    uint32_t i;
    uint8_t *out = (uint8_t*) dest;
    uint8_t a, b, c, d;

    memcpy(&i, &f, sizeof(uint32_t));
    a = (i >> 24) & 0xFF;
    b = (i >> 16) & 0xFF;
    c = (i >> 8) & 0xFF;
    d = (i >> 0) & 0xFF;

    out[0] = b;
    out[1] = a;
    out[2] = d;
    out[3] = c;
}

/* Set a float to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_float_cdab(float f, uint16_t *dest)
{
    uint32_t i;
    uint8_t *out = (uint8_t*) dest;
    uint8_t a, b, c, d;

    memcpy(&i, &f, sizeof(uint32_t));
    a = (i >> 24) & 0xFF;
    b = (i >> 16) & 0xFF;
    c = (i >> 8) & 0xFF;
    d = (i >> 0) & 0xFF;

    out[0] = c;
    out[1] = d;
    out[2] = a;
    out[3] = b;
}

/* DEPRECATED - Set a float to 4 bytes in a sort of Modbus format! */
void modbus_set_float(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}

/* Get a double from 8 bytes (Modbus) without any conversion (ABCDEFGH) */
double modbus_get_double_abcdefgh(const uint16_t *src)
{
    double val;
    uint64_t i;
    uint8_t a, b, c, d, e, f, g, h;

    a = (src[0] >> 8) & 0xFF;
    b = (src[0] >> 0) & 0xFF;
    c = (src[1] >> 8) & 0xFF;
    d = (src[1] >> 0) & 0xFF;
    e = (src[2] >> 8) & 0xFF;
    f = (src[2] >> 0) & 0xFF;
    g = (src[3] >> 8) & 0xFF;
    h = (src[3] >> 0) & 0xFF;

    i = ((uint64_t)a << 56) |
        ((uint64_t)b << 48) |
        ((uint64_t)c << 40) |
        ((uint64_t)d << 32) |
        ((uint64_t)e << 24) |
        ((uint64_t)f << 16) |
        ((uint64_t)g << 8)  |
        ((uint64_t)h << 0);
    memcpy(&val, &i, 8);

    return val;
}

/* Get a double from 8 bytes (Modbus) in inversed format (HGFEDCBA) */
double modbus_get_double_hgfedcba(const uint16_t *src)
{
    double val;
    uint64_t i;
    uint8_t a, b, c, d, e, f, g, h;

    a = (src[0] >> 8) & 0xFF;
    b = (src[0] >> 0) & 0xFF;
    c = (src[1] >> 8) & 0xFF;
    d = (src[1] >> 0) & 0xFF;
    e = (src[2] >> 8) & 0xFF;
    f = (src[2] >> 0) & 0xFF;
    g = (src[3] >> 8) & 0xFF;
    h = (src[3] >> 0) & 0xFF;

    i = ((uint64_t)h << 56) |
        ((uint64_t)g << 48) |
        ((uint64_t)f << 40) |
        ((uint64_t)e << 32) |
        ((uint64_t)d << 24) |
        ((uint64_t)c << 16) |
        ((uint64_t)b << 8)  |
        ((uint64_t)a << 0);
    memcpy(&val, &i, 8);

    return val;
}

/* Get a double from 8 bytes (Modbus) in inversed format (BADCFEHG) */
double modbus_get_double_badcfehg(const uint16_t *src)
{
    double val;
    uint64_t i;
    uint8_t a, b, c, d, e, f, g, h;

    a = (src[0] >> 8) & 0xFF;
    b = (src[0] >> 0) & 0xFF;
    c = (src[1] >> 8) & 0xFF;
    d = (src[1] >> 0) & 0xFF;
    e = (src[2] >> 8) & 0xFF;
    f = (src[2] >> 0) & 0xFF;
    g = (src[3] >> 8) & 0xFF;
    h = (src[3] >> 0) & 0xFF;

    i = ((uint64_t)b << 56) |
        ((uint64_t)a << 48) |
        ((uint64_t)d << 40) |
        ((uint64_t)c << 32) |
        ((uint64_t)f << 24) |
        ((uint64_t)e << 16) |
        ((uint64_t)h << 8)  |
        ((uint64_t)g << 0);
    memcpy(&val, &i, 8);

    return val;
}

/* Get a double from 8 bytes (Modbus) in inversed format (GHEFCDAB) */
double modbus_get_double_ghefcdab(const uint16_t *src)
{
    double val;
    uint64_t i;
    uint8_t a, b, c, d, e, f, g, h;

    a = (src[0] >> 8) & 0xFF;
    b = (src[0] >> 0) & 0xFF;
    c = (src[1] >> 8) & 0xFF;
    d = (src[1] >> 0) & 0xFF;
    e = (src[2] >> 8) & 0xFF;
    f = (src[2] >> 0) & 0xFF;
    g = (src[3] >> 8) & 0xFF;
    h = (src[3] >> 0) & 0xFF;

    i = ((uint64_t)g << 56) |
        ((uint64_t)h << 48) |
        ((uint64_t)e << 40) |
        ((uint64_t)f << 32) |
        ((uint64_t)c << 24) |
        ((uint64_t)d << 16) |
        ((uint64_t)a << 8)  |
        ((uint64_t)b << 0);
    memcpy(&val, &i, 8);

    return val;
}

/* Set a double to 8 bytes for Modbus w/o any conversion (ABCDEFGH) */
void modbus_set_double_abcdefgh(double val, const uint16_t *dest)
{
    uint64_t i;
    uint8_t *out = (uint8_t*) dest;
    uint8_t a, b, c, d, e, f, g, h;

    memcpy(&i, &val, sizeof(uint64_t));
    a = (i >> 56) & 0xFF;
    b = (i >> 48) & 0xFF;
    c = (i >> 40) & 0xFF;
    d = (i >> 32) & 0xFF;
    e = (i >> 24) & 0xFF;
    f = (i >> 16) & 0xFF;
    g = (i >> 8) & 0xFF;
    h = (i >> 0) & 0xFF;

    out[0] = a;
    out[1] = b;
    out[2] = c;
    out[3] = d;
    out[4] = e;
    out[5] = f;
    out[6] = g;
    out[7] = h;
}

/* Set a double to 8 bytes for Modbus w/o any conversion (HGFEDCBA) */
void modbus_set_double_hgfedcba(double val, const uint16_t *dest)
{
    uint64_t i;
    uint8_t *out = (uint8_t*) dest;
    uint8_t a, b, c, d, e, f, g, h;

    memcpy(&i, &val, sizeof(uint64_t));
    a = (i >> 56) & 0xFF;
    b = (i >> 48) & 0xFF;
    c = (i >> 40) & 0xFF;
    d = (i >> 32) & 0xFF;
    e = (i >> 24) & 0xFF;
    f = (i >> 16) & 0xFF;
    g = (i >> 8) & 0xFF;
    h = (i >> 0) & 0xFF;

    out[0] = h;
    out[1] = g;
    out[2] = f;
    out[3] = e;
    out[4] = d;
    out[5] = c;
    out[6] = b;
    out[7] = a;
}

/* Set a double to 8 bytes for Modbus w/o any conversion (BADCFEHG) */
void modbus_set_double_badcfehg(double val, const uint16_t *dest)
{
    uint64_t i;
    uint8_t *out = (uint8_t*) dest;
    uint8_t a, b, c, d, e, f, g, h;

    memcpy(&i, &val, sizeof(uint64_t));
    a = (i >> 56) & 0xFF;
    b = (i >> 48) & 0xFF;
    c = (i >> 40) & 0xFF;
    d = (i >> 32) & 0xFF;
    e = (i >> 24) & 0xFF;
    f = (i >> 16) & 0xFF;
    g = (i >> 8) & 0xFF;
    h = (i >> 0) & 0xFF;

    out[0] = b;
    out[1] = a;
    out[2] = d;
    out[3] = c;
    out[4] = f;
    out[5] = e;
    out[6] = h;
    out[7] = g;
}

/* Set a double to 8 bytes for Modbus w/o any conversion (GHEFCDAB) */
void modbus_set_double_ghefcdab(double val, const uint16_t *dest)
{
    uint64_t i;
    uint8_t *out = (uint8_t*) dest;
    uint8_t a, b, c, d, e, f, g, h;

    memcpy(&i, &val, sizeof(uint64_t));
    a = (i >> 56) & 0xFF;
    b = (i >> 48) & 0xFF;
    c = (i >> 40) & 0xFF;
    d = (i >> 32) & 0xFF;
    e = (i >> 24) & 0xFF;
    f = (i >> 16) & 0xFF;
    g = (i >> 8) & 0xFF;
    h = (i >> 0) & 0xFF;

    out[0] = g;
    out[1] = h;
    out[2] = e;
    out[3] = f;
    out[4] = c;
    out[5] = d;
    out[6] = a;
    out[7] = b;
}

