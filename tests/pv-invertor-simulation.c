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
#define FILE_NAME "modbus_registers.txt"
#define NUM_REGISTERS 10  // Total number of registers

// Prototypes
int set_backend_simulation(int argc, char* argv[]);
char* set_ip_or_device_simulation(int use_backend, int argc, char* argv[]);
modbus_t* initialize_modbus_context_simulation(int use_backend, char* ip_or_device);
modbus_mapping_t* initialize_modbus_mapping_simulation(void);
int setup_server_simulation(int use_backend, modbus_t* ctx);
void update_file(int address, int value);

enum {
    TCP,
    TCP_PI,
    RTU
};

// Function to set the communication backend (TCP, TCP_PI, RTU)
int set_backend_simulation(int argc, char* argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            return TCP;
        } else if (strcmp(argv[1], "tcppi") == 0) {
            return TCP_PI;
        } else if (strcmp(argv[1], "rtu") == 0) {
            return RTU;
        } else {
            printf("Modbus server for unit testing.\n");
            printf("Usage:\n  %s [tcp|tcppi|rtu] [<ip or device>]\n", argv[0]);
            printf("Eg. tcp 127.0.0.1 or rtu /dev/ttyUSB0\n\n");
            return -1;
        }
    }
    return TCP; // Default is TCP
}

// Function to set the IP address or device based on the selected backend
char* set_ip_or_device_simulation(int use_backend, int argc, char* argv[]) {
    if (argc > 2) {
        return argv[2];  // User-provided IP or device
    }

    switch (use_backend) {
        case TCP:
            return "127.0.0.1";
        case TCP_PI:
            return "::1";
        case RTU:
            return "/dev/ttyUSB0";
        default:
            return NULL;
    }
}

// Initialize the Modbus TCP/RTU context
modbus_t* initialize_modbus_context_simulation(int use_backend, char* ip_or_device) {
    modbus_t* ctx = NULL;
    if (use_backend == TCP) {
        ctx = modbus_new_tcp(ip_or_device, 1502);  // Use TCP on port 1502
    } else if (use_backend == TCP_PI) {
        ctx = modbus_new_tcp_pi(ip_or_device, "1502");  // Use TCP_PI
    } else {
        ctx = modbus_new_rtu(ip_or_device, 115200, 'N', 8, 1);  // Use RTU
        modbus_set_slave(ctx, SERVER_ID);
    }
    return ctx;
}

// Initialize Modbus register mapping
modbus_mapping_t* initialize_modbus_mapping_simulation() {
    modbus_mapping_t* mb_mapping = modbus_mapping_new_start_address(0, 0, 0, 0, 0, NUM_REGISTERS, 0, 0);  // Set 10 registers
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
    }
    return mb_mapping;
}

// // Function to update the file with a specific register address and its value
// void update_file(int address, int value) {
//     FILE *file = fopen(FILE_NAME, "a");  // Append to the file instead of overwriting
//     if (file == NULL) {
//         fprintf(stderr, "Failed to open file for writing\n");
//         return;
//     }

//     // Write the address and value to the file
//     fprintf(file, "%d %d\n", address, value);
//     fclose(file);
// }
// Update the value of a specific register in the file
void update_file(int address, int value) {
    FILE *file = fopen(FILE_NAME, "r+");  // Open file for reading and writing
    if (file == NULL) {
        fprintf(stderr, "Failed to open file for reading/writing\n");
        return;
    }

    char buffer[256];  // Buffer for reading lines
    char temp_file[] = "temp_modbus_registers.txt";  // Temporary file for updates
    FILE *temp = fopen(temp_file, "w");  // Temporary file to write updates
    if (temp == NULL) {
        fprintf(stderr, "Failed to create temporary file\n");
        fclose(file);
        return;
    }

    int current_address, current_value;
    int line_number = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        if (sscanf(buffer, "%d %d", &current_address, &current_value) == 2) {
            printf("current address = %d, address = %d, value = %d\n",current_address, address, value);
            if (current_address == address) {
                // If the current line matches the address, update the value
                fprintf(temp, "%d %d\n", address, value);
            } else {
                // Otherwise, write the same content to the temp file
                fputs(buffer, temp);
            }
        }
        line_number++;
    }

    // If the file was shorter than expected, append any missing registers
    for (int i = line_number; i < NUM_REGISTERS; i++) {
        if (i == address) {
            fprintf(temp, "%d %d\n", address, value);  // Append the updated register
        } else {
            fprintf(temp, "%d 10\n", i);  // Initialize missing registers with default value 10
        }
    }

    fclose(file);
    fclose(temp);

    // Replace the original file with the updated temporary file
    remove(FILE_NAME);
    rename(temp_file, FILE_NAME);
}

// Set up the Modbus server for communication
int setup_server_simulation(int use_backend, modbus_t* ctx) {
    int socket_file_descriptor = -1;
    if (use_backend == TCP) {
        socket_file_descriptor = modbus_tcp_listen(ctx, 1);
        if (socket_file_descriptor == -1) {
            fprintf(stderr, "Failed to listen: %s\n", modbus_strerror(errno));
            return -1;
        }
        int tmp = modbus_tcp_accept(ctx, &socket_file_descriptor);
        if (tmp == -1) {
            fprintf(stderr, "Error in modbus_tcp_accept: %s\n", modbus_strerror(errno));
            return -1;
        }
    } else if (use_backend == TCP_PI) {
        socket_file_descriptor = modbus_tcp_pi_listen(ctx, 1);
        modbus_tcp_pi_accept(ctx, &socket_file_descriptor);
    } else {
        int rc = modbus_connect(ctx);
        if (rc == -1) {
            fprintf(stderr, "Unable to connect: %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;
        }
    }
    return socket_file_descriptor;
}

int main(int argc, char* argv[]) {
    int socket_file_descriptor = -1;
    int use_backend = set_backend_simulation(argc, argv);
    char* ip_or_device = set_ip_or_device_simulation(use_backend, argc, argv);
    modbus_t* ctx;
    modbus_mapping_t* mb_mapping;
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH] = {0};  // Buffer for receiving Modbus requests
    int rc, header_length;

    if (use_backend == -1 || ip_or_device == NULL) {
        return -1;
    }

    // Initialize Modbus context
    ctx = initialize_modbus_context_simulation(use_backend, ip_or_device);
    if (ctx == NULL) {
        fprintf(stderr, "Failed to create Modbus context\n");
        return -1;
    }

    // Set debug mode for logging
    modbus_set_debug(ctx, TRUE);

    // Initialize Modbus mapping for registers
    mb_mapping = initialize_modbus_mapping_simulation();
    if (mb_mapping == NULL) {
        modbus_free(ctx);
        return -1;
    }

    // Set up the server
    socket_file_descriptor = setup_server_simulation(use_backend, ctx);
    if (socket_file_descriptor == -1) {
        printf("socket_file_descriptor = -1\n");
        modbus_mapping_free(mb_mapping);
        modbus_free(ctx);
        return -1;
    }

    header_length = modbus_get_header_length(ctx);  // Set the header length for the protocol

    // Initialize all registers to 10
    for (int i = 0; i < NUM_REGISTERS; i++) {
        mb_mapping->tab_registers[i] = 10;
    }

    // Main loop for handling Modbus requests
    for (;;) {
        do {
            rc = modbus_receive(ctx, query);
        } while (rc == 0);  // Wait until non-zero data is received

        if (rc == -1 && errno != EMBBADCRC) {
            printf("Communication error: %s\n", modbus_strerror(errno));
            break;
        }

        // Handle register update requests (single register, multiple registers)
        if (query[header_length] == 0x06) {  // MODBUS_FC_WRITE_SINGLE_REGISTER
            int address = MODBUS_GET_INT16_FROM_INT8(query, header_length + 1);
            uint16_t value = MODBUS_GET_INT16_FROM_INT8(query, header_length + 4);
            printf("Current register value at %d: %d\n", address, mb_mapping->tab_registers[address]);
            mb_mapping->tab_registers[address] = value;  // Update the register value
            printf("Updated register %d with value: %d\n", address, value);

            // Update the file with the new register value
            update_file(address, value);
        }

        // Send response to the Modbus client
        rc = modbus_reply(ctx, query, rc, mb_mapping);
        if (rc == -1) {
            printf("Error in sending reply: %s\n", modbus_strerror(errno));
            break;
        }
    }

    // Cleanup and shutdown
    if (use_backend == TCP || use_backend == TCP_PI) {
        if (socket_file_descriptor != -1) {
            close(socket_file_descriptor);
        }
    }
    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}

/*
#include <errno.h>
#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "unit-test.h"

// Prototypes
int set_backend_simulation(int argc, char* argv[]);
char* set_ip_or_device_simulation(int use_backend, int argc, char* argv[]);
modbus_t* initialize_modbus_context_simulation(int use_backend, char* ip_or_device);
modbus_mapping_t* initialize_modbus_mapping_simulation(void);
void initialize_register_values_simulation(modbus_mapping_t* mb_mapping);
int setup_server_simulation(int use_backend, modbus_t* ctx);
int send_to_substation(modbus_t* substation_ctx, int address, uint16_t value);

#define SERVER_ID_PV 1
#define HOLDING_REGISTERS_NB 10
#define HOLDING_REGISTERS_ADDRESS 0
#define SUBSTATION_IP "10.10.1.50"  // Substation's IP address (Update if necessary)
#define SUBSTATION_PORT 1502       // Substation port (for TCP)

enum {
    TCP,
    TCP_PI,
    RTU
};

// Function to set the backend (TCP, TCP_PI, RTU)
int set_backend_simulation(int argc, char* argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            return TCP;
        } else if (strcmp(argv[1], "tcppi") == 0) {
            return TCP_PI;
        } else if (strcmp(argv[1], "rtu") == 0) {
            return RTU;
        } else {
            printf("Modbus server for unit testing.\n");
            printf("Usage:\n  %s [tcp|tcppi|rtu] [<ip or device>]\n", argv[0]);
            printf("Eg. tcp 127.0.0.1 or rtu /dev/ttyUSB0\n\n");
            return -1;
        }
    }
    return TCP; // Default backend is TCP
}

// Function to set the IP address or device
char* set_ip_or_device_simulation(int use_backend, int argc, char* argv[]) {
    if (argc > 2) {
        return argv[2];  // User-provided IP or device
    }

    switch (use_backend) {
        case TCP:
            return "127.0.0.1";
        case TCP_PI:
            return "::1";
        case RTU:
            return "/dev/ttyUSB0";
        default:
            return NULL;
    }
}

// Function to initialize the Modbus TCP/RTU context
modbus_t* initialize_modbus_context_simulation(int use_backend, char* ip_or_device) {
    modbus_t* ctx = NULL;
    if (use_backend == TCP) {
        ctx = modbus_new_tcp(ip_or_device, 1502);
    } else if (use_backend == TCP_PI) {
        ctx = modbus_new_tcp_pi(ip_or_device, "1502");
    } else {
        ctx = modbus_new_rtu(ip_or_device, 115200, 'N', 8, 1);
        modbus_set_slave(ctx, SERVER_ID_PV);
    }
    return ctx;
}

// Function to initialize Modbus mapping
modbus_mapping_t* initialize_modbus_mapping_simulation() {
    modbus_mapping_t* mb_mapping = modbus_mapping_new_start_address(UT_BITS_ADDRESS,
                                                  UT_BITS_NB,
                                                  UT_INPUT_BITS_ADDRESS,
                                                  UT_INPUT_BITS_NB,
                                                  UT_REGISTERS_ADDRESS,
                                                  UT_REGISTERS_NB_MAX,
                                                  UT_INPUT_REGISTERS_ADDRESS,
                                                  UT_INPUT_REGISTERS_NB);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
    }
    return mb_mapping;
}

// Function to initialize input values for Modbus registers
void initialize_register_values_simulation(modbus_mapping_t* mb_mapping) {
    modbus_set_bits_from_bytes(mb_mapping->tab_input_bits, 0, UT_INPUT_BITS_NB, UT_INPUT_BITS_TAB);
    for (int i = 0; i < UT_INPUT_REGISTERS_NB; i++) {
        mb_mapping->tab_input_registers[i] = UT_INPUT_REGISTERS_TAB[i];
    }
}

// Function to set up the server based on the backend
int setup_server_simulation(int use_backend, modbus_t* ctx) {
    int socket_file_descriptor = -1;
    int rc;
    if (use_backend == TCP) {
        socket_file_descriptor = modbus_tcp_listen(ctx, 1);
        if (socket_file_descriptor == -1) {
            fprintf(stderr, "Failed to listen: %s\n", modbus_strerror(errno));
            return -1;
        }
        int tmp = modbus_tcp_accept(ctx, &socket_file_descriptor);
        if (tmp == -1) {
            fprintf(stderr, "Error in modbus_tcp_accept: %s\n", modbus_strerror(errno));
            return -1;
        }
    } else if (use_backend == TCP_PI) {
        socket_file_descriptor = modbus_tcp_pi_listen(ctx, 1);
        modbus_tcp_pi_accept(ctx, &socket_file_descriptor);
    } else {
        rc = modbus_connect(ctx);
        if (rc == -1) {
            fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;
        }
    }
    return socket_file_descriptor;
}

// Function to send data to the substation
int send_to_substation(modbus_t* substation_ctx, int address, uint16_t value) {  //modify
    int rc = modbus_write_register(substation_ctx, address, value); //modify
    if (rc == -1) {
        fprintf(stderr, "Failed to send data to substation: %s\n", modbus_strerror(errno));
        return -1;
    }
    printf("Sent to substation: register %d updated with value %d\n", address, value); //modify
    return 0;
}

int main(int argc, char* argv[]) {
    int socket_file_descriptor = -1;
    int use_backend = set_backend_simulation(argc, argv);
    char* ip_or_device = set_ip_or_device_simulation(use_backend, argc, argv);
    modbus_t* ctx, *substation_ctx;
    modbus_mapping_t* mb_mapping;
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH] = {0};  // Query buffer initialization
    int rc, header_length;

    if (use_backend == -1 || ip_or_device == NULL) {
        return -1;
    }

    // Initialize Modbus context
    ctx = initialize_modbus_context_simulation(use_backend, ip_or_device);
    if (ctx == NULL) {
        fprintf(stderr, "Failed to create Modbus context\n");
        return -1;
    }

    // Set debug mode
    modbus_set_debug(ctx, TRUE);

    // Initialize Modbus mapping
    mb_mapping = initialize_modbus_mapping_simulation();
    if (mb_mapping == NULL) {
        modbus_free(ctx);
        return -1;
    }

    // Initialize register values
    initialize_register_values_simulation(mb_mapping);

    // Set up the server
    socket_file_descriptor = setup_server_simulation(use_backend, ctx);
    if (socket_file_descriptor == -1) {
        printf("socket_file_descriptor = -1\n");
        modbus_mapping_free(mb_mapping);
        modbus_free(ctx);
        return -1;
    }

    // Connect to substation
    substation_ctx = modbus_new_tcp(SUBSTATION_IP, SUBSTATION_PORT);
    int tmp = modbus_connect(substation_ctx);
    if (tmp == -1) {
        fprintf(stderr, "Failed to connect to substation: %s\n", modbus_strerror(errno));
        // 연결 실패 시 재시도 처리
        int retries = 0;
        while (retries < 20 && tmp == -1) {
            printf("Failed to connect to substation, retrying... to %s, port =%d\n",SUBSTATION_IP, SUBSTATION_PORT);
            tmp = modbus_connect(substation_ctx);
            sleep(2);  // 재시도 간 대기 시간
            retries++;
        }
        modbus_free(substation_ctx);
        modbus_mapping_free(mb_mapping);
        modbus_free(ctx);
        return -1;
    }
    printf("Connected to substation.\n"); 

    header_length = modbus_get_header_length(ctx);  // Set header length

    // Main loop for handling Modbus requests
    for (;;) {
        do {
            rc = modbus_receive(ctx, query);
        } while (rc == 0);  // Wait until non-zero data is received

        if (rc == -1 && errno != EMBBADCRC) {
            printf("Communication error: %s\n", modbus_strerror(errno));
            break;
        }

        // Process register update requests (single register, coil, multiple registers)
        if (query[header_length] == 0x06) {  // MODBUS_FC_WRITE_SINGLE_REGISTER
            int address = MODBUS_GET_INT16_FROM_INT8(query, header_length + 1);
            uint16_t value = MODBUS_GET_INT16_FROM_INT8(query, header_length + 4);
            printf("Current register value at %d: %d\n", address, mb_mapping->tab_registers[address]);
            mb_mapping->tab_registers[address] = value;
            printf("Updated register %d with value: %d\n", address, value);

            // Send the same update to the substation
            send_to_substation(substation_ctx, address, value);

        } else if (query[header_length] == 0x05) {  // MODBUS_FC_WRITE_SINGLE_COIL
            int address = MODBUS_GET_INT16_FROM_INT8(query, header_length + 1);
            uint8_t value = query[header_length + 4];
            printf("Current coil value at %d: %d\n", address, mb_mapping->tab_bits[address]);
            mb_mapping->tab_bits[address] = value ? 1 : 0;
            printf("Updated coil %d with value: %d\n", address, value);
        } else if (query[header_length] == 0x10) {  // MODBUS_FC_WRITE_MULTIPLE_REGISTERS
            int address = MODBUS_GET_INT16_FROM_INT8(query, header_length + 1);
            int reg_count = MODBUS_GET_INT16_FROM_INT8(query, header_length + 3);
            for (int i = 0; i < reg_count; i++) {
                uint16_t value = MODBUS_GET_INT16_FROM_INT8(query, header_length + 7 + i * 2);
                printf("Current register value at %d: %d\n", address + i, mb_mapping->tab_registers[address + i]);
                mb_mapping->tab_registers[address + i] = value;
                printf("Updated register %d with value: %d\n", address + i, value);
            }
        }

        // Send response to client
        rc = modbus_reply(ctx, query, rc, mb_mapping);
        if (rc == -1) {
            printf("Error in sending reply: %s\n", modbus_strerror(errno));
            break;
        }
    }

    // Cleanup and shutdown
    if (use_backend == TCP || use_backend == TCP_PI) {
        if (socket_file_descriptor != -1) {
            close(socket_file_descriptor);
        }
    }
    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    modbus_close(substation_ctx);
    modbus_free(substation_ctx); 
    return 0;
}*/