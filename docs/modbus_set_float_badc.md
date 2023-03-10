# modbus_set_float_badc

## Name

modbus_set_float_badc - set a float value in 2 registers using BADC byte order

## Synopsis

```c
void modbus_set_float_badc(float f, uint16_t *dest);
```

## Description

The *modbus_set_float_badc()* function shall set a float to 4 bytes in swapped
bytes Modbus format (BADC instead of ABCD). The `dest` array must be pointer on
two 16 bits values to be able to store the full result of the conversion.

## Return value

There is no return values.

## See also

- [modbus_get_float_badc](modbus_get_float_badc.md)
- [modbus_set_float_abcd](modbus_set_float_abcd.md)
- [modbus_set_float_cdab](modbus_set_float_cdab.md)
- [modbus_set_float_dcba](modbus_set_float_dcba.md)
