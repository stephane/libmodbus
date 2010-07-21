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

/*
  The library is designed to send and receive data from a device that
  communicate via the Modbus protocol.

  The function names used are inspired by the Modicon Modbus Protocol
  Reference Guide which can be obtained from Schneider at
  www.schneiderautomation.com.

  Documentation:
  http://www.easysw.com/~mike/serial/serial.html
  http://copyleft.free.fr/wordpress/index.php/libmodbus/
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>

/* TCP */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#if defined(__FreeBSD__ ) && __FreeBSD__ < 5
#include <netinet/in_systm.h>
#endif
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#if !defined(UINT16_MAX)
#define UINT16_MAX 0xFFFF
#endif

#include <config.h>
#include "modbus.h"
#include "modbus-private.h"

/* Internal use */
#define MSG_LENGTH_UNDEFINED -1

/* Exported version */
const unsigned int libmodbus_version_major = LIBMODBUS_VERSION_MAJOR;
const unsigned int libmodbus_version_minor = LIBMODBUS_VERSION_MINOR;
const unsigned int libmodbus_version_micro = LIBMODBUS_VERSION_MICRO;

/* This structure reduces the number of params in functions and so
 * optimizes the speed of execution (~ 37%). */
typedef struct {
    int slave;
    int function;
    int t_id;
} sft_t;

/* Table of CRC values for high-order byte */
static uint8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static uint8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

static const int TAB_HEADER_LENGTH[2] = {
    HEADER_LENGTH_RTU,
    HEADER_LENGTH_TCP
};

static const int TAB_CHECKSUM_LENGTH[2] = {
    CHECKSUM_LENGTH_RTU,
    CHECKSUM_LENGTH_TCP
};

static const int TAB_MAX_ADU_LENGTH[2] = {
    MODBUS_MAX_ADU_LENGTH_RTU,
    MODBUS_MAX_ADU_LENGTH_TCP,
};

/* Max between RTU and TCP max adu length */
#define MAX_MESSAGE_LENGTH MODBUS_MAX_ADU_LENGTH_TCP

const char *modbus_strerror(int errnum) {
    switch (errnum) {
    case EMBXILFUN:
        return "Illegal function";
    case EMBXILADD:
        return "Illegal data address";
    case EMBXILVAL:
        return "Illegal data value";
    case EMBXSFAIL:
        return "Slave device or server failure";
    case EMBXACK:
        return "Acknowledge";
    case EMBXSBUSY:
        return "Slave device or server is busy";
    case EMBXNACK:
        return "Negative acknowledge";
    case EMBXMEMPAR:
        return "Memory parity error";
    case EMBXGPATH:
        return "Gateway path unavailable";
    case EMBXGTAR:
        return "Target device failed to respond";
    case EMBBADCRC:
        return "Invalid CRC";
    case EMBBADDATA:
        return "Invalid data";
    case EMBBADEXC:
        return "Invalid exception code";
    case EMBMDATA:
        return "Too many data";
    default:
        return strerror(errnum);
    }
}

static void error_print(modbus_t *ctx, const char *context)
{
    if (ctx->debug) {
        fprintf(stderr, "ERROR %s", modbus_strerror(errno));
        if (context != NULL) {
            fprintf(stderr, ": %s\n", context);
        } else {
            fprintf(stderr, "\n");
        }
    }
}

int modbus_flush(modbus_t *ctx)
{
    int rc;

    if (ctx->type_com == RTU) {
        rc = tcflush(ctx->s, TCIOFLUSH);
    } else {
        do {
            /* Extract the garbage from the socket */
            char devnull[MODBUS_MAX_ADU_LENGTH_TCP];
#if (!HAVE_DECL___CYGWIN__)
            rc = recv(ctx->s, devnull, MODBUS_MAX_ADU_LENGTH_TCP, MSG_DONTWAIT);
#else
            /* On Cygwin, it's a bit more complicated to not wait */
            fd_set rfds;
            struct timeval tv;

            tv.tv_sec = 0;
            tv.tv_usec = 0;
            FD_ZERO(&rfds);
            FD_SET(ctx->s, &rfds);
            rc = select(ctx->s+1, &rfds, NULL, NULL, &tv);
            if (rc == -1) {
                return -1;
            }

            rc = recv(ctx->s, devnull, MODBUS_MAX_ADU_LENGTH_TCP, 0);
#endif
            if (ctx->debug && rc != -1) {
                printf("\n%d bytes flushed\n", rc);
            }
        } while (rc > 0);
    }

    return rc;
}

/* Computes the length of the expected response */
static unsigned int compute_response_length(modbus_t *ctx, uint8_t *req)
{
    int length;
    int offset;

    offset = TAB_HEADER_LENGTH[ctx->type_com];

    switch (req[offset]) {
    case FC_READ_COILS:
    case FC_READ_DISCRETE_INPUTS: {
        /* Header + nb values (code from write_bits) */
        int nb = (req[offset + 3] << 8) | req[offset + 4];
        length = 2 + (nb / 8) + ((nb % 8) ? 1 : 0);
    }
        break;
    case FC_READ_HOLDING_REGISTERS:
    case FC_READ_INPUT_REGISTERS:
        /* Header + 2 * nb values */
        length = 2 + 2 * (req[offset + 3] << 8 |
                          req[offset + 4]);
        break;
    case FC_READ_EXCEPTION_STATUS:
        length = 3;
        break;
    case FC_REPORT_SLAVE_ID:
        /* The response is device specific (the header provides the
           length) */
        return MSG_LENGTH_UNDEFINED;
    default:
        length = 5;
    }

    return length + offset + TAB_CHECKSUM_LENGTH[ctx->type_com];
}

/* Builds a RTU request header */
static int build_request_basis_rtu(int slave, int function,
                                   int addr, int nb,
                                   uint8_t *req)
{
    req[0] = slave;
    req[1] = function;
    req[2] = addr >> 8;
    req[3] = addr & 0x00ff;
    req[4] = nb >> 8;
    req[5] = nb & 0x00ff;

    return PRESET_REQ_LENGTH_RTU;
}

/* Builds a TCP request header */
static int build_request_basis_tcp(int slave, int function,
                                   int addr, int nb,
                                   uint8_t *req)
{

    /* Extract from MODBUS Messaging on TCP/IP Implementation Guide V1.0b
       (page 23/46):
       The transaction identifier is used to associate the future response
       with the request. So, at a time, on a TCP connection, this identifier
       must be unique. */
    static uint16_t t_id = 0;

    /* Transaction ID */
    if (t_id < UINT16_MAX)
        t_id++;
    else
        t_id = 0;
    req[0] = t_id >> 8;
    req[1] = t_id & 0x00ff;

    /* Protocol Modbus */
    req[2] = 0;
    req[3] = 0;

    /* Length will be defined later by set_req_length_tcp at offsets 4
       and 5 */

    req[6] = slave;
    req[7] = function;
    req[8] = addr >> 8;
    req[9] = addr & 0x00ff;
    req[10] = nb >> 8;
    req[11] = nb & 0x00ff;

    return PRESET_REQ_LENGTH_TCP;
}

static int build_request_basis(modbus_t *ctx, int function, int addr,
                               int nb, uint8_t *req)
{
    if (ctx->type_com == RTU)
        return build_request_basis_rtu(ctx->slave, function, addr, nb, req);
    else
        return build_request_basis_tcp(ctx->slave, function, addr, nb, req);
}

/* Builds a RTU response header */
static int build_response_basis_rtu(sft_t *sft, uint8_t *rsp)
{
    rsp[0] = sft->slave;
    rsp[1] = sft->function;

    return PRESET_RSP_LENGTH_RTU;
}

/* Builds a TCP response header */
static int build_response_basis_tcp(sft_t *sft, uint8_t *rsp)
{
    /* Extract from MODBUS Messaging on TCP/IP Implementation
       Guide V1.0b (page 23/46):
       The transaction identifier is used to associate the future
       response with the request. */
    rsp[0] = sft->t_id >> 8;
    rsp[1] = sft->t_id & 0x00ff;

    /* Protocol Modbus */
    rsp[2] = 0;
    rsp[3] = 0;

    /* Length will be set later by send_msg (4 and 5) */

    rsp[6] = 0xFF;
    rsp[7] = sft->function;

    return PRESET_RSP_LENGTH_TCP;
}

static int build_response_basis(modbus_t *ctx, sft_t *sft, uint8_t *rsp)
{
    if (ctx->type_com == RTU)
        return build_response_basis_rtu(sft, rsp);
    else
        return build_response_basis_tcp(sft, rsp);
}

/* Fast CRC */
static uint16_t crc16(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
    uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned int i; /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ table_crc_hi[i];
        crc_lo = table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}

/* The check_crc16 function shall return the message length if the CRC is
   valid. Otherwise it shall return -1 and set errno to EMBADCRC. */
static int check_crc16(modbus_t *ctx, uint8_t *msg, const int msg_length)
{
    uint16_t crc_calculated;
    uint16_t crc_received;

    crc_calculated = crc16(msg, msg_length - 2);
    crc_received = (msg[msg_length - 2] << 8) | msg[msg_length - 1];

    /* Check CRC of msg */
    if (crc_calculated == crc_received) {
        return msg_length;
    } else {
        if (ctx->debug) {
            fprintf(stderr, "ERROR CRC received %0X != CRC calculated %0X\n",
                    crc_received, crc_calculated);
        }
        if (ctx->error_recovery) {
            modbus_flush(ctx);
        }
        errno = EMBBADCRC;
        return -1;
    }
}

/* Sends a req/response over a serial or a TCP communication */
static int send_msg(modbus_t *ctx, uint8_t *req, int req_length)
{
    int rc;
    uint16_t s_crc;
    int i;

    if (ctx->type_com == RTU) {
        s_crc = crc16(req, req_length);
        req[req_length++] = s_crc >> 8;
        req[req_length++] = s_crc & 0x00FF;
    } else {
        /* Substract the header length to the message length */
        int mbap_length = req_length - 6;

        req[4] = mbap_length >> 8;
        req[5] = mbap_length & 0x00FF;
    }

    if (ctx->debug) {
        for (i = 0; i < req_length; i++)
            printf("[%.2X]", req[i]);
        printf("\n");
    }

    /* In recovery mode, the write command will be issued until to be
       successful! Disabled by default. */
    do {
        if (ctx->type_com == RTU)
            rc = write(ctx->s, req, req_length);
        else
            /* MSG_NOSIGNAL
               Requests not to send SIGPIPE on errors on stream oriented
               sockets when the other end breaks the connection.  The EPIPE
               error is still returned. */
            rc = send(ctx->s, req, req_length, MSG_NOSIGNAL);

        if (rc == -1) {
            error_print(ctx, NULL);
            if (ctx->error_recovery &&
                (errno == EBADF || errno == ECONNRESET || errno == EPIPE)) {
                modbus_close(ctx);
                modbus_connect(ctx);
            }
        }
    } while (ctx->error_recovery && rc == -1);

    if (rc > 0 && rc != req_length) {
        errno = EMBBADDATA;
        return -1;
    }

    return rc;
}

/* Computes the length of the header following the function code */
static uint8_t compute_request_length_header(int function)
{
    int length;

    if (function <= FC_WRITE_SINGLE_COIL ||
        function == FC_WRITE_SINGLE_REGISTER)
        /* Read and single write */
        length = 4;
    else if (function == FC_WRITE_MULTIPLE_COILS ||
             function == FC_WRITE_MULTIPLE_REGISTERS)
        /* Multiple write */
        length = 5;
    else if (function == FC_REPORT_SLAVE_ID)
        length = 1;
    else
        length = 0;

    return length;
}

/* Computes the length of the data to write in the request */
static int compute_request_length_data(modbus_t *ctx, uint8_t *msg)
{
    int function = msg[TAB_HEADER_LENGTH[ctx->type_com]];
    int length;

    if (function == FC_WRITE_MULTIPLE_COILS ||
        function == FC_WRITE_MULTIPLE_REGISTERS)
        length = msg[TAB_HEADER_LENGTH[ctx->type_com] + 5];
    else if (function == FC_REPORT_SLAVE_ID)
        length = msg[TAB_HEADER_LENGTH[ctx->type_com] + 1];
    else
        length = 0;

    length += TAB_CHECKSUM_LENGTH[ctx->type_com];

    return length;
}

#define WAIT_DATA() {                                                   \
        while ((s_rc = select(ctx->s+1, &rfds, NULL, NULL, &tv)) == -1) { \
            if (errno == EINTR) {                                       \
                if (ctx->debug) {                                       \
                    fprintf(stderr,                                     \
                            "A non blocked signal was caught\n");       \
                }                                                       \
                /* Necessary after an error */                          \
                FD_ZERO(&rfds);                                         \
                FD_SET(ctx->s, &rfds);                                  \
            } else {                                                    \
                error_print(ctx, "select");                             \
                if (ctx->error_recovery && (errno == EBADF)) {          \
                    modbus_close(ctx);                                  \
                    modbus_connect(ctx);                                \
                    errno = EBADF;                                      \
                    return -1;                                          \
                } else {                                                \
                    return -1;                                          \
                }                                                       \
            }                                                           \
        }                                                               \
                                                                        \
        if (s_rc == 0) {                                                \
            /* Timeout */                                               \
            if (msg_length == (TAB_HEADER_LENGTH[ctx->type_com] + 2 +   \
                               TAB_CHECKSUM_LENGTH[ctx->type_com])) {   \
                /* Optimization allowed because exception response is   \
                   the smallest trame in modbus protocol (3) so always  \
                   raise a timeout error.                               \
                   Temporary error before exception analyze. */         \
                errno = EMBUNKEXC;                                      \
            } else {                                                    \
                errno = ETIMEDOUT;                                      \
                error_print(ctx, "select");                             \
            }                                                           \
            return -1;                                                  \
        }                                                               \
    }

/* Waits a response from a modbus server or a request from a modbus client.
   This function blocks if there is no replies (3 timeouts).

   The argument msg_length_computed must be set to MSG_LENGTH_UNDEFINED if
   undefined.

   The function shall return the number of received characters and the received
   message in an array of uint8_t if successful. Otherwise it shall return -1
   and errno is set to one of the values defined below:
   - ECONNRESET
   - EMBBADDATA
   - EMBUNKEXC
   - ETIMEDOUT
   - read() or recv() error codes
*/
static int receive_msg(modbus_t *ctx, int msg_length_computed, uint8_t *msg)
{
    int s_rc;
    int read_rc;
    fd_set rfds;
    struct timeval tv;
    int length_to_read;
    uint8_t *p_msg;
    enum { FUNCTION, DATA, COMPLETE };
    int state;
    int msg_length = 0;

    if (ctx->debug) {
        if (msg_length_computed == MSG_LENGTH_UNDEFINED)
            printf("Waiting for a message...\n");
        else
            printf("Waiting for a message (%d bytes)...\n",
                   msg_length_computed);
    }

    /* Add a file descriptor to the set */
    FD_ZERO(&rfds);
    FD_SET(ctx->s, &rfds);

    if (msg_length_computed == MSG_LENGTH_UNDEFINED) {
        /* Wait for a message */
        tv.tv_sec = 60;
        tv.tv_usec = 0;

        /* The message length is undefined (request receiving) so we need to
         * analyse the message step by step.  At the first step, we want to
         * reach the function code because all packets contains this
         * information. */
        state = FUNCTION;
        msg_length_computed = TAB_HEADER_LENGTH[ctx->type_com] + 1;
    } else {
        tv.tv_sec = ctx->timeout_begin.tv_sec;
        tv.tv_usec = ctx->timeout_begin.tv_usec;
        state = COMPLETE;
    }

    length_to_read = msg_length_computed;

    s_rc = 0;
    WAIT_DATA();

    p_msg = msg;
    while (s_rc) {
        if (ctx->type_com == RTU)
            read_rc = read(ctx->s, p_msg, length_to_read);
        else
            read_rc = recv(ctx->s, p_msg, length_to_read, 0);

        if (read_rc == 0) {
            errno = ECONNRESET;
            read_rc = -1;
        }

        if (read_rc == -1) {
            error_print(ctx, "read");
            if (ctx->error_recovery && (errno == ECONNRESET ||
                                        errno == ECONNREFUSED)) {
                modbus_close(ctx);
                modbus_connect(ctx);
                /* Could be removed by previous calls */
                errno = ECONNRESET;
                return -1;
            }
            return -1;
        }

        /* Sums bytes received */
        msg_length += read_rc;

        /* Display the hex code of each character received */
        if (ctx->debug) {
            int i;
            for (i=0; i < read_rc; i++)
                printf("<%.2X>", p_msg[i]);
        }

        if (msg_length < msg_length_computed) {
            /* Message incomplete */
            length_to_read = msg_length_computed - msg_length;
        } else {
            switch (state) {
            case FUNCTION:
                /* Function code position */
                length_to_read = compute_request_length_header(
                    msg[TAB_HEADER_LENGTH[ctx->type_com]]);
                msg_length_computed += length_to_read;
                /* It's useless to check the value of
                   msg_length_computed in this case (only
                   defined values are used). */
                state = DATA;
                break;
            case DATA:
                length_to_read = compute_request_length_data(ctx, msg);
                msg_length_computed += length_to_read;
                if (msg_length_computed > TAB_MAX_ADU_LENGTH[ctx->type_com]) {
                    errno = EMBBADDATA;
                    error_print(ctx, "too many data");
                    return -1;
                }
                state = COMPLETE;
                break;
            case COMPLETE:
                length_to_read = 0;
                break;
            }
        }

        /* Moves the pointer to receive other data */
        p_msg = &(p_msg[read_rc]);

        if (length_to_read > 0) {
            /* If no character at the buffer wait
               TIME_OUT_END_OF_TRAME before to generate an error. */
            tv.tv_sec = ctx->timeout_end.tv_sec;
            tv.tv_usec = ctx->timeout_end.tv_usec;

            WAIT_DATA();
        } else {
            /* All chars are received */
            s_rc = FALSE;
        }
    }

    if (ctx->debug)
        printf("\n");

    if (ctx->type_com == RTU) {
        /* Returns msg_length on success and a negative value on
           failure */
        return check_crc16(ctx, msg, msg_length);
    } else {
        /* OK */
        return msg_length;
    }
}

/* Receive the request from a modbus master, requires the socket file descriptor
   etablished with the master device in argument or -1 to use the internal one
   of modbus_t.

   The function shall return the request received and its byte length if
   successul. Otherwise, it shall return -1 and errno is set. */
int modbus_receive(modbus_t *ctx, int sockfd, uint8_t *req)
{
    if (sockfd != -1) {
        ctx->s = sockfd;
    }

    /* The length of the request to receive isn't known. */
    return receive_msg(ctx, MSG_LENGTH_UNDEFINED, req);
}

/* Receives the response and checks values (and checksum in RTU).

   The function shall return the number of values (bits or words) and the
   response if successful. Otherwise, its shall return -1 and errno is set.

   Note: all functions used to send or receive data with modbus return
   these values. */
static int receive_msg_req(modbus_t *ctx, uint8_t *req, uint8_t *rsp)
{
    int rc;
    int rsp_length_computed;
    int offset = TAB_HEADER_LENGTH[ctx->type_com];

    rsp_length_computed = compute_response_length(ctx, req);
    rc = receive_msg(ctx, rsp_length_computed, rsp);
    if (rc != -1) {
        /* GOOD RESPONSE */
        int req_nb_value;
        int rsp_nb_value;

        /* The number of values is returned if it's corresponding
         * to the request */
        switch (rsp[offset]) {
        case FC_READ_COILS:
        case FC_READ_DISCRETE_INPUTS:
            /* Read functions, 8 values in a byte (nb
             * of values in the request and byte count in
             * the response. */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            req_nb_value = (req_nb_value / 8) + ((req_nb_value % 8) ? 1 : 0);
            rsp_nb_value = rsp[offset + 1];
            break;
        case FC_READ_HOLDING_REGISTERS:
        case FC_READ_INPUT_REGISTERS:
            /* Read functions 1 value = 2 bytes */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 1] / 2);
            break;
        case FC_WRITE_MULTIPLE_COILS:
        case FC_WRITE_MULTIPLE_REGISTERS:
            /* N Write functions */
            req_nb_value = (req[offset + 3] << 8) + req[offset + 4];
            rsp_nb_value = (rsp[offset + 3] << 8) | rsp[offset + 4];
            break;
        case FC_REPORT_SLAVE_ID:
            /* Report slave ID (bytes received) */
            req_nb_value = rsp_nb_value = rc;
            break;
        default:
            /* 1 Write functions & others */
            req_nb_value = rsp_nb_value = 1;
        }

        if (req_nb_value == rsp_nb_value) {
            rc = rsp_nb_value;
        } else {
            if (ctx->debug) {
                fprintf(stderr,
                        "Quantity not corresponding to the request (%d != %d)\n",
                        rsp_nb_value, req_nb_value);
            }
            errno = EMBBADDATA;
            rc = -1;
        }
    } else if (errno == EMBUNKEXC) {
        /* EXCEPTION CODE RECEIVED */

        /* CRC must be checked here (not done in receive_msg) */
        if (ctx->type_com == RTU) {
            rc = check_crc16(ctx, rsp, EXCEPTION_RSP_LENGTH_RTU);
            if (rc == -1)
                return -1;
        }

        /* Check for exception response.
           0x80 + function is stored in the exception
           response. */
        if (0x80 + req[offset] == rsp[offset]) {
            int exception_code = rsp[offset + 1];
            if (exception_code < MODBUS_EXCEPTION_MAX) {
                errno = MODBUS_ENOBASE + exception_code;
            } else {
                errno = EMBBADEXC;
            }
            error_print(ctx, NULL);
            return -1;
        }
    }

    return rc;
}

static int response_io_status(int address, int nb,
                              uint8_t *tab_io_status,
                              uint8_t *rsp, int offset)
{
    int shift = 0;
    int byte = 0;
    int i;

    for (i = address; i < address+nb; i++) {
        byte |= tab_io_status[i] << shift;
        if (shift == 7) {
            /* Byte is full */
            rsp[offset++] = byte;
            byte = shift = 0;
        } else {
            shift++;
        }
    }

    if (shift != 0)
        rsp[offset++] = byte;

    return offset;
}

/* Build the exception response */
static int response_exception(modbus_t *ctx, sft_t *sft,
                              int exception_code, uint8_t *rsp)
{
    int rsp_length;

    sft->function = sft->function + 0x80;
    rsp_length = build_response_basis(ctx, sft, rsp);

    /* Positive exception code */
    rsp[rsp_length++] = exception_code;

    return rsp_length;
}

/* Send a response to the receive request.
   Analyses the request and constructs a response.

   If an error occurs, this function construct the response
   accordingly.
*/
int modbus_reply(modbus_t *ctx, const uint8_t *req,
                 int req_length, modbus_mapping_t *mb_mapping)
{
    int offset = TAB_HEADER_LENGTH[ctx->type_com];
    int slave = req[offset - 1];
    int function = req[offset];
    uint16_t address = (req[offset + 1] << 8) + req[offset + 2];
    uint8_t rsp[MAX_MESSAGE_LENGTH];
    int resp_length = 0;
    sft_t sft;

    /* Filter on the Modbus unit identifier (slave) in RTU mode */
    if (ctx->type_com == RTU &&
        slave != ctx->slave && slave != MODBUS_BROADCAST_ADDRESS) {
        /* Ignores the request (not for me) */
        if (ctx->debug) {
            printf("Request for slave %d ignored (not %d)\n",
                   slave, ctx->slave);
        }
        return 0;
    }

    sft.slave = slave;
    sft.function = function;
    if (ctx->type_com == TCP) {
        sft.t_id = (req[0] << 8) + req[1];
    } else {
        sft.t_id = 0;
        req_length -= CHECKSUM_LENGTH_RTU;
    }

    switch (function) {
    case FC_READ_COILS: {
        int nb = (req[offset + 3] << 8) + req[offset + 4];

        if ((address + nb) > mb_mapping->nb_bits) {
            if (ctx->debug) {
                fprintf(stderr, "Illegal data address %0X in read_bits\n",
                        address + nb);
            }
            resp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
        } else {
            resp_length = build_response_basis(ctx, &sft, rsp);
            rsp[resp_length++] = (nb / 8) + ((nb % 8) ? 1 : 0);
            resp_length = response_io_status(address, nb,
                                             mb_mapping->tab_bits,
                                             rsp, resp_length);
        }
    }
        break;
    case FC_READ_DISCRETE_INPUTS: {
        /* Similar to coil status (but too many arguments to use a
         * function) */
        int nb = (req[offset + 3] << 8) + req[offset + 4];

        if ((address + nb) > mb_mapping->nb_input_bits) {
            if (ctx->debug) {
                fprintf(stderr, "Illegal data address %0X in read_input_bits\n",
                        address + nb);
            }
            resp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
        } else {
            resp_length = build_response_basis(ctx, &sft, rsp);
            rsp[resp_length++] = (nb / 8) + ((nb % 8) ? 1 : 0);
            resp_length = response_io_status(address, nb,
                                             mb_mapping->tab_input_bits,
                                             rsp, resp_length);
        }
    }
        break;
    case FC_READ_HOLDING_REGISTERS: {
        int nb = (req[offset + 3] << 8) + req[offset + 4];

        if ((address + nb) > mb_mapping->nb_registers) {
            if (ctx->debug) {
                fprintf(stderr, "Illegal data address %0X in read_registers\n",
                        address + nb);
            }
            resp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
        } else {
            int i;

            resp_length = build_response_basis(ctx, &sft, rsp);
            rsp[resp_length++] = nb << 1;
            for (i = address; i < address + nb; i++) {
                rsp[resp_length++] = mb_mapping->tab_registers[i] >> 8;
                rsp[resp_length++] = mb_mapping->tab_registers[i] & 0xFF;
            }
        }
    }
        break;
    case FC_READ_INPUT_REGISTERS: {
        /* Similar to holding registers (but too many arguments to use a
         * function) */
        int nb = (req[offset + 3] << 8) + req[offset + 4];

        if ((address + nb) > mb_mapping->nb_input_registers) {
            if (ctx->debug) {
                fprintf(stderr, "Illegal data address %0X in read_input_registers\n",
                        address + nb);
            }
            resp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
        } else {
            int i;

            resp_length = build_response_basis(ctx, &sft, rsp);
            rsp[resp_length++] = nb << 1;
            for (i = address; i < address + nb; i++) {
                rsp[resp_length++] = mb_mapping->tab_input_registers[i] >> 8;
                rsp[resp_length++] = mb_mapping->tab_input_registers[i] & 0xFF;
            }
        }
    }
        break;
    case FC_WRITE_SINGLE_COIL:
        if (address >= mb_mapping->nb_bits) {
            if (ctx->debug) {
                fprintf(stderr, "Illegal data address %0X in write_bit\n",
                        address);
            }
            resp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
        } else {
            int data = (req[offset + 3] << 8) + req[offset + 4];

            if (data == 0xFF00 || data == 0x0) {
                mb_mapping->tab_bits[address] = (data) ? ON : OFF;

                /* In RTU mode, the CRC is computed and added
                   to the request by send_msg, the computed
                   CRC will be same and optimisation is
                   possible here (FIXME). */
                memcpy(rsp, req, req_length);
                resp_length = req_length;
            } else {
                if (ctx->debug) {
                    fprintf(stderr,
                            "Illegal data value %0X in write_bit request at address %0X\n",
                            data, address);
                }
                resp_length = response_exception(
                    ctx, &sft,
                    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE, rsp);
            }
        }
        break;
    case FC_WRITE_SINGLE_REGISTER:
        if (address >= mb_mapping->nb_registers) {
            if (ctx->debug) {
                fprintf(stderr, "Illegal data address %0X in write_register\n",
                        address);
            }
            resp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
        } else {
            int data = (req[offset + 3] << 8) + req[offset + 4];

            mb_mapping->tab_registers[address] = data;
            memcpy(rsp, req, req_length);
            resp_length = req_length;
        }
        break;
    case FC_WRITE_MULTIPLE_COILS: {
        int nb = (req[offset + 3] << 8) + req[offset + 4];

        if ((address + nb) > mb_mapping->nb_bits) {
            if (ctx->debug) {
                fprintf(stderr, "Illegal data address %0X in write_bits\n",
                        address + nb);
            }
            resp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
        } else {
            /* 6 = byte count */
            modbus_set_bits_from_bytes(mb_mapping->tab_bits, address, nb, &req[offset + 6]);

            resp_length = build_response_basis(ctx, &sft, rsp);
            /* 4 to copy the bit address (2) and the quantity of bits */
            memcpy(rsp + resp_length, req + resp_length, 4);
            resp_length += 4;
        }
    }
        break;
    case FC_WRITE_MULTIPLE_REGISTERS: {
        int nb = (req[offset + 3] << 8) + req[offset + 4];

        if ((address + nb) > mb_mapping->nb_registers) {
            if (ctx->debug) {
                fprintf(stderr, "Illegal data address %0X in write_registers\n",
                        address + nb);
            }
            resp_length = response_exception(
                ctx, &sft,
                MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS, rsp);
        } else {
            int i, j;
            for (i = address, j = 6; i < address + nb; i++, j += 2) {
                /* 6 and 7 = first value */
                mb_mapping->tab_registers[i] =
                    (req[offset + j] << 8) + req[offset + j + 1];
            }

            resp_length = build_response_basis(ctx, &sft, rsp);
            /* 4 to copy the address (2) and the no. of registers */
            memcpy(rsp + resp_length, req + resp_length, 4);
            resp_length += 4;
        }
    }
        break;
    case FC_READ_EXCEPTION_STATUS:
    case FC_REPORT_SLAVE_ID:
        if (ctx->debug) {
            fprintf(stderr, "FIXME Not implemented\n");
        }
        errno = ENOPROTOOPT;
        return -1;
        break;
    default:
        /* FIXME Invalid function exception */
        break;
    }

    return send_msg(ctx, rsp, resp_length);
}

/* Reads IO status */
static int read_io_status(modbus_t *ctx, int function,
                          int addr, int nb, uint8_t *data_dest)
{
    int rc;
    int req_length;

    uint8_t req[MIN_REQ_LENGTH];
    uint8_t rsp[MAX_MESSAGE_LENGTH];

    req_length = build_request_basis(ctx, function, addr, nb, req);

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        int i, temp, bit;
        int pos = 0;
        int offset;
        int offset_end;

        rc = receive_msg_req(ctx, req, rsp);
        if (rc == -1)
            return -1;

        offset = TAB_HEADER_LENGTH[ctx->type_com];
        offset_end = offset + rc;
        for (i = offset; i < offset_end; i++) {
            /* Shift reg hi_byte to temp */
            temp = rsp[i + 2];

            for (bit = 0x01; (bit & 0xff) && (pos < nb);) {
                data_dest[pos++] = (temp & bit) ? TRUE : FALSE;
                bit = bit << 1;
            }

        }
    }

    return rc;
}

/* Reads the boolean status of bits and sets the array elements
   in the destination to TRUE or FALSE (single bits). */
int modbus_read_bits(modbus_t *ctx, int addr, int nb, uint8_t *data_dest)
{
    int rc;

    if (nb > MODBUS_MAX_BITS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "ERROR Too many bits requested (%d > %d)\n",
                    nb, MODBUS_MAX_BITS);
        }
        errno = EMBMDATA;
        return -1;
    }

    rc = read_io_status(ctx, FC_READ_COILS, addr, nb, data_dest);

    if (rc == -1)
        return -1;
    else
        return nb;
}


/* Same as modbus_read_bits but reads the remote device input table */
int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *data_dest)
{
    int rc;

    if (nb > MODBUS_MAX_BITS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "ERROR Too many discrete inputs requested (%d > %d)\n",
                    nb, MODBUS_MAX_BITS);
        }
        errno = EMBMDATA;
        return -1;
    }

    rc = read_io_status(ctx, FC_READ_DISCRETE_INPUTS, addr, nb, data_dest);

    if (rc == -1)
        return -1;
    else
        return nb;
}

/* Reads the data from a remove device and put that data into an array */
static int read_registers(modbus_t *ctx, int function, int addr, int nb,
                          uint16_t *data_dest)
{
    int rc;
    int req_length;
    uint8_t req[MIN_REQ_LENGTH];
    uint8_t rsp[MAX_MESSAGE_LENGTH];

    if (nb > MODBUS_MAX_REGISTERS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "ERROR Too many registers requested (%d > %d)\n",
                    nb, MODBUS_MAX_REGISTERS);
        }
        errno = EMBMDATA;
        return -1;
    }

    req_length = build_request_basis(ctx, function, addr, nb, req);

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        int offset;
        int i;

        rc = receive_msg_req(ctx, req, rsp);

        offset = TAB_HEADER_LENGTH[ctx->type_com];

        /* If rc is negative, the loop is jumped ! */
        for (i = 0; i < rc; i++) {
            /* shift reg hi_byte to temp OR with lo_byte */
            data_dest[i] = (rsp[offset + 2 + (i << 1)] << 8) |
                rsp[offset + 3 + (i << 1)];
        }
    }

    return rc;
}

/* Reads the holding registers of remote device and put the data into an
   array */
int modbus_read_registers(modbus_t *ctx, int addr, int nb, uint16_t *data_dest)
{
    int status;

    if (nb > MODBUS_MAX_REGISTERS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "ERROR Too many registers requested (%d > %d)\n",
                    nb, MODBUS_MAX_REGISTERS);
        }
        errno = EMBMDATA;
        return -1;
    }

    status = read_registers(ctx, FC_READ_HOLDING_REGISTERS,
                            addr, nb, data_dest);
    return status;
}

/* Reads the input registers of remote device and put the data into an array */
int modbus_read_input_registers(modbus_t *ctx, int addr, int nb,
                                uint16_t *data_dest)
{
    int status;

    if (nb > MODBUS_MAX_REGISTERS) {
        fprintf(stderr,
                "ERROR Too many input registers requested (%d > %d)\n",
                nb, MODBUS_MAX_REGISTERS);
        errno = EMBMDATA;
        return -1;
    }

    status = read_registers(ctx, FC_READ_INPUT_REGISTERS,
                            addr, nb, data_dest);

    return status;
}

/* Write a value to the specified register of the remote device.
   Used by write_bit and write_register */
static int write_single(modbus_t *ctx, int function, int addr, int value)
{
    int rc;
    int req_length;
    uint8_t req[MIN_REQ_LENGTH];

    req_length = build_request_basis(ctx, function, addr, value, req);

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        /* Used by write_bit and write_register */
        uint8_t rsp[MIN_REQ_LENGTH];
        rc = receive_msg_req(ctx, req, rsp);
    }

    return rc;
}

/* Turns ON or OFF a single bit of the remote device */
int modbus_write_bit(modbus_t *ctx, int addr, int state)
{
    int status;

    if (state)
        state = 0xFF00;

    status = write_single(ctx, FC_WRITE_SINGLE_COIL, addr, state);

    return status;
}

/* Writes a value in one register of the remote device */
int modbus_write_register(modbus_t *ctx, int addr, int value)
{
    int status;

    status = write_single(ctx, FC_WRITE_SINGLE_REGISTER, addr, value);

    return status;
}

/* Write the bits of the array in the remote device */
int modbus_write_bits(modbus_t *ctx, int addr, int nb, const uint8_t *data_src)
{
    int rc;
    int i;
    int byte_count;
    int req_length;
    int bit_check = 0;
    int pos = 0;

    uint8_t req[MAX_MESSAGE_LENGTH];

    if (nb > MODBUS_MAX_BITS) {
        if (ctx->debug) {
            fprintf(stderr, "ERROR Writing too many bits (%d > %d)\n",
                    nb, MODBUS_MAX_BITS);
        }
        errno = EMBMDATA;
        return -1;
    }

    req_length = build_request_basis(ctx, FC_WRITE_MULTIPLE_COILS,
                                     addr, nb, req);
    byte_count = (nb / 8) + ((nb % 8) ? 1 : 0);
    req[req_length++] = byte_count;

    for (i = 0; i < byte_count; i++) {
        int bit;

        bit = 0x01;
        req[req_length] = 0;

        while ((bit & 0xFF) && (bit_check++ < nb)) {
            if (data_src[pos++])
                req[req_length] |= bit;
            else
                req[req_length] &=~ bit;

            bit = bit << 1;
        }
        req_length++;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        uint8_t rsp[MAX_MESSAGE_LENGTH];
        rc = receive_msg_req(ctx, req, rsp);
    }


    return rc;
}

/* Write the values from the array to the registers of the remote device */
int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *data_src)
{
    int rc;
    int i;
    int req_length;
    int byte_count;

    uint8_t req[MAX_MESSAGE_LENGTH];

    if (nb > MODBUS_MAX_REGISTERS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "ERROR Trying to write to too many registers (%d > %d)\n",
                    nb, MODBUS_MAX_REGISTERS);
        }
        errno = EMBMDATA;
        return -1;
    }

    req_length = build_request_basis(ctx, FC_WRITE_MULTIPLE_REGISTERS,
                                     addr, nb, req);
    byte_count = nb * 2;
    req[req_length++] = byte_count;

    for (i = 0; i < nb; i++) {
        req[req_length++] = data_src[i] >> 8;
        req[req_length++] = data_src[i] & 0x00FF;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        uint8_t rsp[MAX_MESSAGE_LENGTH];
        rc = receive_msg_req(ctx, req, rsp);
    }

    return rc;
}

/* Send a request to get the slave ID of the device (only available in serial
 * communication) */
int modbus_report_slave_id(modbus_t *ctx, uint8_t *data_dest)
{
    int rc;
    int req_length;
    uint8_t req[MIN_REQ_LENGTH];

    if (ctx->type_com != RTU) {
        /* Only for serial communications */
        errno = EINVAL;
        return -1;
    }

    req_length = build_request_basis(ctx, FC_REPORT_SLAVE_ID, 0, 0, req);

    /* HACKISH, addr and count are not used */
    req_length -= 4;

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        int i;
        int offset;
        int offset_end;
        uint8_t rsp[MAX_MESSAGE_LENGTH];

        /* Byte count, slave id, run indicator status,
           additional data */
        rc = receive_msg_req(ctx, req, rsp);
        if (rc == -1)
            return -1;

        offset = TAB_HEADER_LENGTH[ctx->type_com] - 1;
        offset_end = offset + rc;

        for (i = offset; i < offset_end; i++)
            data_dest[i] = rsp[i];
    }

    return rc;
}

static void init_common(modbus_t *ctx)
{
    ctx->timeout_begin.tv_sec = 0;
    ctx->timeout_begin.tv_usec = TIME_OUT_BEGIN_OF_TRAME;

    ctx->timeout_end.tv_sec = 0;
    ctx->timeout_end.tv_usec = TIME_OUT_END_OF_TRAME;

    ctx->error_recovery = FALSE;
    ctx->debug = FALSE;
}

/* Allocate and initialize the modbus_t structure for RTU
   - device: "/dev/ttyS0"
   - baud:   9600, 19200, 57600, 115200, etc
   - parity: 'N' stands for None, 'E' for Even and 'O' for odd
   - data_bits: 5, 6, 7, 8
   - stop_bits: 1, 2
   - slave: slave number of the caller
*/
modbus_t* modbus_new_rtu(const char *device,
                         int baud, char parity, int data_bit,
                         int stop_bit, int slave)
{
    modbus_t *ctx;
    modbus_rtu_t *ctx_rtu;

    ctx = (modbus_t *) malloc(sizeof(modbus_t));
    init_common(ctx);
    if (modbus_set_slave(ctx, slave) == -1) {
        return NULL;
    }

    ctx->type_com = RTU;

    ctx->com = (modbus_rtu_t *) malloc(sizeof(modbus_rtu_t));
    ctx_rtu = (modbus_rtu_t *)ctx->com;

    strcpy(ctx_rtu->device, device);
    ctx_rtu->baud = baud;
    if (parity == 'N' || parity == 'E' || parity == 'O') {
        ctx_rtu->parity = parity;
    } else {
        errno = EINVAL;
        return NULL;
    }
    ctx_rtu->data_bit = data_bit;
    ctx_rtu->stop_bit = stop_bit;

    return ctx;
}

/* Allocates and initializes the modbus_t structure for TCP.
   - ip : "192.168.0.5"
   - port : 1099

   Set the port to MODBUS_TCP_DEFAULT_PORT to use the default one
   (502). It's convenient to use a port number greater than or equal
   to 1024 because it's not necessary to be root to use this port
   number.
*/
modbus_t* modbus_new_tcp(const char *ip, int port)
{
    modbus_t *ctx;
    modbus_tcp_t *ctx_tcp;

    ctx = (modbus_t *) malloc(sizeof(modbus_t));
    init_common(ctx);
    ctx->slave = MODBUS_TCP_SLAVE;

    ctx->type_com = TCP;

    ctx->com = (modbus_tcp_t *) malloc(sizeof(modbus_tcp_t));
    ctx_tcp = (modbus_tcp_t *)ctx->com;

    strncpy(ctx_tcp->ip, ip, sizeof(char)*16);
    ctx_tcp->port = port;

    /* Can be changed after to reach remote serial Modbus device */
    return ctx;
}

/* Define the slave number, the special value MODBUS_TCP_SLAVE (0xFF) can be
 * used in TCP mode to restore the default value. */
int modbus_set_slave(modbus_t *ctx, int slave)
{
    if (slave >= 1 && slave <= 247) {
        ctx->slave = slave;
    } else if (ctx->type_com == TCP && slave == MODBUS_TCP_SLAVE) {
        ctx->slave = slave;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

/*
  When disabled (default), it is expected that the application will check for
  error returns and deal with them as necessary.

  It's not recommanded to enable error recovery for slave/server.

  When enabled, the library will attempt an immediate reconnection which may
  hang for several seconds if the network to the remote target unit is down.
  The write will try a infinite close/connect loop until to be successful and
  the select/read calls will just try to retablish the connection one time then
  will return an error (if the connecton was down, the values to read are
  certainly not available anymore after reconnection, except for slave/server).
*/
int modbus_set_error_recovery(modbus_t *ctx, int enabled)
{
    if (enabled == TRUE || enabled == FALSE) {
        ctx->error_recovery = (uint8_t) enabled;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

/* Get the timeout of begin of trame */
void modbus_get_timeout_begin(modbus_t *ctx, struct timeval *timeout)
{
    *timeout = ctx->timeout_begin;
}

/* Set timeout when waiting the beginning of a trame */
void modbus_set_timeout_begin(modbus_t *ctx, const struct timeval *timeout)
{
    ctx->timeout_begin = *timeout;
}

/* Get the timeout of end of trame */
void modbus_get_timeout_end(modbus_t *ctx, struct timeval *timeout)
{
    *timeout = ctx->timeout_end;
}

/* Set timeout when waiting the end of a trame */
void modbus_set_timeout_end(modbus_t *ctx, const struct timeval *timeout)
{
    ctx->timeout_end = *timeout;
}

/* Sets up a serial port for RTU communications */
static int modbus_connect_rtu(modbus_t *ctx)
{
    struct termios tios;
    speed_t speed;
    modbus_rtu_t *ctx_rtu = ctx->com;

    if (ctx->debug) {
        printf("Opening %s at %d bauds (%c, %d, %d)\n",
               ctx_rtu->device, ctx_rtu->baud, ctx_rtu->parity,
               ctx_rtu->data_bit, ctx_rtu->stop_bit);
    }

    /* The O_NOCTTY flag tells UNIX that this program doesn't want
       to be the "controlling terminal" for that port. If you
       don't specify this then any input (such as keyboard abort
       signals and so forth) will affect your process

       Timeouts are ignored in canonical input mode or when the
       NDELAY option is set on the file via open or fcntl */
    ctx->s = open(ctx_rtu->device, O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL);
    if (ctx->s == -1) {
        fprintf(stderr, "ERROR Can't open the device %s (%s)\n",
                ctx_rtu->device, strerror(errno));
        return -1;
    }

    /* Save */
    tcgetattr(ctx->s, &(ctx_rtu->old_tios));

    memset(&tios, 0, sizeof(struct termios));

    /* C_ISPEED     Input baud (new interface)
       C_OSPEED     Output baud (new interface)
    */
    switch (ctx_rtu->baud) {
    case 110:
        speed = B110;
        break;
    case 300:
        speed = B300;
        break;
    case 600:
        speed = B600;
        break;
    case 1200:
        speed = B1200;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 57600:
        speed = B57600;
        break;
    case 115200:
        speed = B115200;
        break;
    default:
        speed = B9600;
        if (ctx->debug) {
            fprintf(stderr,
                    "WARNING Unknown baud rate %d for %s (B9600 used)\n",
                    ctx_rtu->baud, ctx_rtu->device);
        }
    }

    /* Set the baud rate */
    if ((cfsetispeed(&tios, speed) < 0) ||
        (cfsetospeed(&tios, speed) < 0)) {
        return -1;
    }

    /* C_CFLAG      Control options
       CLOCAL       Local line - do not change "owner" of port
       CREAD        Enable receiver
    */
    tios.c_cflag |= (CREAD | CLOCAL);
    /* CSIZE, HUPCL, CRTSCTS (hardware flow control) */

    /* Set data bits (5, 6, 7, 8 bits)
       CSIZE        Bit mask for data bits
    */
    tios.c_cflag &= ~CSIZE;
    switch (ctx_rtu->data_bit) {
    case 5:
        tios.c_cflag |= CS5;
        break;
    case 6:
        tios.c_cflag |= CS6;
        break;
    case 7:
        tios.c_cflag |= CS7;
        break;
    case 8:
    default:
        tios.c_cflag |= CS8;
        break;
    }

    /* Stop bit (1 or 2) */
    if (ctx_rtu->stop_bit == 1)
        tios.c_cflag &=~ CSTOPB;
    else /* 2 */
        tios.c_cflag |= CSTOPB;

    /* PARENB       Enable parity bit
       PARODD       Use odd parity instead of even */
    if (ctx_rtu->parity == 'N') {
        /* None */
        tios.c_cflag &=~ PARENB;
    } else if (ctx_rtu->parity == 'E') {
        /* Even */
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
    } else {
        /* Odd */
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
    }

    /* Read the man page of termios if you need more information. */

    /* This field isn't used on POSIX systems
       tios.c_line = 0;
    */

    /* C_LFLAG      Line options

       ISIG Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
       ICANON       Enable canonical input (else raw)
       XCASE        Map uppercase \lowercase (obsolete)
       ECHO Enable echoing of input characters
       ECHOE        Echo erase character as BS-SP-BS
       ECHOK        Echo NL after kill character
       ECHONL       Echo NL
       NOFLSH       Disable flushing of input buffers after
       interrupt or quit characters
       IEXTEN       Enable extended functions
       ECHOCTL      Echo control characters as ^char and delete as ~?
       ECHOPRT      Echo erased character as character erased
       ECHOKE       BS-SP-BS entire line on line kill
       FLUSHO       Output being flushed
       PENDIN       Retype pending input at next read or input char
       TOSTOP       Send SIGTTOU for background output

       Canonical input is line-oriented. Input characters are put
       into a buffer which can be edited interactively by the user
       until a CR (carriage return) or LF (line feed) character is
       received.

       Raw input is unprocessed. Input characters are passed
       through exactly as they are received, when they are
       received. Generally you'll deselect the ICANON, ECHO,
       ECHOE, and ISIG options when using raw input
    */

    /* Raw input */
    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* C_IFLAG      Input options

       Constant     Description
       INPCK        Enable parity check
       IGNPAR       Ignore parity errors
       PARMRK       Mark parity errors
       ISTRIP       Strip parity bits
       IXON Enable software flow control (outgoing)
       IXOFF        Enable software flow control (incoming)
       IXANY        Allow any character to start flow again
       IGNBRK       Ignore break condition
       BRKINT       Send a SIGINT when a break condition is detected
       INLCR        Map NL to CR
       IGNCR        Ignore CR
       ICRNL        Map CR to NL
       IUCLC        Map uppercase to lowercase
       IMAXBEL      Echo BEL on input line too long
    */
    if (ctx_rtu->parity == 'N') {
        /* None */
        tios.c_iflag &= ~INPCK;
    } else {
        tios.c_iflag |= INPCK;
    }

    /* Software flow control is disabled */
    tios.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* C_OFLAG      Output options
       OPOST        Postprocess output (not set = raw output)
       ONLCR        Map NL to CR-NL

       ONCLR ant others needs OPOST to be enabled
    */

    /* Raw ouput */
    tios.c_oflag &=~ OPOST;

    /* C_CC         Control characters
       VMIN         Minimum number of characters to read
       VTIME        Time to wait for data (tenths of seconds)

       UNIX serial interface drivers provide the ability to
       specify character and packet timeouts. Two elements of the
       c_cc array are used for timeouts: VMIN and VTIME. Timeouts
       are ignored in canonical input mode or when the NDELAY
       option is set on the file via open or fcntl.

       VMIN specifies the minimum number of characters to read. If
       it is set to 0, then the VTIME value specifies the time to
       wait for every character read. Note that this does not mean
       that a read call for N bytes will wait for N characters to
       come in. Rather, the timeout will apply to the first
       character and the read call will return the number of
       characters immediately available (up to the number you
       request).

       If VMIN is non-zero, VTIME specifies the time to wait for
       the first character read. If a character is read within the
       time given, any read will block (wait) until all VMIN
       characters are read. That is, once the first character is
       read, the serial interface driver expects to receive an
       entire packet of characters (VMIN bytes total). If no
       character is read within the time allowed, then the call to
       read returns 0. This method allows you to tell the serial
       driver you need exactly N bytes and any read call will
       return 0 or N bytes. However, the timeout only applies to
       the first character read, so if for some reason the driver
       misses one character inside the N byte packet then the read
       call could block forever waiting for additional input
       characters.

       VTIME specifies the amount of time to wait for incoming
       characters in tenths of seconds. If VTIME is set to 0 (the
       default), reads will block (wait) indefinitely unless the
       NDELAY option is set on the port with open or fcntl.
    */
    /* Unused because we use open with the NDELAY option */
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(ctx->s, TCSANOW, &tios) < 0) {
        return -1;
    }

    return 0;
}

/* Establishes a modbus TCP connection with a Modbus server. */
static int modbus_connect_tcp(modbus_t *ctx)
{
    int rc;
    int option;
    struct sockaddr_in addr;
    modbus_tcp_t *ctx_tcp = ctx->com;

    ctx->s = socket(PF_INET, SOCK_STREAM, 0);
    if (ctx->s == -1) {
        return -1;
    }

    /* Set the TCP no delay flag */
    /* SOL_TCP = IPPROTO_TCP */
    option = 1;
    rc = setsockopt(ctx->s, IPPROTO_TCP, TCP_NODELAY,
                    (const void *)&option, sizeof(int));
    if (rc == -1) {
        close(ctx->s);
        return -1;
    }

#if (!HAVE_DECL___CYGWIN__)
    /**
     * Cygwin defines IPTOS_LOWDELAY but can't handle that flag so it's
     * necessary to workaround that problem.
     **/
    /* Set the IP low delay option */
    option = IPTOS_LOWDELAY;
    rc = setsockopt(ctx->s, IPPROTO_IP, IP_TOS,
                    (const void *)&option, sizeof(int));
    if (rc == -1) {
        close(ctx->s);
        return -1;
    }
#endif

    if (ctx->debug) {
        printf("Connecting to %s\n", ctx_tcp->ip);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ctx_tcp->port);
    addr.sin_addr.s_addr = inet_addr(ctx_tcp->ip);
    rc = connect(ctx->s, (struct sockaddr *)&addr,
                 sizeof(struct sockaddr_in));
    if (rc == -1) {
        close(ctx->s);
        return -1;
    }

    return 0;
}

/* Establishes a modbus connection.
   Returns 0 on success or -1 on failure. */
int modbus_connect(modbus_t *ctx)
{
    int rc;

    if (ctx->type_com == RTU)
        rc = modbus_connect_rtu(ctx);
    else
        rc = modbus_connect_tcp(ctx);

    return rc;
}

/* Closes the file descriptor in RTU mode */
static void modbus_close_rtu(modbus_t *ctx)
{
    modbus_rtu_t *ctx_rtu = ctx->com;

    tcsetattr(ctx->s, TCSANOW, &(ctx_rtu->old_tios));
    close(ctx->s);
}

/* Closes the network connection and socket in TCP mode */
static void modbus_close_tcp(modbus_t *ctx)
{
    shutdown(ctx->s, SHUT_RDWR);
    close(ctx->s);
}

/* Closes a  connection */
void modbus_close(modbus_t *ctx)
{
    if (ctx == NULL)
        return;

    if (ctx->type_com == RTU)
        modbus_close_rtu(ctx);
    else
        modbus_close_tcp(ctx);
}

/* Free an initialized modbus_t */
void modbus_free(modbus_t *ctx)
{
    if (ctx == NULL)
        return;

    free(ctx->com);
    free(ctx);
}

/* Activates the debug messages */
void modbus_set_debug(modbus_t *ctx, int boolean)
{
    ctx->debug = boolean;
}

/* Allocates 4 arrays to store bits, input bits, registers and inputs
   registers. The pointers are stored in modbus_mapping structure.

   The modbus_mapping_new() function shall return the new allocated structure if
   successful. Otherwise it shall return NULL and set errno to ENOMEM. */
modbus_mapping_t* modbus_mapping_new(int nb_bits, int nb_input_bits,
                                     int nb_registers, int nb_input_registers)
{
    modbus_mapping_t *mb_mapping;

    mb_mapping = (modbus_mapping_t *)malloc(sizeof(modbus_mapping_t));
    if (mb_mapping == NULL) {
        return NULL;
    }

    /* 0X */
    mb_mapping->nb_bits = nb_bits;
    mb_mapping->tab_bits =
        (uint8_t *) malloc(nb_bits * sizeof(uint8_t));
    if (mb_mapping->tab_bits == NULL) {
        free(mb_mapping);
        return NULL;
    }
    memset(mb_mapping->tab_bits, 0,
           nb_bits * sizeof(uint8_t));

    /* 1X */
    mb_mapping->nb_input_bits = nb_input_bits;
    mb_mapping->tab_input_bits =
        (uint8_t *) malloc(nb_input_bits * sizeof(uint8_t));
    if (mb_mapping->tab_input_bits == NULL) {
        free(mb_mapping);
        free(mb_mapping->tab_bits);
        return NULL;
    }
    memset(mb_mapping->tab_input_bits, 0,
           nb_input_bits * sizeof(uint8_t));

    /* 4X */
    mb_mapping->nb_registers = nb_registers;
    mb_mapping->tab_registers =
        (uint16_t *) malloc(nb_registers * sizeof(uint16_t));
    if (mb_mapping->tab_registers == NULL) {
        free(mb_mapping);
        free(mb_mapping->tab_bits);
        free(mb_mapping->tab_input_bits);
        return NULL;
    }
    memset(mb_mapping->tab_registers, 0,
           nb_registers * sizeof(uint16_t));

    /* 3X */
    mb_mapping->nb_input_registers = nb_input_registers;
    mb_mapping->tab_input_registers =
        (uint16_t *) malloc(nb_input_registers * sizeof(uint16_t));
    if (mb_mapping->tab_input_registers == NULL) {
        free(mb_mapping);
        free(mb_mapping->tab_bits);
        free(mb_mapping->tab_input_bits);
        free(mb_mapping->tab_registers);
        return NULL;
    }
    memset(mb_mapping->tab_input_registers, 0,
           nb_input_registers * sizeof(uint16_t));

    return mb_mapping;
}

/* Frees the 4 arrays */
void modbus_mapping_free(modbus_mapping_t *mb_mapping)
{
    free(mb_mapping->tab_bits);
    free(mb_mapping->tab_input_bits);
    free(mb_mapping->tab_registers);
    free(mb_mapping->tab_input_registers);
    free(mb_mapping);
}

/* Listens for any request from one or many modbus masters in TCP */
int modbus_listen(modbus_t *ctx, int nb_connection)
{
    int new_socket;
    int yes;
    struct sockaddr_in addr;
    modbus_tcp_t *ctx_tcp = ctx->com;

    if (ctx->type_com != TCP) {
        if (ctx->debug)
            fprintf(stderr, "Not implemented");
        errno = EINVAL;
        return -1;
    }

    new_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (new_socket == -1) {
        return -1;
    }

    yes = 1;
    if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR,
                   (char *) &yes, sizeof(yes)) == -1) {
        close(new_socket);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    /* If the modbus port is < to 1024, we need the setuid root. */
    addr.sin_port = htons(ctx_tcp->port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(new_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(new_socket);
        return -1;
    }

    if (listen(new_socket, nb_connection) == -1) {
        close(new_socket);
        return -1;
    }

    return new_socket;
}

/* On success, the function return a non-negative integer that is a descriptor
   for the accepted socket. On error, -1 is returned, and errno is set
   appropriately. */
int modbus_accept(modbus_t *ctx, int *socket)
{
    struct sockaddr_in addr;
    socklen_t addrlen;

    if (ctx->type_com != TCP) {
        if (ctx->debug)
            fprintf(stderr, "Not implemented");
        errno = EINVAL;
        return -1;
    }

    addrlen = sizeof(struct sockaddr_in);
    ctx->s = accept(*socket, (struct sockaddr *)&addr, &addrlen);
    if (ctx->s == -1) {
        close(*socket);
        *socket = 0;
        return -1;
    }

    if (ctx->debug) {
        printf("The client %s is connected\n", inet_ntoa(addr.sin_addr));
    }

    return ctx->s;
}

/** Utils **/

/* Sets many bits from a single byte value (all 8 bits of the byte value are
   set) */
void modbus_set_bits_from_byte(uint8_t *dest, int address, const uint8_t value)
{
    int i;

    for (i=0; i<8; i++) {
        dest[address+i] = (value & (1 << i)) ? ON : OFF;
    }
}

/* Sets many bits from a table of bytes (only the bits between address and
   address + nb_bits are set) */
void modbus_set_bits_from_bytes(uint8_t *dest, int address, unsigned int nb_bits,
                                const uint8_t tab_byte[])
{
    int i;
    int shift = 0;

    for (i = address; i < address + nb_bits; i++) {
        dest[i] = tab_byte[(i - address) / 8] & (1 << shift) ? ON : OFF;
        /* gcc doesn't like: shift = (++shift) % 8; */
        shift++;
        shift %= 8;
    }
}

/* Gets the byte value from many bits.
   To obtain a full byte, set nb_bits to 8. */
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int address, unsigned int nb_bits)
{
    int i;
    uint8_t value = 0;

    if (nb_bits > 8) {
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
