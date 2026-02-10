# modbus_set_bits_from_byte

## Name

modbus_set_bits_from_byte - set many bits from a single byte value


## Synopsis

```c
void modbus_set_bits_from_byte(uint8_t *dest, int index, const uint8_t value);
```

## Description

The *modbus_set_bits_from_byte()* function shall set many bits from a single
byte. All 8 bits from the byte `value` will be written to `dest` array starting
at `index` position.

## Return value

There is no return value.

## See also

- [modbus_get_byte_from_bits](modbus_get_byte_from_bits.md)
- [modbus_set_bits_from_bytes](modbus_set_bits_from_bytes.md)
