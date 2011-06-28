/*
 * Copyright © 2008-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include <modbus.h>

#define G_MSEC_PER_SEC 1000

uint32_t gettime_ms(void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);

    return (uint32_t) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

enum {
    TCP,
    RTU
};

/* Tests based on PI-MBUS-300 documentation */
int main(int argc, char *argv[])
{
    uint8_t *tab_bit;
    uint16_t *tab_reg;
    modbus_t *ctx;
    int i;
    int nb_points;
    double elapsed;
    uint32_t start;
    uint32_t end;
    uint32_t bytes;
    uint32_t rate;
    int rc;
    int n_loop;
    int use_backend;

    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            use_backend = TCP;
            n_loop = 100000;
        } else if (strcmp(argv[1], "rtu") == 0) {
            use_backend = RTU;
            n_loop = 100;
        } else {
            printf("Usage:\n  %s [tcp|rtu] - Modbus client to measure data bandwith\n\n", argv[0]);
            exit(1);
        }
    } else {
        /* By default */
        use_backend = TCP;
        n_loop = 100000;
    }

    if (use_backend == TCP) {
        ctx = modbus_new_tcp("127.0.0.1", 1502);
    } else {
        ctx = modbus_new_rtu("/dev/ttyUSB1", 115200, 'N', 8, 1);
        modbus_set_slave(ctx, 1);
    }
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connexion failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Allocate and initialize the memory to store the status */
    tab_bit = (uint8_t *) malloc(MODBUS_MAX_READ_BITS * sizeof(uint8_t));
    memset(tab_bit, 0, MODBUS_MAX_READ_BITS * sizeof(uint8_t));

    /* Allocate and initialize the memory to store the registers */
    tab_reg = (uint16_t *) malloc(MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));
    memset(tab_reg, 0, MODBUS_MAX_READ_REGISTERS * sizeof(uint16_t));

    printf("READ BITS\n\n");

    nb_points = MODBUS_MAX_READ_BITS;
    start = gettime_ms();
    for (i=0; i<n_loop; i++) {
        rc = modbus_read_bits(ctx, 0, nb_points, tab_bit);
        if (rc == -1) {
            fprintf(stderr, "%s\n", modbus_strerror(errno));
            return -1;
        }
    }
    end = gettime_ms();
    elapsed = end - start;

    rate = (n_loop * nb_points) * G_MSEC_PER_SEC / (end - start);
    printf("Transfert rate in points/seconds:\n");
    printf("* %d points/s\n", rate);
    printf("\n");

    bytes = n_loop * (nb_points / 8) + ((nb_points % 8) ? 1 : 0);
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
    printf("Values:\n");
    printf("* %d x %d values\n", n_loop, nb_points);
    printf("* %.3f ms for %d bytes\n", elapsed, bytes);
    printf("* %d KiB/s\n", rate);
    printf("\n");

    /* TCP: Query and reponse header and values */
    bytes = 12 + 9 + (nb_points / 8) + ((nb_points % 8) ? 1 : 0);
    printf("Values and TCP Modbus overhead:\n");
    printf("* %d x %d bytes\n", n_loop, bytes);
    bytes = n_loop * bytes;
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
    printf("* %.3f ms for %d bytes\n", elapsed, bytes);
    printf("* %d KiB/s\n", rate);
    printf("\n\n");

    printf("READ REGISTERS\n\n");

    nb_points = MODBUS_MAX_READ_REGISTERS;
    start = gettime_ms();
    for (i=0; i<n_loop; i++) {
        rc = modbus_read_registers(ctx, 0, nb_points, tab_reg);
        if (rc == -1) {
            fprintf(stderr, "%s\n", modbus_strerror(errno));
            return -1;
        }
    }
    end = gettime_ms();
    elapsed = end - start;

    rate = (n_loop * nb_points) * G_MSEC_PER_SEC / (end - start);
    printf("Transfert rate in points/seconds:\n");
    printf("* %d registers/s\n", rate);
    printf("\n");

    bytes = n_loop * nb_points * sizeof(uint16_t);
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
    printf("Values:\n");
    printf("* %d x %d values\n", n_loop, nb_points);
    printf("* %.3f ms for %d bytes\n", elapsed, bytes);
    printf("* %d KiB/s\n", rate);
    printf("\n");

    /* TCP:Query and reponse header and values */
    bytes = 12 + 9 + (nb_points * sizeof(uint16_t));
    printf("Values and TCP Modbus overhead:\n");
    printf("* %d x %d bytes\n", n_loop, bytes);
    bytes = n_loop * bytes;
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
    printf("* %.3f ms for %d bytes\n", elapsed, bytes);
    printf("* %d KiB/s\n", rate);
    printf("\n\n");

    printf("READ AND WRITE REGISTERS\n\n");

    nb_points = MODBUS_MAX_RW_WRITE_REGISTERS;
    start = gettime_ms();
    for (i=0; i<n_loop; i++) {
        rc = modbus_write_and_read_registers(ctx,
                                             0, nb_points, tab_reg,
                                             0, nb_points, tab_reg);
        if (rc == -1) {
            fprintf(stderr, "%s\n", modbus_strerror(errno));
            return -1;
        }
    }
    end = gettime_ms();
    elapsed = end - start;

    rate = (n_loop * nb_points) * G_MSEC_PER_SEC / (end - start);
    printf("Transfert rate in points/seconds:\n");
    printf("* %d registers/s\n", rate);
    printf("\n");

    bytes = n_loop * nb_points * sizeof(uint16_t);
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
    printf("Values:\n");
    printf("* %d x %d values\n", n_loop, nb_points);
    printf("* %.3f ms for %d bytes\n", elapsed, bytes);
    printf("* %d KiB/s\n", rate);
    printf("\n");

    /* TCP:Query and reponse header and values */
    bytes = 12 + 9 + (nb_points * sizeof(uint16_t));
    printf("Values and TCP Modbus overhead:\n");
    printf("* %d x %d bytes\n", n_loop, bytes);
    bytes = n_loop * bytes;
    rate = bytes / 1024 * G_MSEC_PER_SEC / (end - start);
    printf("* %.3f ms for %d bytes\n", elapsed, bytes);
    printf("* %d KiB/s\n", rate);
    printf("\n");

    /* Free the memory */
    free(tab_bit);
    free(tab_reg);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
