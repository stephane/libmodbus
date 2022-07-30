/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 * Özgür Keleş <ozgurkeles@gmail.com>
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

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include <modbus.h>

int main(void)
{
    int s = -1;
    modbus_t *ctx;
    modbus_t *ctx_rtu;

    ctx = modbus_new_tcp("127.0.0.1", 1502);
    /* modbus_set_debug(ctx, TRUE); */

    ctx_rtu = modbus_new_rtu("/dev/ttyUSB1", 115200, 'N', 8, 1);

    if (modbus_connect(ctx_rtu) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx_rtu);
        modbus_free(ctx);
        return -1;
    }

    s = modbus_tcp_listen(ctx, 1);
    modbus_tcp_accept(ctx, &s);

    for (;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int query_length;
        uint8_t response[MODBUS_RTU_MAX_ADU_LENGTH];
        int rc;
        int data_start_index;
        int raw_length;
        int exception;
        int slave;

        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            exception = 0;
            query_length = rc;
            data_start_index = modbus_get_header_length(ctx) - 1;
            slave = query[data_start_index];
            raw_length = rc - data_start_index - modbus_get_checksum_length(ctx);
            modbus_flush(ctx_rtu);
            modbus_set_slave(ctx_rtu, slave);
            if (modbus_send_raw_request(ctx_rtu, query + data_start_index, raw_length) != -1) {
                rc = modbus_receive_confirmation(ctx_rtu, response);
                if ( rc != -1) {
                    /* rc is the response size */
                    data_start_index = modbus_get_header_length(ctx_rtu) - 1;
                    raw_length = rc - data_start_index - modbus_get_checksum_length(ctx_rtu);
                    modbus_reply_raw_response(ctx, query, query_length, response + data_start_index, raw_length);
                } else {
                    exception = errno;
                }
            } else {
                exception = errno;
            }

            if (exception != 0) {
                if (exception > MODBUS_ENOBASE && MODBUS_ENOBASE < (MODBUS_ENOBASE + MODBUS_EXCEPTION_MAX)) {
                    exception -= MODBUS_ENOBASE;
                } else {
                    exception = EMBXSFAIL;
                }
                modbus_reply_exception(ctx, query, exception);
            }
        } else if (rc == -1) {
            /* Connection closed by the client or error */
            break;
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (s != -1) {
        close(s);
    }

    modbus_close(ctx_rtu);
    modbus_free(ctx_rtu);

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
