/*
 * Copyright © 2023 Axel Gembe <axel@gembe.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MODBUS_RTU_USB_H
#define MODBUS_RTU_USB_H

#include "modbus.h"

MODBUS_BEGIN_DECLS

#define MODBUS_RTU_USB_MAX_ADU_LENGTH 256

typedef enum {
    /* see https://www.se.com/us/en/download/document/SPD_MPAO-98KJ7F_EN/ */
    MODBUS_USB_MODE_APC = 0,
} modbus_usb_modes;

typedef struct {
    char *vendor_str;
    char *product_str;
    char *serial_str;
    char *port_path_str;

    uint8_t bus;
    uint8_t bus_port;
    uint8_t port_path[7]; // Maximum number of USB hubs is 7
    size_t port_path_len;
    uint8_t device_address;

    uint16_t vid;
    uint16_t pid;
    uint16_t bcd_device;

    unsigned char *hid_report_descriptor_buf;
    size_t hid_report_descriptor_len;
} modbus_usb_device_t;

typedef int (*modbus_usb_device_selection_callback_t)(const modbus_usb_device_t *device);

MODBUS_API modbus_t *modbus_new_rtu_usb(modbus_usb_modes mode,
                                        modbus_usb_device_selection_callback_t callback);

MODBUS_API modbus_t *modbus_new_rtu_usb_from_path(modbus_usb_modes mode,
                                                  const char *path);

MODBUS_API int modbus_rtu_usb_build_path(uint8_t bus,
                                         uint8_t *port_numbers,
                                         size_t port_numbers_len,
                                         char *path_buffer,
                                         size_t path_buffer_size);

MODBUS_API int modbus_rtu_usb_set_path(modbus_t *ctx, const char *path);

MODBUS_API int
modbus_rtu_usb_set_callback(modbus_t *ctx,
                            modbus_usb_device_selection_callback_t callback);

MODBUS_API int modbus_rtu_usb_set_report_ids(modbus_t *ctx, uint8_t rx, uint8_t tx);

MODBUS_END_DECLS

#endif /* MODBUS_RTU_USB_H */
