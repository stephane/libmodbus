# modbus_set_float

## Name

modbus_set_float - set a float value from 2 registers

## Synopsis

```c
void modbus_set_float(float f, uint16_t *dest);
```

Warning, this function is *deprecated* since libmodbus v3.2.0 and has been
replaced by *modbus_set_float_dcba()*.

## Description

The *modbus_set_float()* function shall set a float to 4 bytes in Modbus format
(ABCD). The `dest` array must be pointer on two 16 bits values to be able to
store the full result of the conversion.

## Return value

There is no return values.

## See also

- [modbus_get_float](modbus_get_float.md)
- [modbus_set_float_dcba](modbus_set_float_dcba.md)
