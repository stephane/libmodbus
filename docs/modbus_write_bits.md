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
`src` array must contain bytes set to `TRUE` or `FALSE`.

The `src` array must be allocated with at least `nb` elements. It is the
caller's responsibility to ensure the buffer is large enough to hold all the
bits to be written.

The function uses the Modbus function code 0x0F (force multiple coils).

## Return value

The function shall return the number of written bits if successful. Otherwise it
shall return -1 and set errno.

## Errors

- *EINVAL*, the `ctx` or `src` argument is NULL, or `nb` is less than 1.
- *EMBMDATA*, writing too many bits (nb > MODBUS_MAX_WRITE_BITS).

## See also

- [modbus_read_bits](modbus_read_bits.md)
- [modbus_write_bit](modbus_write_bit.md)
