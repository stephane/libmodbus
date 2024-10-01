/*
 * Substation (Client)
 */

#include <errno.h>
#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // sleep

#define PV_SERVER_IP "127.0.0.1"
#define SERVER_PORT 1502
#define HOLDING_REGISTERS_NB 10
#define HOLDING_REGISTERS_ADDRESS 0 

int main(void) {
    modbus_t *ctx;
    uint16_t register_values[HOLDING_REGISTERS_NB];
    int rc;

    // TCP -> Need to be change with  TCP_PI and RTU
    ctx = modbus_new_tcp(PV_SERVER_IP, SERVER_PORT);
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    printf("Substation connected to PV Inverter...\n");

    // Keep reading register from server
    while (1) {
        // Read register
        rc = modbus_read_registers(ctx, HOLDING_REGISTERS_ADDRESS, HOLDING_REGISTERS_NB, register_values);
        if (rc == -1) {
            fprintf(stderr, "Failed to read registers: %s\n", modbus_strerror(errno));
            sleep(1);
            continue;
        }

        printf("Current register values from PV Inverter:\n");
        for (int i = 0; i < HOLDING_REGISTERS_NB; i++) {
            printf("Register %d: %d\n", i, register_values[i]);
        }

        sleep(1);
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
