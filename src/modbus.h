/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
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

#ifndef _MODBUS_H_
#define _MODBUS_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* Add this for macros that defined unix flavor */
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <stdint.h>
#include <sys/time.h>

#include "modbus-version.h"

#ifdef  __cplusplus
# define MODBUS_BEGIN_DECLS  extern "C" {
# define MODBUS_END_DECLS    }
#else
# define MODBUS_BEGIN_DECLS
# define MODBUS_END_DECLS
#endif

MODBUS_BEGIN_DECLS

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef OFF
#define OFF 0
#endif

#ifndef ON
#define ON 1
#endif

#define MODBUS_BROADCAST_ADDRESS    0

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 1 page 12)
 * Quantity of Coils to read (2 bytes): 1 to 2000 (0x7D0)
 * (chapter 6 section 11 page 29)
 * Quantity of Coils to write (2 bytes): 1 to 1968 (0x7B0)
 */
#define MODBUS_MAX_READ_BITS              2000
#define MODBUS_MAX_WRITE_BITS             1968

/* Modbus_Application_Protocol_V1_1b.pdf (chapter 6 section 3 page 15)
 * Quantity of Registers to read (2 bytes): 1 to 125 (0x7D)
 * (chapter 6 section 12 page 31)
 * Quantity of Registers to write (2 bytes) 1 to 123 (0x7B)
 * (chapter 6 section 17 page 38)
 * Quantity of Registers to write in R/W registers (2 bytes) 1 to 121 (0x79)
 */
#define MODBUS_MAX_READ_REGISTERS          125
#define MODBUS_MAX_WRITE_REGISTERS         123
#define MODBUS_MAX_RW_WRITE_REGISTERS      121

/* Random number to avoid errno conflicts */
#define MODBUS_ENOBASE 112345678

/* Protocol exceptions */
enum {
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,
    MODBUS_EXCEPTION_ACKNOWLEDGE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
    MODBUS_EXCEPTION_MEMORY_PARITY,
    MODBUS_EXCEPTION_NOT_DEFINED,
    MODBUS_EXCEPTION_GATEWAY_PATH,
    MODBUS_EXCEPTION_GATEWAY_TARGET,
    MODBUS_EXCEPTION_MAX
};

#define EMBXILFUN  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_FUNCTION)
#define EMBXILADD  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS)
#define EMBXILVAL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE)
#define EMBXSFAIL  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE)
#define EMBXACK    (MODBUS_ENOBASE + MODBUS_EXCEPTION_ACKNOWLEDGE)
#define EMBXSBUSY  (MODBUS_ENOBASE + MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY)
#define EMBXNACK   (MODBUS_ENOBASE + MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE)
#define EMBXMEMPAR (MODBUS_ENOBASE + MODBUS_EXCEPTION_MEMORY_PARITY)
#define EMBXGPATH  (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_PATH)
#define EMBXGTAR   (MODBUS_ENOBASE + MODBUS_EXCEPTION_GATEWAY_TARGET)

/* Native libmodbus error codes */
#define EMBBADCRC  (EMBXGTAR + 1)
#define EMBBADDATA (EMBXGTAR + 2)
#define EMBBADEXC  (EMBXGTAR + 3)
#define EMBUNKEXC  (EMBXGTAR + 4)
#define EMBMDATA   (EMBXGTAR + 5)

extern const unsigned int libmodbus_version_major;
extern const unsigned int libmodbus_version_minor;
extern const unsigned int libmodbus_version_micro;

typedef struct _modbus modbus_t;

typedef struct {
    int nb_bits;
    int nb_input_bits;
    int nb_input_registers;
    int nb_registers;
    uint8_t *tab_bits;
    uint8_t *tab_input_bits;
    uint16_t *tab_input_registers;
    uint16_t *tab_registers;
} modbus_mapping_t;

int modbus_set_slave(modbus_t* ctx, int slave);

int modbus_set_error_recovery(modbus_t *ctx, int enabled);

void modbus_get_timeout_begin(modbus_t *ctx, struct timeval *timeout);
void modbus_set_timeout_begin(modbus_t *ctx, const struct timeval *timeout);

void modbus_get_timeout_end(modbus_t *ctx, struct timeval *timeout);
void modbus_set_timeout_end(modbus_t *ctx, const struct timeval *timeout);

int modbus_connect(modbus_t *ctx);
void modbus_close(modbus_t *ctx);

void modbus_free(modbus_t *ctx);

int modbus_flush(modbus_t *ctx);
void modbus_set_debug(modbus_t *ctx, int boolean);

const char *modbus_strerror(int errnum);

int modbus_read_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
int modbus_read_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
int modbus_read_input_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
int modbus_write_bit(modbus_t *ctx, int coil_addr, int state);
int modbus_write_register(modbus_t *ctx, int reg_addr, int value);
int modbus_write_bits(modbus_t *ctx, int addr, int nb, const uint8_t *data);
int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *data);
int modbus_read_and_write_registers(modbus_t *ctx, int read_addr,
                                    int read_nb, uint16_t *dest, int write_addr,
                                    int write_nb, const uint16_t *data);
int modbus_report_slave_id(modbus_t *ctx, uint8_t *dest);

modbus_mapping_t* modbus_mapping_new(int nb_coil_status, int nb_input_status,
                                     int nb_holding_registers, int nb_input_registers);
void modbus_mapping_free(modbus_mapping_t *mb_mapping);

int modbus_receive(modbus_t *ctx, int sockfd, uint8_t *req);
int modbus_reply(modbus_t *ctx, const uint8_t *req,
                 int req_length, modbus_mapping_t *mb_mapping);

/**
 * UTILS FUNCTIONS
 **/

#define MODBUS_GET_HIGH_BYTE(data) ((data >> 8) & 0xFF)
#define MODBUS_GET_LOW_BYTE(data) (data & 0xFF)

void modbus_set_bits_from_byte(uint8_t *dest, int address, const uint8_t value);
void modbus_set_bits_from_bytes(uint8_t *dest, int address, unsigned int nb_bits,
                                const uint8_t *tab_byte);
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int address, unsigned int nb_bits);
float modbus_get_float(const uint16_t *src);
void modbus_set_float(float real, uint16_t *dest);

#include "modbus-tcp.h"
#include "modbus-rtu.h"

MODBUS_END_DECLS

#endif  /* _MODBUS_H_ */
