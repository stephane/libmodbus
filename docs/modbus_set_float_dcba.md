# modbus_set_float_dcba

## Name

modbus_set_float_dcba - set a float value in 2 registers using DCBA byte order

## Synopsis

```c
void modbus_set_float_dcba(float f, uint16_t *dest);
```

## Description

The *modbus_set_float_dcba()* function shall set a float to 4 bytes in inverted
Modbus format (DCBA order). The `dest` array must be pointer on two 16 bits
values to be able to store the full result of the conversion.

## Return value

There is no return values.

## See also

- [modbus_get_float_dcba](modbus_get_float_dcba.md)
- [modbus_set_float](modbus_set_float.md)
- [modbus_get_float](modbus_get_float.md)
