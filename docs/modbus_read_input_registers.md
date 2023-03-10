# modbus_read_input_registers

## Name

modbus_read_input_registers - read many input registers

## Synopsis

```c
int modbus_read_input_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
```

## Description

The *modbus_read_input_registers()* function shall read the content of the `nb`
input registers to address `addr` of the remote device. The result of the
reading is stored in `dest` array as word values (16 bits).

You must take care to allocate enough memory to store the results in `dest` (at
least `nb * sizeof(uint16_t)`).

The function uses the Modbus function code 0x04 (read input registers). The
holding registers and input registers have different historical meaning, but
nowadays it's more common to use holding registers only.

## Return value

The function shall return the number of read input registers if
successful. Otherwise it shall return -1 and set errno.

## Errors

- *EMBMDATA*, too many bits requested.

## See also

- [modbus_read_input_bits](modbus_read_input_bits.md)
- [modbus_write_register](modbus_write_register.md)
- [modbus_write_registers](modbus_write_registers.md)
