# modbus_write_registers

## Name

modbus_write_registers - write many registers

## Synopsis

```c
int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *src);
```

## Description

The *modbus_write_registers()* function shall write the content of the `nb`
holding registers from the array `src` at address `addr` of the remote device.

The function uses the Modbus function code 0x10 (preset multiple registers).

## Return value

The function shall return the number of written registers if
successful. Otherwise it shall return -1 and set errno.

## See also

- [modbus_write_register](modbus_write_register.md)
- [modbus_read_registers](modbus_read_registers.md)
