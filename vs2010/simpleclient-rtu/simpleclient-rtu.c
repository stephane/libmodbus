/*
 * Copyright © 2011 Christian Leutloff <leutloff@sundancer.oche.de>
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
/*
This is small client with hard coded values to read a single register
value from a Modbus RTU connected device. 
It is derived from the unit-test-client from Stéphane.

It was successfully used on a Win 7, SP1, 64 bit System. Though the 
executable still was a 32 bit one.
*/

#include <config.h>

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>

// Serial interface 
const char *SERIALPORT_DEVICE_NAME = "COM11";
const int SERIALPORT_BAUD = 38400; 
const char SERIALPORT_PARITY = 'O'; // one of 'N', 'O', 'E'
const int SERIALPORT_DATA_BITS = 8;
const int SERIALPORT_STOP_BIT = 1;

// used device
const int SLAVE_ID = 65; // modbus id
const int READ_REGISTER_ADDRESS = 2056;
const int READ_REGISTER_EXPECTED_VALUE = 1635;
const uint16_t UT_INPUT_REGISTERS_NB = 0x1;

int main(int argc, char *argv[])
{
    int nb_points = UT_INPUT_REGISTERS_NB;
    uint16_t *tab_rp_registers = NULL;
    int rc = -1;

    modbus_t *ctx = modbus_new_rtu(SERIALPORT_DEVICE_NAME, SERIALPORT_BAUD, 
        SERIALPORT_PARITY, SERIALPORT_DATA_BITS, SERIALPORT_STOP_BIT);
    if (ctx == NULL)
    {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }
    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_LINK_AND_PROTOCOL);

    modbus_set_slave(ctx, SLAVE_ID);
    if (modbus_connect(ctx) == -1) 
    {
        fprintf(stderr, "Connection failed: %s\n",modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    
    /* Allocate and initialize the memory to store the registers */
    tab_rp_registers = (uint16_t *)malloc(nb_points * sizeof(uint16_t));
    if (NULL == tab_rp_registers) { goto close; }
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));


    // read single register (function 4)
    rc = modbus_read_input_registers(ctx, READ_REGISTER_ADDRESS, 1, tab_rp_registers);
    printf("modbus_read_input_registers: ");
    if (rc != 1)
    {
        printf("FAILED (nb points %d)\n", rc);
        goto close;
    }
    if (tab_rp_registers[0] != READ_REGISTER_EXPECTED_VALUE)// the expected value to read is READ_REGISTER_EXPECTED_VALUE/1635
    {
        printf("FAILED (%d/0x%0X != %d/0x%0X)\n", tab_rp_registers[0], tab_rp_registers[0], READ_REGISTER_EXPECTED_VALUE, READ_REGISTER_EXPECTED_VALUE);
        goto close;
    }
    else
    {
        printf("received: 0x%0X (%d)\n", tab_rp_registers[0], tab_rp_registers[0]);
    }

close:
    /* Free the memory */
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
