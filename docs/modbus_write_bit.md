# modbus_write_bit

## Name

modbus_write_bit - write a single bit

## Synopsis

```c
int modbus_write_bit(modbus_t *ctx, int addr, int status);
```

## Description

The *modbus_write_bit()* function shall write the status of `status` at the
address `addr` of the remote device. The value must be set to `TRUE` or `FALSE`.

The function uses the Modbus function code 0x05 (force single coil).

## Return value

The function shall return 1 if successful. Otherwise it shall return -1 and set
errno.

## See also

- [modbus_read_bits](modbus_read_bits.md)
- [modbus_write_bits](modbus_write_bits.md)
