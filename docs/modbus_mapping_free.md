# modbus_mapping_free

## Name

modbus_mapping_free - free a modbus_mapping_t structure

## Synopsis

```c
void modbus_mapping_free(modbus_mapping_t *mb_mapping);
```

## Description

The function shall free the four arrays of mb_mapping_t structure and finally
the mb_mapping_t referenced by `mb_mapping`.

## Return value

There is no return values.

## See also

- [modbus_mapping_new](modbus_mapping_new.md)
