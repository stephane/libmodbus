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

int main(int argc, char * argv [])
{
    int s = -1;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;

    // fuzz
    size_t insize;
    #define FUZZ_BUFF_SIZE 5120
    char buf[FUZZ_BUFF_SIZE];
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    socklen_t sizeof_serv_addr = sizeof(serv_addr);
    // fuzz

    ctx = modbus_new_tcp("127.0.0.1", 0);
    /* modbus_set_debug(ctx, TRUE); */

    mb_mapping = modbus_mapping_new(500, 500, 500, 500);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    s = modbus_tcp_listen(ctx, 1);
    if (s < 0) {
        error(1, 1, "\nError : Failed to start server\n");
    }

    int getsockname_result = getsockname(s, &serv_addr, &sizeof_serv_addr);
    if ( getsockname_result < 0 ) {
        error(1, getsockname_result, "Failed to get server port number");
    }

    for (;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc;

        // fuzz
        memset(buf, 0, FUZZ_BUFF_SIZE);
        insize = fread(buf, 1, sizeof(buf), stdin);
        //if ( !feof(0) ) {
        //    error(1, 1, "ERROR not all read");
        //}
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            error(1, 1, "ERROR opening socket");
        }
        if( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof_serv_addr) < 0) {
            error(1, 1, "\nError : Connect Failed\n");
        } 
        modbus_tcp_accept(ctx, &s);
        sendto(sockfd, buf, insize, 0, NULL, -1);
        // fuzz

        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            /* rc is the query size */
            modbus_reply(ctx, query, rc, mb_mapping);
        } else if (rc == -1) {
            /* Connection closed by the client or error */
            break;
        }

        // fuzz
        break;
        // fuzz
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (s != -1) {
        close(s);
    }
    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
