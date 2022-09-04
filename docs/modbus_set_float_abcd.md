# modbus_set_float_abcd

## Name

modbus_set_float_abcd - set a float value in 2 registers using ABCD byte order

## Synopsis

```c
void modbus_set_float_abcd(float f, uint16_t *dest);
```

## Description

The *modbus_set_float_abcd()* function shall set a float to 4 bytes in usual
Modbus format. The `dest` array must be pointer on two 16 bits values to be able
to store the full result of the conversion.

## Return value

There is no return values.

## See also

- [modbus_get_float_abcd](modbus_get_float_abcd.md)
- [modbus_set_float_badc](modbus_set_float_badc.md)
- [modbus_set_float_cdab](modbus_set_float_cdab.md)
- [modbus_set_float_dcba](modbus_set_float_dcba.md)
