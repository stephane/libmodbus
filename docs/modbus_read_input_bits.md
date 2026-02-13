# modbus_read_input_bits

## Name

modbus_read_input_bits - read many input bits

## Synopsis

```c
int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
```

## Description

The *modbus_read_input_bits()* function shall read the content of the `nb` input
bits to the address `addr` of the remote device. The result of reading is stored
in `dest` array as unsigned bytes (8 bits) set to `TRUE` or `FALSE`.

The `dest` array must be allocated with at least `nb * sizeof(uint8_t)` bytes.
It is the caller's responsibility to ensure the buffer is large enough to hold
all the bits to be read.

The function uses the Modbus function code 0x02 (read input status).

## Return value

The function shall return the number of read input status if
successful. Otherwise it shall return -1 and set errno.

## Errors

- *EINVAL*, the `ctx` or `dest` argument is NULL, or `nb` is less than 1.
- *EMBXILVAL*, too many discrete inputs requested (nb > MODBUS_MAX_READ_BITS).

## See also

- [modbus_read_input_registers](modbus_read_input_registers.md)
