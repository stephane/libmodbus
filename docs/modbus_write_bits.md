# modbus_write_bits

## Name

modbus_write_bits - write many bits

## Synopsis

```c
int modbus_write_bits(modbus_t *ctx, int addr, int nb, const uint8_t *src);
```

## Description

The *modbus_write_bits()* function shall write the status of the `nb` bits
(coils) from `src` at the address `addr` of the remote device. The
`src` array must contains bytes set to `TRUE` or `FALSE`.

The function uses the Modbus function code 0x0F (force multiple coils).

## Return value

The function shall return the number of written bits if successful. Otherwise it
shall return -1 and set errno.

## Errors

- *EMBMDATA*, writing too many bits.

## See also

- [modbus_read_bits](modbus_read_bits.md)
- [modbus_write_bit](modbus_write_bit.md)
