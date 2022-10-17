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
      "dev-id-test-client <read_code> <starting_object_id> [<size_of_first_obj> [<size_of_second> "
      "[...]]]\n";

static int read_dev_id_repeatedly(modbus_t *ctx, int read_code, int object_id,
                                  int max_objects, uint8_t *obj_ids,
                                  uint8_t **obj_values, int *obj_lengths,
                                  int *conformity)
{
    int total_retrieved = 0, n_retrieved;

    do {
        n_retrieved = modbus_read_device_id(ctx, read_code, object_id,
            max_objects, obj_ids, obj_values, obj_lengths, conformity,
            &object_id);

        total_retrieved += n_retrieved;
        obj_ids += n_retrieved;
        obj_values += n_retrieved;
        obj_lengths += n_retrieved;
        max_objects -= n_retrieved;
    } while (n_retrieved > 0 && object_id > 0
             && object_id < MODBUS_DEVID_MAX_OBJ_ID && max_objects > 0);

    return total_retrieved;
}

static const char *cat2string(int conformity)
{
    switch (MODBUS_READ_DEV_ID_CONFORMITY_CAT(conformity)) {
        default:
            return "unknown";
        case MODBUS_FC_READ_DEV_ID_BASIC_STREAM:
            return "Basic";
        case MODBUS_FC_READ_DEV_ID_REGULAR_STREAM:
            return "Regular";
        case MODBUS_FC_READ_DEV_ID_EXT_STREAM:
            return "Extended";
    }
}

static const char *at2string(int conformity)
{
    if (MODBUS_READ_DEV_ID_SUPPORTS_INDIVIDUAL(conformity)) {
        return "Stream+Individual";
    } else {
        return "Stream only";
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        puts(usage);
        return -1;
    }

    char* conv_end;

    int read_code = strtol(argv[1], &conv_end, 0);
    if (*conv_end != '\0') {
        puts("Unable to parse the read id code");
        return -1;
    }

    int starting_object = strtol(argv[2], &conv_end, 0);
    if (*conv_end != '\0') {
        puts("Unable to parse the starting id");
        return -1;
    }

    int n_reqd_objs = argc - 3;
    if (n_reqd_objs > N_BUFFERS) {
        puts("Too many objects requested");
        return -1;
    }

    uint8_t obj_ids[N_BUFFERS];
    uint8_t* obj_buffers[N_BUFFERS];
    int buffer_lengths[N_BUFFERS];
    int i = 0;

    for (; i < n_reqd_objs; i++) {
        buffer_lengths[i] = strtol(argv[i + 3], &conv_end, 0);
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
    //ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);
    //modbus_set_slave(ctx, 1);
    modbus_set_debug(ctx, TRUE);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    int orig_buffer_lengths[N_BUFFERS];
    memcpy(orig_buffer_lengths, buffer_lengths, sizeof(buffer_lengths));


    int conformity;
    int n_read_objs = read_dev_id_repeatedly(ctx, read_code, starting_object,
        n_reqd_objs, obj_ids, obj_buffers, buffer_lengths, &conformity);

    if (n_read_objs == -1) {
        fprintf(stderr, "Read Device ID failed: %s\n", modbus_strerror(errno));
    } else {
        int actual_avail_objs = (n_read_objs < n_reqd_objs)? n_read_objs : n_reqd_objs;
        printf("Requested: %d, Read: %d\n", n_reqd_objs, actual_avail_objs);
        printf("Reported conformity level: %s %s\n", cat2string(conformity),
               at2string(conformity));
        puts("Obj ID\tLength\tTrunc?\tValue");
        for (i = 0; i < actual_avail_objs; i++) {
            char trunc = orig_buffer_lengths[i] < buffer_lengths[i];
            char data_length = trunc? orig_buffer_lengths[i] : buffer_lengths[i];
            printf("%.2X\t%3d\t%c\t%.*s\t", obj_ids[i], buffer_lengths[i],
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
