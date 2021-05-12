/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <error.h>
//#include <fcntl.h>

#include <modbus.h>

#define MAX_FUZZ_INPUT_SIZE 5120

int main(int argc, char * argv [])
{
    // general
    int e;

    // server
    int socket_of_server = -1;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping = NULL;

    // fuzz and client
    size_t insize;
    char buf[MAX_FUZZ_INPUT_SIZE];
    int socket_to_server;
    struct sockaddr_in serv_addr;
    socklen_t sizeof_serv_addr = sizeof(serv_addr);

    // init server
    ctx = modbus_new_tcp("127.0.0.1", 0);
    socket_of_server = modbus_tcp_listen(ctx, 1);
    if (socket_of_server < 0) {
        error(-1, 1, "Failed to start server: %s", modbus_strerror(errno));
    }

    // get the port of the server
    int getsockname_result = getsockname(socket_of_server, &serv_addr, &sizeof_serv_addr);
    if ( getsockname_result < 0 ) {
        error(-1, getsockname_result, "Failed to get server port number");
    }

    #ifdef __AFL_LOOP
    while (__AFL_LOOP(1000)) {
    #else
    do {
    #endif
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc;

        // Reset state from last fuzz
        modbus_mapping_free(mb_mapping);
        mb_mapping = modbus_mapping_new(500, 500, 500, 500);
        if (mb_mapping == NULL) {
            error(-1, -1, "Failed to allocate the mapping: %s", modbus_strerror(errno));
        }

        // Read fizzing data to send to server
        memset(buf, 0, sizeof(buf));
        insize = fread(buf, 1, sizeof(buf), stdin);
        // TODO: Confirm whole test case was read

        // Connect to server
        socket_to_server = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_to_server < 0) {
            error(1, socket_to_server, "ERROR opening socket");
        }
        e = connect(socket_to_server, (struct sockaddr *) &serv_addr, sizeof_serv_addr);
        if(e < 0) {
            error(1, e, "\nError : Connect to server Failed\n");
        } 

        // Send fuzzing data to server
        sendto(socket_to_server, buf, insize, 0, NULL, -1);

        // Run a general ModBus server logic
        modbus_tcp_accept(ctx, &socket_of_server);
        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            /* rc is the query size */
            modbus_reply(ctx, query, rc, mb_mapping);
        } else if (rc == -1) {
            /* Connection closed by the client or error */
            break;
        }

        // Read response from server and close connection
        e = read(socket_to_server, buf, sizeof(buf));
        if (e < 0) {
            error(1, e, "Failed to read response from server");
        }
        e = close(socket_to_server);
        if (e < 0) {
            error(1, e, "Failed to close socket to server");
        }

    #ifdef __AFL_LOOP
    }
    #else
    } while (0);
    #endif

    if (socket_of_server >= 0) {
        close(socket_of_server);
    }
    if (socket_to_server >= 0) {
        close(socket_to_server);
    }
    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
