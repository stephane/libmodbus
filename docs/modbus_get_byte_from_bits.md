# modbus_get_byte_from_bits

## Name

modbus_get_byte_from_bits - get the value from many bits

## Synopsis

```c
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int index, unsigned int nb_bits);
```

## Description

The *modbus_get_byte_from_bits()* function shall extract a value from many
bits. All `nb_bits` bits from `src` at position `index` will be read as a
single value. To obtain a full byte, set nb_bits to 8.

## Return value

The function shall return a byte containing the bits read.

## See also

- [modbus_set_bits_from_byte](modbus_set_bits_from_byte.md)
- [modbus_set_bits_from_bytes](modbus_set_bits_from_bytes.md)
