# modbus_get_float_dcba

## Name

modbus_get_float_dcba - get a float value from 2 registers in DCBA byte order

## Synopsis

```c
float modbus_get_float_dcba(const uint16_t *src);
```

## Description

The *modbus_get_float_dcba()* function shall get a float from 4 bytes in
inverted Modbus format (DCBA order instead of ABCD). The `src` array must be a
pointer on two 16 bits values, for example, if the first word is set to 0x47F1
and the second to 0x2000, the float value will be read as 123456.0.

## Return value

The function shall return a float.

## See also

- [modbus_set_float_dcba](modbus_set_float_dcba.md)
- [modbus_get_float_abcd](modbus_get_float_abcd.md)
- [modbus_get_float_badc](modbus_get_float_badc.md)
- [modbus_get_float_cdab](modbus_get_float_cdab.md)
