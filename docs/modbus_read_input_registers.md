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

The `dest` array must be allocated with at least `nb * sizeof(uint16_t)` bytes.
It is the caller's responsibility to ensure the buffer is large enough to hold
all the registers to be read.

The function uses the Modbus function code 0x04 (read input registers). The
holding registers and input registers have different historical meaning, but
nowadays it's more common to use holding registers only.

## Return value

The function shall return the number of read input registers if
successful. Otherwise it shall return -1 and set errno.

## Errors

- *EINVAL*, the `ctx` or `dest` argument is NULL, or `nb` is less than 1.
- *EMBMDATA*, too many input registers requested (nb > MODBUS_MAX_READ_REGISTERS).

## See also

- [modbus_read_input_bits](modbus_read_input_bits.md)
- [modbus_write_register](modbus_write_register.md)
- [modbus_write_registers](modbus_write_registers.md)
