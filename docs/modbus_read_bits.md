# modbus_read_bits

## Name

modbus_read_bits - read many bits

## Synopsis

```c
int modbus_read_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
```

## Description

The *modbus_read_bits()* function shall read the status of the `nb` bits (coils)
to the address `addr` of the remote device. The result of reading is stored in
`dest` array as unsigned bytes (8 bits) set to `TRUE` or `FALSE`.

You must take care to allocate enough memory to store the results in `dest`
(at least `nb * sizeof(uint8_t)`).

The function uses the Modbus function code 0x01 (read coil status).

## Return value

The function shall return the number of read bits if successful. Otherwise it
shall return -1 and set errno.

## Errors

- *EMBMDATA*, too many bits requested

## See also

- [modbus_write_bit](modbus_write_bit.md)
- [modbus_write_bits](modbus_write_bits.md)
