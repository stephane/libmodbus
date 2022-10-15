# modbus_read_input_bits

## Name

modbus_read_input_bits - read many input bits

## Synopsis

```c
int modbus_read_input_bits(modbus_t *ctx, int addr, int nb, uint8_t *dest);
```

## Description

The *modbus_read_input_bits()* function shall read the content of the `nb` input
bits to the address `addr` of the remote device.  The result of reading is stored
in `dest` array as unsigned bytes (8 bits) set to `TRUE` or `FALSE`.

You must take care to allocate enough memory to store the results in `dest`
(at least `nb * sizeof(uint8_t)`).

The function uses the Modbus function code 0x02 (read input status).

## Return value

The function shall return the number of read input status if
successful. Otherwise it shall return -1 and set errno.

## Errors

- *EMBMDATA*, too many discrete inputs requested

## See also

- [modbus_read_input_registers](modbus_read_input_registers.md)
