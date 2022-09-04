# modbus_set_bits_from_bytes

## Name

modbus_set_bits_from_bytes - set many bits from an array of bytes

## Synopsis

```c
void modbus_set_bits_from_bytes(uint8_t *dest, int index, unsigned int nb_bits, const uint8_t *tab_byte);
```

## Description

The *modbus_set_bits_from_bytes* function shall set bits by reading an array of
bytes. All the bits of the bytes read from the first position of the array
`tab_byte` are written as bits in the `dest` array starting at position `index`.

## Return value

There is no return values.

## See also

- [modbus_set_bits_from_byte](modbus_set_bits_from_byte.md)
- [modbus_get_byte_from_bits](modbus_get_byte_from_bits.md)
