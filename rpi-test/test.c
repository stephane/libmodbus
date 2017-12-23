/*
 * Libmodbus RPi C template
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

#define SERVER_ID 1
#define UART_PORT "/dev/ttyAMA0"
#define BAUD_RATE 19200
#define PARITY 'N'
#define BYTESIZE 8
#define STOPBITS 1

#define BCM_PIN_DE 17
#define BCM_PIN_RE 18

#define REGISTER_ADDRESS 97
#define NO_OF_REGISTERS 1

int main(int argc, char *argv[])
{
    uint16_t *tab_rp_registers = NULL;
    modbus_t *ctx = NULL;
    uint32_t sec_to = 1;
    uint32_t usec_to = 0;
    int i;
    int rc;
    int nb_points = 1;
    ctx = modbus_new_rtu(UART_PORT, BAUD_RATE, PARITY, BYTESIZE, STOPBITS);
    if (ctx == NULL)
    {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }
    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);
    modbus_set_slave(ctx, SERVER_ID);
    modbus_get_response_timeout(ctx, &sec_to, &usec_to);
    modbus_enable_rpi(ctx,TRUE);
    modbus_configure_rpi_bcm_pins(ctx,BCM_PIN_DE,BCM_PIN_RE);
    modbus_rpi_pin_export_direction(ctx);
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    // modbus sample query - modify it for your own purpose
    tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));
    rc = modbus_read_registers(ctx, REGISTER_ADDRESS,NO_OF_REGISTERS, tab_rp_registers);
    printf("Date received is : %d\n",tab_rp_registers[0]);
    free(tab_rp_registers);

    /* Close the connection */
    modbus_rpi_pin_unexport_direction(ctx);
    modbus_close(ctx);
    modbus_free(ctx);
}
