# modbus_set_float_cdab

## Name

modbus_set_float_cdab - set a float value in 2 registers using CDAB byte order

## Synopsis

```c
void modbus_set_float_cdab(float f, uint16_t *dest);
```

## Description

The *modbus_set_float_cdab()* function shall set a float to 4 bytes in swapped
words Modbus format (CDAB order instead of ABCD). The `dest` array must be
pointer on two 16 bits values to be able to store the full result of the
conversion.

## Return value

There is no return values.

## See also

- [modbus_get_float_cdab](modbus_get_float_cdab.md)
- [modbus_set_float_abcd](modbus_set_float_abcd.md)
- [modbus_set_float_badc](modbus_set_float_badc.md)
- [modbus_set_float_dcba](modbus_set_float_dcba.md)
