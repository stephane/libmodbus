/*
 * Copyright © 2022 Ebee Smart GmBH <juan.carrano@ebee.berlin>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <modbus.h>

/* One more than 255 so that we can test the library limit */
#define N_BUFFERS 256

static const char usage[]
    = "Test client for Modbus function (0x2B / 0x0E) Read Device Identification\n"
      "Usage:\n"
      "dev-id-test-client <starting_object_id> [<size_of_first_obj> [<size_of_second> "
      "[...]]]\n";

int main(int argc, char* argv[])
{
    if (argc < 2) {
        puts(usage);
        return -1;
    }

    char* conv_end;
    int starting_object = strtol(argv[1], &conv_end, 0);
    if (*conv_end != '\0') {
        puts("Unable to parse the starting id");
        return -1;
    }

    int n_reqd_objs = argc - 2;
    if (n_reqd_objs > N_BUFFERS) {
        puts("Too many objects requested");
        return -1;
    }

    uint8_t* obj_buffers[N_BUFFERS];
    int buffer_lengths[N_BUFFERS];
    int i = 0;

    for (; i < n_reqd_objs; i++) {
        buffer_lengths[i] = strtol(argv[i + 2], &conv_end, 0);
        if (*conv_end != '\0') {
            printf("Unable to parse the size of buffer %d\n", i);
            return -1;
        }
        obj_buffers[i] = malloc(buffer_lengths[i]);
        assert(obj_buffers[i] != NULL);
    }

    for (; i < N_BUFFERS; i++) {
        obj_buffers[i] = NULL;
        buffer_lengths[i] = 0;
    }

    modbus_t* ctx;

    ctx = modbus_new_tcp("127.0.0.1", 1502);
    modbus_set_debug(ctx, TRUE);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    int orig_buffer_lengths[N_BUFFERS];
    memcpy(orig_buffer_lengths, buffer_lengths, sizeof(buffer_lengths));
    int n_read_objs = modbus_read_device_id(ctx, starting_object, n_reqd_objs,
                                            obj_buffers, buffer_lengths);

    if (n_read_objs == -1) {
        fprintf(stderr, "Read Device ID failed: %s\n", modbus_strerror(errno));
    } else {
        printf("Requested: %d, Read: %d\n", n_reqd_objs, n_read_objs);
        puts("Obj ID\tLength\tTrunc?\tValue");
        for (i = 0; i < n_read_objs; i++) {
            char trunc = orig_buffer_lengths[i] < buffer_lengths[i];
            char data_length = trunc? orig_buffer_lengths[i] : buffer_lengths[i];
            printf("%.2X\t%3d\t%c\t%.*s\t", starting_object + i, buffer_lengths[i],
                   trunc? 'T' : ' ', data_length, (char*)obj_buffers[i]);
            for (int b = 0; b < data_length; b++) {
                printf("[%X]", obj_buffers[i][b]);
            }
            putchar('\n');
        }
    }

    for (i = n_reqd_objs; i < N_BUFFERS; i++) {
        assert(buffer_lengths[i] == 0);
    }

    for (i = 0; i < n_reqd_objs; i++) {
        free(obj_buffers[i]);
    }

    modbus_free(ctx);

    return (n_read_objs > 0) ? 0 : 1;
}
