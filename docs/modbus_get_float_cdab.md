# modbus_get_float_cdab

## Name

modbus_get_float_cdab - get a float value from 2 registers in CDAB byte order

## Synopsis

```c
float modbus_get_float_cdab(const uint16_t *src);
```

## Description

The *modbus_get_float_cdab()* function shall get a float from 4 bytes with
swapped words (CDAB order instead of ABCD). The `src` array must be a pointer on
two 16 bits values, for example, if the first word is set to F147 and the second
to 0x0020, the float value will be read as 123456.0.

## Return value

The function shall return a float.

## See also

- [modbus_set_float_cdab](modbus_set_float_cdab.md)
- [modbus_get_float_abcd](modbus_get_float_abcd.md)
- [modbus_get_float_badc](modbus_get_float_badc.md)
- [modbus_get_float_dcba](modbus_get_float_dcba.md)
