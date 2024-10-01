/*
 * Substation (Client)
 */

#include <errno.h>
#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // sleep

#define PV_SERVER_IP "10.10.1.50"  // Update with the actual IP of PV Inverter
#define SERVER_PORT 1503          // Update with the actual port if needed
#define HOLDING_REGISTERS_NB 10
#define HOLDING_REGISTERS_ADDRESS 0

int main(void) {
    modbus_t *ctx;
    uint16_t register_values[HOLDING_REGISTERS_NB];
    int rc;
    uint16_t address;
    uint16_t value;

    // Connect to PV Inverter via TCP (Can be updated to TCP_PI or RTU if needed)
    ctx = modbus_new_tcp(PV_SERVER_IP, SERVER_PORT);
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    printf("Substation connected to PV Inverter...\n");

    while (1) {
        // Receive address and value from the Modbus server (PV Inverter)

        printf("Waiting for PV Inverter to send register modification...\n");

        // Read specific register address from PV Inverter
        rc = modbus_read_registers(ctx, HOLDING_REGISTERS_ADDRESS, 1, &address); // Modify here
        if (rc == -1) {
            fprintf(stderr, "Failed to read register address: %s\n", modbus_strerror(errno));
            sleep(1);
            continue;
        }

        // Read the value to be set for that address
        rc = modbus_read_registers(ctx, HOLDING_REGISTERS_ADDRESS + 1, 1, &value); // Modify here
        if (rc == -1) {
            fprintf(stderr, "Failed to read register value: %s\n", modbus_strerror(errno));
            sleep(1);
            continue;
        }

        printf("Received request to update register %d with value %d\n", address, value);

        // Write the value to the corresponding register
        rc = modbus_write_register(ctx, address, value);
        if (rc == -1) {
            fprintf(stderr, "Failed to write register: %s\n", modbus_strerror(errno));
            sleep(1);
            continue;
        }

        printf("Updated register %d with value %d\n", address, value);
        sleep(1);
    }

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
