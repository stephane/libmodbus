/*
 * Substation (Client)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <modbus.h>

#define FILE_NAME "modbus_registers.txt"
#define NUM_REGISTERS 11

#define MODBUS_IP "10.10.2.100"
#define MODBUS_PORT 1502

void initialize_registers(int *registers);
void check_file(int *prev_values, modbus_t *ctx);
modbus_t* initialize_modbus(void);

void initialize_registers(int *registers) {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        registers[i] = 10;
    }
}

void check_file(int *prev_values, modbus_t *ctx) {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file for reading\n");
        return;
    }

    int address, value;
    while (fscanf(file, "%d %d", &address, &value) != EOF) {
        if (address >= 0 && address < NUM_REGISTERS) {
            if (prev_values[address] != value) {
                printf("Register %d changed from %d to %d\n", address, prev_values[address], value);
                prev_values[address] = value;

                // Let Garibaldi changing value via Modbus
                if (ctx != NULL) {
                    int rc = modbus_write_register(ctx, address, value);
                    if (rc == -1) {
                        fprintf(stderr, "Failed to notify Modbus device: %s\n", modbus_strerror(errno));
                    } else {
                        printf("Notified Modbus device: Register %d updated to %d\n", address, value);
                    }
                }
            }
        }
    }
    fclose(file);
}

modbus_t* initialize_modbus(void) {
    modbus_t *ctx = modbus_new_tcp(MODBUS_IP, MODBUS_PORT);
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection to Modbus server failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return NULL;
    }
    printf("Connected to Modbus server at %s:%d\n", MODBUS_IP, MODBUS_PORT);
    return ctx;
}

int main(int argc, char* argv[]) {
    int prev_values[NUM_REGISTERS];
    initialize_registers(prev_values);

    printf("Watching the file for changes...\n");

    // Modbus Connection Setting
    modbus_t *ctx = initialize_modbus();

    while (1) {
        check_file(prev_values, ctx);
        sleep(1);
    }

    if (ctx != NULL) {
        modbus_close(ctx);
        modbus_free(ctx);
    }

    return 0;
}


/*
#include <errno.h>
#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // sleep

#define PV_SERVER_IP "192.168.0.194"  // Update with the actual IP of PV Inverter
#define SERVER_PORT 1502          // Update with the actual port if needed
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
}*/
