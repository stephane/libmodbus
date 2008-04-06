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

const uint16_t UT_INPUT_REGISTERS_ADDRESS = 0x08;
const uint16_t UT_INPUT_REGISTERS_NB_POINTS = 0x1;
const uint16_t UT_INPUT_REGISTERS_TAB[] = { 0x000A };

#endif /* _UNIT_TEST_H_ */
