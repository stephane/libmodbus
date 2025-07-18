# modbus_get_float

## Name

modbus_get_float - get a float value from 2 registers

## Synopsis

```c
float modbus_get_float(const uint16_t *src);
```

Warning, this function is *deprecated* since libmodbus v3.2.0 and has been
replaced by *modbus_get_float_dcba()*.

## Description

The *modbus_get_float()* function shall get a float from 4 bytes in Modbus
format (DCBA byte order). The `src` array must be a pointer on two 16 bits
values, for example, if the first word is set to 0x4465 and the second to
0x229a, the float value will be 916.540649.

## Return value

The function shall return a float.

## See also

- [modbus_set_float](modbus_set_float.md)
- [modbus_set_float_dcba](modbus_set_float_dcba.md)
- [modbus_get_float_dcba](modbus_get_float_dcba.md)
