/*
 * Copyright © 2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

/* Sets many bits from a single byte value (all 8 bits of the byte value are
   set) */
void modbus_set_bits_from_byte(uint8_t *dest, int address, const uint8_t value)
{
    int i;

    for (i=0; i<8; i++) {
        dest[address+i] = (value & (1 << i)) ? 1 : 0;
    }
}

/* Sets many bits from a table of bytes (only the bits between address and
   address + nb_bits are set) */
void modbus_set_bits_from_bytes(uint8_t *dest, int address, unsigned int nb_bits,
                                const uint8_t *tab_byte)
{
    int i;
    int shift = 0;

    for (i = address; i < address + nb_bits; i++) {
        dest[i] = tab_byte[(i - address) / 8] & (1 << shift) ? 1 : 0;
        /* gcc doesn't like: shift = (++shift) % 8; */
        shift++;
        shift %= 8;
    }
}

/* Gets the byte value from many bits.
   To obtain a full byte, set nb_bits to 8. */
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int address,
                                  unsigned int nb_bits)
{
    int i;
    uint8_t value = 0;

    if (nb_bits > 8) {
        /* Assert is ignored if NDEBUG is set */
        assert(nb_bits < 8);
        nb_bits = 8;
    }

    for (i=0; i < nb_bits; i++) {
        value |= (src[address+i] << i);
    }

    return value;
}

/* Get a float from 4 bytes in Modbus format */
float modbus_get_float(const uint16_t *src)
{
    float r = 0.0f;
    uint32_t i;

    i = (((uint32_t)src[1]) << 16) + src[0];
    memcpy(&r, &i, sizeof (r));
    return r;
}

/* Set a float to 4 bytes in Modbus format */
void modbus_set_float(float real, uint16_t *dest)
{
    uint32_t i = 0;

    memcpy(&i, &real, sizeof (i));
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}
