# modbus_rtu_set_serial_mode

## Name

modbus_rtu_set_serial_mode - set the serial mode

## Synopsis

```c
int modbus_rtu_set_serial_mode(modbus_t *ctx, int mode);
```

## Description

The *modbus_rtu_set_serial_mode()* function shall set the selected serial
mode:

- **MODBUS_RTU_RS232**, the serial line is set for RS232 communication. RS-232
  (Recommended Standard 232) is the traditional name for a series of standards
  for serial binary single-ended data and control signals connecting between a
  DTE (Data Terminal Equipment) and a DCE (Data Circuit-terminating Equipment).
  It is commonly used in computer serial ports.

- **MODBUS_RTU_RS485**, the serial line is set for RS485 communication.
EIA-485, also known as TIA/EIA-485 or RS-485, is a standard defining the
electrical characteristics of drivers and receivers for use in balanced
digital multipoint systems. This standard is widely used for communications
in industrial automation because it can be used effectively over long
distances and in electrically noisy environments.

This function is only supported on Linux kernels 2.6.28 onwards.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno to one of the values defined below.

## Errors

- *EINVAL*, the current libmodbus backend is not RTU.
- *ENOTSUP*, the function is not supported on your platform.

If the call to `ioctl()` fails, the error code of ioctl will be returned.
