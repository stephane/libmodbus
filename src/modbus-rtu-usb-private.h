/*
 * Copyright © 2023 Axel Gembe <axel@gembe.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef MODBUS_RTU_USB_PRIVATE_H
#define MODBUS_RTU_USB_PRIVATE_H

#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif

#include <libusb-1.0/libusb.h>

#define _MODBUS_RTU_USB_HEADER_LENGTH     1
#define _MODBUS_RTU_USB_PRESET_REQ_LENGTH 6
#define _MODBUS_RTU_USB_PRESET_RSP_LENGTH 2

static const uint8_t _MODBUS_RTU_USB_USB_EP_IN = (1 << 7);

static const uint8_t _MODBUS_RTU_USB_Default_RX_Report_Id = 0x90;
static const uint8_t _MODBUS_RTU_USB_Default_TX_Report_Id = 0x89;

static const int _MODBUS_USB_REPORT_SIZE = 64;
static const int _MODBUS_USB_PAYLOAD_SIZE = _MODBUS_USB_REPORT_SIZE - 1;

typedef struct _modbus_rtu_usb {
    /* Device selection callback */
    modbus_usb_device_selection_callback_t callback;
    /* Path: "/dev/input/bla" */
    char *path;
    /* Libusb device handle */
    struct libusb_device_handle *device_handle;
    /* USB endpoint */
    int endpoint;
    /* RTU USB report ids */
    uint8_t rx_report_id;
    uint8_t tx_report_id;
    /* To handle many slaves on the same link */
    int confirmation_to_ignore;
    /* USB Buffer */
    char usb_buffer[315]; /* 5 USB reports is the maximum size */
    int usb_buffer_start;
    int usb_buffer_end;
} modbus_rtu_usb_t;

#endif /* MODBUS_RTU_USB_PRIVATE_H */
