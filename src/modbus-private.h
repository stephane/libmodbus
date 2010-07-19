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

#ifndef _MODBUS_PRIVATE_H_
#define _MODBUS_PRIVATE_H_

#include "modbus.h"

MODBUS_BEGIN_DECLS

#define HEADER_LENGTH_RTU      1
#define PRESET_REQ_LENGTH_RTU  6
#define PRESET_RSP_LENGTH_RTU  2

#define HEADER_LENGTH_TCP      7
#define PRESET_REQ_LENGTH_TCP 12
#define PRESET_RSP_LENGTH_TCP  8

#define CHECKSUM_LENGTH_RTU    2
#define CHECKSUM_LENGTH_TCP    0

/* It's not really the minimal length (the real one is report slave ID
 * in RTU (4 bytes)) but it's a convenient size to use in RTU or TCP
 * communications to read many values or write a single one.
 * Maximum between :
 * - HEADER_LENGTH_TCP (7) + function (1) + address (2) + number (2)
 * - HEADER_LENGTH_RTU (1) + function (1) + address (2) + number (2) + CRC (2)
 */
#define MIN_REQ_LENGTH           12

#define EXCEPTION_RSP_LENGTH_RTU  5

#define REPORT_SLAVE_ID_LENGTH   75

/* Time out between trames in microsecond */
#define TIME_OUT_BEGIN_OF_TRAME    500000
#define TIME_OUT_END_OF_TRAME      500000

/* Function codes */
#define FC_READ_COILS                0x01
#define FC_READ_DISCRETE_INPUTS      0x02
#define FC_READ_HOLDING_REGISTERS    0x03
#define FC_READ_INPUT_REGISTERS      0x04
#define FC_WRITE_SINGLE_COIL         0x05
#define FC_WRITE_SINGLE_REGISTER     0x06
#define FC_READ_EXCEPTION_STATUS     0x07
#define FC_WRITE_MULTIPLE_COILS      0x0F
#define FC_WRITE_MULTIPLE_REGISTERS  0x10
#define FC_REPORT_SLAVE_ID           0x11

typedef enum { RTU=0, TCP } type_com_t;

struct _modbus {
    /* Communication mode: RTU or TCP */
    type_com_t type_com;
    /* Slave address */
    int slave;
    /* Socket or file descriptor */
    int s;
    int debug;
    int error_recovery;
    struct timeval timeout_begin;
    struct timeval timeout_end;
    void *com;
};

typedef struct _modbus_rtu {
    /* Device: "/dev/ttyS0", "/dev/ttyUSB0" or "/dev/tty.USA19*" on Mac OS X for
       KeySpan USB<->Serial adapters this string had to be made bigger on OS X
       as the directory+file name was bigger than 19 bytes. Making it 67 bytes
       for now, but OS X does support 256 byte file names. May become a problem
       in the future. */
#ifdef __APPLE_CC__
    char device[64];
#else
    char device[16];
#endif
    /* Bauds: 9600, 19200, 57600, 115200, etc */
    int baud;
    /* Data bit */
    uint8_t data_bit;
    /* Stop bit */
    uint8_t stop_bit;
    /* Parity: 'N', 'O', 'E' */
    char parity;
    /* Save old termios settings */
    struct termios old_tios;
} modbus_rtu_t;

typedef struct _modbus_tcp {
    /* TCP port */
    int port;
    /* IP address */
    char ip[16];
} modbus_tcp_t;

MODBUS_END_DECLS

#endif  /* _MODBUS_PRIVATE_H_ */
