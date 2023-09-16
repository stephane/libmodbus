/*
 * Copyright © 2023 Axel Gembe <axel@gembe.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include "modbus-private.h"
#include <assert.h>

#include "modbus-rtu-usb-private.h"
#include "modbus-rtu-usb.h"

/*
 * https://download.schneider-electric.com/files?p_enDocType=Application+Notes&p_File_Name=MPAO-98KJ7F_R1_EN.pdf&p_Doc_Ref=SPD_MPAO-98KJ7F_EN
 */

libusb_context *usb_ctx = NULL;
size_t usb_ctx_refcnt = 0;

static int _usb_error_to_errno(int libusb_error_code)
{
    /* No errno for things that aren't an error code */
    if (libusb_error_code >= 0)
        return 0;

    switch (libusb_error_code) {
    case LIBUSB_ERROR_IO:
        return EIO;
    case LIBUSB_ERROR_INVALID_PARAM:
        return EINVAL;
    case LIBUSB_ERROR_ACCESS:
        return EACCES;
    case LIBUSB_ERROR_NO_DEVICE:
        return ENODEV;
    case LIBUSB_ERROR_NOT_FOUND:
        return ENOENT;
    case LIBUSB_ERROR_BUSY:
        return EBUSY;
    case LIBUSB_ERROR_TIMEOUT:
        return ETIMEDOUT;
    case LIBUSB_ERROR_OVERFLOW:
        return EOVERFLOW;
    case LIBUSB_ERROR_PIPE:
        return EPIPE;
    case LIBUSB_ERROR_INTERRUPTED:
        return EINTR;
    case LIBUSB_ERROR_NO_MEM:
        return ENOMEM;
    case LIBUSB_ERROR_NOT_SUPPORTED:
        return ENOTSUP;
    case LIBUSB_ERROR_OTHER:
        return ENOMSG;
    default:
        return ENOMSG;
    }
}

static int _usb_init(void)
{
    int res;

    if (usb_ctx_refcnt > 0) {
        return 0;
    }

    if ((res = libusb_init(&usb_ctx)) != LIBUSB_SUCCESS) {
        errno = _usb_error_to_errno(res);
        return -1;
    }

    usb_ctx_refcnt++;

    return 0;
}

static void _usb_exit(void)
{
    if (usb_ctx_refcnt == 0) {
        return;
    }

    libusb_exit(usb_ctx);
    usb_ctx = NULL;
    usb_ctx_refcnt--;
}

/* Define the slave ID of the remote device to talk in master mode or set the
 * internal slave ID in slave mode */
static int _modbus_rtu_usb_set_slave(modbus_t *ctx, int slave)
{
    int max_slave = (ctx->quirks & MODBUS_QUIRK_MAX_SLAVE) ? 255 : 247;

    /* Broadcast address is 0 (MODBUS_BROADCAST_ADDRESS) */
    if (slave >= 0 && slave <= max_slave) {
        ctx->slave = slave;
    } else {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

/* Builds a RTU request header */
static int _modbus_rtu_usb_build_request_basis(
    modbus_t *ctx, int function, int addr, int nb, uint8_t *req)
{
    assert(ctx->slave != -1);
    req[0] = ctx->slave;
    req[1] = function;
    req[2] = addr >> 8;
    req[3] = addr & 0x00ff;
    req[4] = nb >> 8;
    req[5] = nb & 0x00ff;

    return _MODBUS_RTU_USB_PRESET_REQ_LENGTH;
}

/* Builds a RTU response header */
static int _modbus_rtu_usb_build_response_basis(sft_t *sft, uint8_t *rsp)
{
    /* In this case, the slave is certainly valid because a check is already
     * done in _modbus_rtu_listen */
    rsp[0] = sft->slave;
    rsp[1] = sft->function;

    return _MODBUS_RTU_USB_PRESET_RSP_LENGTH;
}

static int _modbus_rtu_usb_prepare_response_tid(const uint8_t *req, int *req_length)
{
    /* No TID */
    return 0;
}

static int _modbus_rtu_usb_send_msg_pre(uint8_t *req, int req_length)
{
    return req_length;
}

static ssize_t _modbus_rtu_usb_send(modbus_t *ctx, const uint8_t *req, int req_length)
{
    uint8_t usb_report[_MODBUS_USB_REPORT_SIZE];
    int to_transfer, transferred, total_transferred, total_remaining, r;
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    if (ctx_rtu_usb->device_handle == NULL) {
        errno = EINVAL;
        return -1;
    }

    total_transferred = 0;
    total_remaining = req_length;
    while (total_remaining > 0) {
        to_transfer = total_remaining % _MODBUS_USB_PAYLOAD_SIZE;
        usb_report[0] = ctx_rtu_usb->rx_report_id;
        memcpy(usb_report + 1, req, to_transfer);
        memset(usb_report + 1 + to_transfer, 0, sizeof(usb_report) - 1 - to_transfer);

        r = libusb_interrupt_transfer(ctx_rtu_usb->device_handle,
                                      LIBUSB_ENDPOINT_OUT | ctx_rtu_usb->endpoint,
                                      usb_report,
                                      sizeof(usb_report),
                                      &transferred,
                                      0);
        if (r != LIBUSB_SUCCESS) {
            errno = _usb_error_to_errno(r);
            return -1;
        }

        total_remaining -= to_transfer;
        total_transferred += to_transfer;

        if (transferred < to_transfer + 1) {
            break;
        }
    }

    return total_transferred;
}

static int _modbus_rtu_usb_receive(modbus_t *ctx, uint8_t *req)
{
    int rc;
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    if (ctx_rtu_usb->device_handle == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (ctx_rtu_usb->confirmation_to_ignore) {
        _modbus_receive_msg(ctx, req, MSG_CONFIRMATION);
        /* Ignore errors and reset the flag */
        ctx_rtu_usb->confirmation_to_ignore = FALSE;
        rc = 0;
        if (ctx->debug) {
            printf("Confirmation to ignore\n");
        }
    } else {
        rc = _modbus_receive_msg(ctx, req, MSG_INDICATION);
        if (rc == 0) {
            /* The next expected message is a confirmation to ignore */
            ctx_rtu_usb->confirmation_to_ignore = TRUE;
        }
    }
    return rc;
}

static ssize_t _modbus_rtu_usb_recv_more(modbus_t *ctx, unsigned int timeout_msecs)
{
    uint8_t usb_report[_MODBUS_USB_REPORT_SIZE];
    int transferred, r;
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    if (ctx_rtu_usb->device_handle == NULL) {
        errno = EINVAL;
        return -1;
    }

    while (1) {
        r = libusb_interrupt_transfer(ctx_rtu_usb->device_handle,
                                      LIBUSB_ENDPOINT_IN | ctx_rtu_usb->endpoint,
                                      usb_report,
                                      sizeof(usb_report),
                                      &transferred,
                                      timeout_msecs);
        if (r != LIBUSB_SUCCESS) {
            errno = _usb_error_to_errno(r);
            return -1;
        }

        /* We are only interested in MODBUS Tx reports*/
        if (usb_report[0] != ctx_rtu_usb->tx_report_id) {
            continue;
        }

        break;
    }

    memcpy(ctx_rtu_usb->usb_buffer + ctx_rtu_usb->usb_buffer_end,
           usb_report + 1,
           sizeof(usb_report) - 1);
    ctx_rtu_usb->usb_buffer_end += sizeof(usb_report) - 1;

    return sizeof(usb_report) - 1;
}

static ssize_t _modbus_rtu_usb_recv(modbus_t *ctx, uint8_t *rsp, int rsp_length)
{
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    if (ctx_rtu_usb->device_handle == NULL) {
        errno = EINVAL;
        return -1;
    }

    while ((ctx_rtu_usb->usb_buffer_end - ctx_rtu_usb->usb_buffer_start) < rsp_length) {
        if (_modbus_rtu_usb_recv_more(ctx, 0) <= 0) {
            return -1;
        }
    }

    memcpy(rsp, ctx_rtu_usb->usb_buffer + ctx_rtu_usb->usb_buffer_start, rsp_length);
    ctx_rtu_usb->usb_buffer_start += rsp_length;
    return rsp_length;
}

static int _modbus_rtu_usb_pre_check_confirmation(modbus_t *ctx,
                                                  const uint8_t *req,
                                                  const uint8_t *rsp,
                                                  int rsp_length)
{
    /* Check responding slave is the slave we requested (except for broacast
     * request) */
    if (req[0] != rsp[0] && req[0] != MODBUS_BROADCAST_ADDRESS) {
        if (ctx->debug) {
            fprintf(stderr,
                    "The responding slave %d isn't the requested slave %d\n",
                    rsp[0],
                    req[0]);
        }
        errno = EMBBADSLAVE;
        return -1;
    } else {
        return 0;
    }
}

static void _modbus_rtu_usb_clear_buffers(modbus_t *ctx)
{
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    ctx_rtu_usb->usb_buffer_start = 0;
    ctx_rtu_usb->usb_buffer_end = 0;
    memset(ctx_rtu_usb->usb_buffer, 0, sizeof(ctx_rtu_usb->usb_buffer));
}

static int
_modbus_rtu_usb_check_integrity(modbus_t *ctx, uint8_t *msg, const int msg_length)
{
    _modbus_rtu_usb_clear_buffers(ctx);
    return msg_length;
}

#pragma pack(1)

typedef struct {
    uint8_t bDescriptorType;
    uint16_t wDescriptorLength;
} hid_descriptor_extra_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    hid_descriptor_extra_t extra[1];
} uhd_hid_descriptor_t;

#pragma pack()

static int _usb_get_hid_descriptor(modbus_t *ctx,
                                   libusb_device_handle *dev_handle,
                                   unsigned char *hid_rd_buf,
                                   size_t hid_rd_buf_size)
{
    int r, iface_idx, alt_idx;
    struct libusb_config_descriptor *conf_desc;
    const struct libusb_interface *iface;
    const struct libusb_interface_descriptor *alt_iface_desc;

    if ((r = libusb_get_active_config_descriptor(libusb_get_device(dev_handle),
                                                 &conf_desc)) != LIBUSB_SUCCESS) {
        return r;
    }

    for (iface_idx = 0; iface_idx < conf_desc->bNumInterfaces; iface_idx++) {
        iface = &conf_desc->interface[iface_idx];

        for (alt_idx = 0; alt_idx < iface->num_altsetting; alt_idx++) {
            alt_iface_desc = &iface->altsetting[alt_idx];

            if (alt_iface_desc->bInterfaceClass != LIBUSB_CLASS_HID) {
                continue;
            }

            r = libusb_kernel_driver_active(dev_handle, alt_iface_desc->bInterfaceNumber);
            if (r == 1) {
                r = libusb_detach_kernel_driver(dev_handle,
                                                alt_iface_desc->bInterfaceNumber);
                if (r != LIBUSB_SUCCESS) {
                    return r;
                }
            }

            r = libusb_claim_interface(dev_handle, alt_iface_desc->bInterfaceNumber);
            if (r != LIBUSB_SUCCESS) {
                return r;
            }

            r = libusb_control_transfer(
                dev_handle,
                LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_STANDARD |
                    LIBUSB_RECIPIENT_INTERFACE,
                LIBUSB_REQUEST_GET_DESCRIPTOR,
                (LIBUSB_DT_REPORT << 8) | alt_iface_desc->bInterfaceNumber,
                0,
                hid_rd_buf,
                hid_rd_buf_size,
                5000);

            libusb_release_interface(dev_handle, alt_iface_desc->bInterfaceNumber);

            return r;
        }
    }

    return -1;
}

static int _modbus_rtu_usb_connect(modbus_t *ctx)
{
    libusb_device **devs, *d;
    struct libusb_device_descriptor dev_desc;
    libusb_device_handle *dev_handle;
    ssize_t devs_len, i;
    int r, is_match;
    char path_buffer[256], vendor_buffer[256], product_buffer[256], serial_buffer[256];
    unsigned char hid_report_descriptor[2048];
    modbus_usb_device_t ud;
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    if (ctx_rtu_usb->device_handle != NULL) {
        if (ctx->debug) {
            printf("%s: Already connected\n", __func__);
        }
        return 0;
    }

    _usb_init();

    devs_len = libusb_get_device_list(NULL, &devs);
    if (devs_len < 0) {
        fprintf(stderr, "libusb_get_device_list failed: %s\n", libusb_strerror(devs_len));
        _usb_exit();
        errno = _usb_error_to_errno(devs_len);
        return -1;
    }

    if (ctx->debug) {
        printf("Number of USB devices: %ld\n", devs_len);
    }

    for (i = 0; i < devs_len; i++) {
        d = devs[i];

        /* Root ports have no parent, we can skip them */
        if (libusb_get_parent(d) == NULL) {
            continue;
        }

        r = libusb_get_device_descriptor(d, &dev_desc);
        if (r != LIBUSB_SUCCESS) {
            if (ctx->debug) {
                fprintf(stderr,
                        "libusb_get_device_descriptor for device #%ld failed: %s\n",
                        i,
                        libusb_strerror(r));
            }
            continue;
        }

        if (ctx->debug) {
            printf("Considering device #%ld (%04x:%04x)\n",
                   i,
                   dev_desc.idVendor,
                   dev_desc.idProduct);
        }

        memset(&ud, 0, sizeof(ud));

        memset(&path_buffer, 0, sizeof(path_buffer));
        ud.port_path_str = path_buffer;

        ud.vid = dev_desc.idVendor;
        ud.pid = dev_desc.idProduct;
        ud.bcd_device = dev_desc.bcdDevice;

        ud.bus = libusb_get_bus_number(d);
        ud.bus_port = libusb_get_port_number(d);
        r = libusb_get_port_numbers(d, ud.port_path, sizeof(ud.port_path));
        if (r < 1) {
            if (ctx->debug) {
                fprintf(stderr, "libusb_get_port_numbers for device #%ld failed\n", i);
            }
            continue;
        }
        ud.port_path_len = r;
        ud.device_address = libusb_get_device_address(d);

        assert(modbus_rtu_usb_build_path(ud.bus,
                                         ud.port_path,
                                         ud.port_path_len,
                                         path_buffer,
                                         sizeof(path_buffer)) == 0);

        if ((r = libusb_open(d, &dev_handle)) != LIBUSB_SUCCESS) {
            if (ctx->debug) {
                fprintf(stderr,
                        "libusb_open for device #%ld failed: %s\n",
                        i,
                        libusb_strerror(r));
            }
            continue;
        }

        if (dev_desc.iManufacturer) {
            memset(&vendor_buffer, 0, sizeof(vendor_buffer));
            r = libusb_get_string_descriptor_ascii(dev_handle,
                                                   dev_desc.iManufacturer,
                                                   (unsigned char *) vendor_buffer,
                                                   sizeof(vendor_buffer));
            if (r > 0) {
                ud.vendor_str = vendor_buffer;
            }
        }

        if (dev_desc.iProduct) {
            memset(&product_buffer, 0, sizeof(product_buffer));
            r = libusb_get_string_descriptor_ascii(dev_handle,
                                                   dev_desc.iProduct,
                                                   (unsigned char *) product_buffer,
                                                   sizeof(product_buffer));
            if (r > 0) {
                ud.product_str = product_buffer;
            }
        }

        if (dev_desc.iSerialNumber) {
            memset(&serial_buffer, 0, sizeof(serial_buffer));
            r = libusb_get_string_descriptor_ascii(dev_handle,
                                                   dev_desc.iSerialNumber,
                                                   (unsigned char *) serial_buffer,
                                                   sizeof(serial_buffer));
            if (r > 0) {
                ud.serial_str = serial_buffer;
            }
        }

        r = _usb_get_hid_descriptor(
            ctx, dev_handle, hid_report_descriptor, sizeof(hid_report_descriptor));
        if (r >= 0) {
            ud.hid_report_descriptor_buf = hid_report_descriptor;
            ud.hid_report_descriptor_len = (size_t) r;
        } else {
            if (ctx->debug) {
                fprintf(stderr,
                        "%s: failed to get HID descriptor: %s\n",
                        __func__,
                        libusb_strerror(r));
            }
        }

        is_match = 0;
        if (ctx_rtu_usb->callback != NULL && ctx_rtu_usb->callback(&ud) == 0) {
            is_match = 1;
        } else if (ctx_rtu_usb->path && strcmp(ctx_rtu_usb->path, path_buffer) == 0) {
            is_match = 1;
        }

        if (is_match) {
            if (ctx->debug) {
                printf("Found Device %ld (Path %s):\n", i, path_buffer);
                printf("  Vendor ID: 0x%04x\n", ud.vid);
                printf("  Product ID: 0x%04x\n", ud.pid);
            }

            ctx_rtu_usb->device_handle = dev_handle;

            break;
        }

        libusb_close(dev_handle);
    }

    if (i >= devs_len) {
        if (ctx->debug) {
            fprintf(stderr, "No matching device found\n");
        }
        libusb_free_device_list(devs, 1);
        _usb_exit();
        errno = ENODEV;
        return -1;
    }

    libusb_free_device_list(devs, 1);

    return 0;
}

static unsigned int _modbus_rtu_usb_is_connected(modbus_t *ctx)
{
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;
    return ctx_rtu_usb->device_handle != NULL;
}

static void _modbus_rtu_usb_close(modbus_t *ctx)
{
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    if (ctx_rtu_usb->device_handle != NULL) {
        libusb_close(ctx_rtu_usb->device_handle);
        ctx_rtu_usb->device_handle = NULL;
        _usb_exit();
    }
}

static int _modbus_rtu_usb_flush(modbus_t *ctx)
{
    int rc;
    int rc_sum = 0;

    for (;;) {
        rc = _modbus_rtu_usb_recv_more(ctx, 10);

        if (rc < 0) {
            if (errno == ETIMEDOUT)
                break;

            _modbus_rtu_usb_clear_buffers(ctx);
            return -1;
        }

        rc_sum += rc;
    }

    _modbus_rtu_usb_clear_buffers(ctx);
    return rc_sum;
}

static int _modbus_rtu_usb_select(modbus_t *ctx,
                                  fd_set *rset,
                                  struct timeval *tv,
                                  int length_to_read)
{
    unsigned int timeout_msecs;
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    if (ctx_rtu_usb->device_handle == NULL) {
        errno = EINVAL;
        return -1;
    }

    timeout_msecs = (tv->tv_sec * 1000) + (tv->tv_usec / 1000);

    while ((ctx_rtu_usb->usb_buffer_end - ctx_rtu_usb->usb_buffer_start) <
           length_to_read) {
        if (_modbus_rtu_usb_recv_more(ctx, timeout_msecs) <= 0) {
            return -1;
        }
    }

    return 0;
}

static void _modbus_rtu_usb_free(modbus_t *ctx)
{
    if (ctx->backend_data) {
        _modbus_rtu_usb_close(ctx);
        free(ctx->backend_data);
    }

    free(ctx);
}

// clang-format off
const modbus_backend_t _modbus_rtu_usb_backend = {
    _MODBUS_BACKEND_TYPE_RTU_USB,
    _MODBUS_RTU_USB_HEADER_LENGTH,
    0,
    MODBUS_RTU_USB_MAX_ADU_LENGTH,
    _modbus_rtu_usb_set_slave,
    _modbus_rtu_usb_build_request_basis,
    _modbus_rtu_usb_build_response_basis,
    _modbus_rtu_usb_prepare_response_tid,
    _modbus_rtu_usb_send_msg_pre,
    _modbus_rtu_usb_send,
    _modbus_rtu_usb_receive,
    _modbus_rtu_usb_recv,
    _modbus_rtu_usb_check_integrity,
    _modbus_rtu_usb_pre_check_confirmation,
    _modbus_rtu_usb_connect,
    _modbus_rtu_usb_is_connected,
    _modbus_rtu_usb_close,
    _modbus_rtu_usb_flush,
    _modbus_rtu_usb_select,
    _modbus_rtu_usb_free
};

// clang-format on

static modbus_t *_modbus_new_rtu_usb_common(modbus_usb_modes mode)
{
    modbus_t *ctx;
    modbus_rtu_usb_t *ctx_rtu_usb;

    ctx = (modbus_t *) malloc(sizeof(modbus_t));
    if (ctx == NULL) {
        return NULL;
    }

    _modbus_init_common(ctx);
    ctx->backend = &_modbus_rtu_usb_backend;
    ctx->backend_data = (modbus_rtu_usb_t *) malloc(sizeof(modbus_rtu_usb_t));
    if (ctx->backend_data == NULL) {
        modbus_free(ctx);
        errno = ENOMEM;
        return NULL;
    }
    ctx_rtu_usb = (modbus_rtu_usb_t *) ctx->backend_data;

    memset(ctx_rtu_usb, 0, sizeof(modbus_rtu_usb_t));

    ctx_rtu_usb->endpoint = 1;
    ctx_rtu_usb->confirmation_to_ignore = FALSE;

    _modbus_rtu_usb_clear_buffers(ctx);

    modbus_rtu_usb_set_report_ids(
        ctx, _MODBUS_RTU_USB_Default_RX_Report_Id, _MODBUS_RTU_USB_Default_TX_Report_Id);

    return ctx;
}

modbus_t *modbus_new_rtu_usb_from_path(modbus_usb_modes mode, const char *path)
{
    modbus_t *ctx;

    if ((ctx = _modbus_new_rtu_usb_common(mode)) == NULL) {
        return NULL;
    }

    if (modbus_rtu_usb_set_path(ctx, path) != 0) {
        modbus_free(ctx);
        return NULL;
    }

    return ctx;
}

modbus_t *modbus_new_rtu_usb(modbus_usb_modes mode,
                             modbus_usb_device_selection_callback_t callback)
{
    modbus_t *ctx;

    if ((ctx = _modbus_new_rtu_usb_common(mode)) == NULL) {
        return NULL;
    }

    if (modbus_rtu_usb_set_callback(ctx, callback) != 0) {
        modbus_free(ctx);
        return NULL;
    }

    return ctx;
}

int modbus_rtu_usb_build_path(uint8_t bus,
                              uint8_t *port_numbers,
                              size_t port_numbers_len,
                              char *path_buffer,
                              size_t path_buffer_size)
{
    int r;
    size_t port_idx;
    size_t path_offset = 0;

    r = snprintf(path_buffer, path_buffer_size, "%u-", bus);
    if (r < 0 || (size_t) r >= path_buffer_size) {
        errno = EOVERFLOW;
        return -1;
    }

    path_offset += r;

    for (port_idx = 0; port_idx < port_numbers_len; port_idx++) {
        r = snprintf(path_buffer + path_offset,
                     path_buffer_size - path_offset,
                     port_idx < (port_numbers_len - 1) ? "%u." : "%u",
                     port_numbers[port_idx]);
        if (r < 0 || (size_t) r >= path_buffer_size - path_offset) {
            errno = EOVERFLOW;
            return -1;
        }

        path_offset += r;
    }

    return 0;
}

int modbus_rtu_usb_set_path(modbus_t *ctx, const char *path)
{
    modbus_rtu_usb_t *ctx_rtu_usb;
    size_t path_len;

    /* Check arguments */
    if (path == NULL || *path == 0) {
        if (ctx->debug) {
            fprintf(stderr, "The path string is empty\n");
        }
        errno = EINVAL;
        return -1;
    }

    ctx_rtu_usb = ctx->backend_data;

    if (ctx_rtu_usb->path) {
        free(ctx_rtu_usb->path);
        ctx_rtu_usb->path = NULL;
    }

    path_len = strlen(path) + 1;

    /* Device name and \0 */
    ctx_rtu_usb->path = (char *) malloc(path_len);
    if (ctx_rtu_usb->path == NULL) {
        modbus_free(ctx);
        errno = ENOMEM;
        return -1;
    }

#if defined(_WIN32)
    strcpy_s(ctx_rtu_usb->path, path_len, path);
#else
    strcpy(ctx_rtu_usb->path, path);
#endif

    return 0;
}

int modbus_rtu_usb_set_callback(modbus_t *ctx,
                                modbus_usb_device_selection_callback_t callback)
{
    modbus_rtu_usb_t *ctx_rtu_usb;

    /* Check arguments */
    if (callback == NULL) {
        if (ctx->debug) {
            fprintf(stderr, "The callback is NULL\n");
        }
        errno = EINVAL;
        return -1;
    }

    ctx_rtu_usb = ctx->backend_data;
    ctx_rtu_usb->callback = callback;

    return 0;
}

int modbus_rtu_usb_set_report_ids(modbus_t *ctx, uint8_t rx, uint8_t tx)
{
    modbus_rtu_usb_t *ctx_rtu_usb = ctx->backend_data;

    ctx_rtu_usb->rx_report_id = rx;
    ctx_rtu_usb->tx_report_id = tx;

    return 0;
}
