/*
 * PV Inverter (Server)
 */

#include <errno.h>
#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_ID 1
#define HOLDING_REGISTERS_NB 10
#define HOLDING_REGISTERS_ADDRESS 0

int main(void) {
    int server_socket;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

    // TCP based PV Inverter Server -> To be changed to TCP_PI and RTU
    ctx = modbus_new_tcp("127.0.0.1", 1502); // IP와 포트 지정
    modbus_set_slave(ctx, SERVER_ID);

    // Modbus Register Mapping Setting
    mb_mapping = modbus_mapping_new(0, 0, HOLDING_REGISTERS_NB, 0);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    // PV Inverter register initialization
    mb_mapping->tab_registers[0] = 100;

    // Server is waitting
    server_socket = modbus_tcp_listen(ctx, 1);
    modbus_tcp_accept(ctx, &server_socket);

    for (;;) {
        printf("Enter the register address to write to Substation: ");
        int address;
        scanf("%d", &address);

        if (address < 0 || address >= HOLDING_REGISTERS_NB) {
            printf("Invalid address. Valid range is 0 to %d\n", HOLDING_REGISTERS_NB - 1);
            continue;
        }

        printf("Enter a value to write to register %d: ", address);
        uint16_t value;
        scanf("%hu", &value);

        // Update Register
        mb_mapping->tab_registers[address] = value;
        printf("Updated register %d with value: %d\n", address, value);

        // Recieve from client
        int rc = modbus_receive(ctx, query);
        if (rc > 0) {
            modbus_reply(ctx, query, rc, mb_mapping);  // Reply to client
        } else if (rc == -1) {
            printf("Error in communication with Substation: %s\n", modbus_strerror(errno));
            break;
        }
    }

    printf("Server closed.\n");

    close(server_socket);
    modbus_mapping_free(mb_mapping);
    modbus_free(ctx);
    return 0;
}
