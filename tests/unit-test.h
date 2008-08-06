/*
 * Copyright © 2008 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#include <stdint.h>

const uint16_t UT_COIL_STATUS_ADDRESS = 0x13;
const uint16_t UT_COIL_STATUS_NB_POINTS = 0x25;
const uint8_t UT_COIL_STATUS_TAB[] = { 0xCD, 0x6B, 0xB2, 0x0E, 0x1B }; 

const uint16_t UT_INPUT_STATUS_ADDRESS = 0xC4;
const uint16_t UT_INPUT_STATUS_NB_POINTS = 0x16;
const uint8_t UT_INPUT_STATUS_TAB[] = { 0xAC, 0xDB, 0x35 };

const uint16_t UT_HOLDING_REGISTERS_ADDRESS = 0x6B;
const uint16_t UT_HOLDING_REGISTERS_NB_POINTS = 0x3;
const uint16_t UT_HOLDING_REGISTERS_TAB[] = { 0x022B, 0x0000, 0x0064 };
/* If the following value is used, a bad response is sent.
   It's better to test with a lower value than
   UT_HOLDING_REGISTERS_NB_POINTS to try to raise a segfault. */
const uint16_t UT_HOLDING_REGISTERS_NB_POINTS_SPECIAL = 0x2;

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x08;
const uint16_t UT_INPUT_REGISTERS_NB_POINTS = 0x1;
const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x000A };

#endif /* _UNIT_TEST_H_ */
