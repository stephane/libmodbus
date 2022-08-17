# modbus_get_header_length

## Name

modbus_get_header_length - retrieve the current header length

## Synopsis

```c
int modbus_get_header_length(modbus_t *ctx);
```

## Description

The *modbus_get_header_length()* function shall retrieve the current header
length from the backend. This function is convenient to manipulate a message and
so its limited to low-level operations.

## Return value

The header length as integer value.
