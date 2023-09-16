/*
 * Copyright © 2023 Axel Gembe <axel@gembe.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <modbus.h>

static void _swap_bytes(uint16_t* array, size_t length) {
    for (size_t i = 0; i < length; i++) {
        array[i] = (array[i] >> 8) | (array[i] << 8);
    }
}

static int _read_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest)
{
    if (modbus_read_registers(ctx, addr, nb, dest) > 0) {
        usleep(35000); // 4.2.2 Modbus Message RTU Framing, interframe delay
        return 1;
    } else {
        fprintf(stderr, "Register read of %d:%d failed: %s\n", addr, addr + nb, modbus_strerror(errno));
        return 0;
    }
}

static void _print_register_int(uint16_t *reg, const char *name, float scale)
{
    printf("%s = %u (%f scaled)\n", name, reg[0], reg[0] / scale);
}

static void _print_register_str(uint16_t *reg, const char *name, int nb)
{
    _swap_bytes(reg, nb);
    printf("%s = \"%s\"\n", name, (char*)reg);
}

static int _match_callback(const modbus_usb_device_t *device)
{
    if (device->vid == 0x51d) {
        return 0;
    }

    return -1;
}

int main(void)
{
    modbus_t *ctx;
    uint16_t register_buf[16];

    ctx = modbus_new_rtu_usb(MODBUS_USB_MODE_APC, _match_callback);

    /* From MPAO-98KJ7F-R1-EN:
     * The slave address of the APC device will default to 1 unless changed
     * via some other mechanism. */
    modbus_set_slave(ctx, 1);

    modbus_set_debug(ctx, 1);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    if (_read_registers(ctx, 151, 1, register_buf)) {
        _print_register_int(register_buf, "REG_INPUT_0_VOLTAGE", 6.0f);
    }

    if (_read_registers(ctx, 140, 1, register_buf)) {
        _print_register_int(register_buf, "REG_OUTPUT_0_CURRENT", 5.0f);
    }

    if (_read_registers(ctx, 564, 8, register_buf)) {
        _print_register_str(register_buf, "REG_SERIAL_NUMBER", 8);
    }

    if (_read_registers(ctx, 532, 16, register_buf)) {
        _print_register_str(register_buf, "REG_MODEL", 16);
    }

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
