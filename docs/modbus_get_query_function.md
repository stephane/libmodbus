# modbus_get_header_length

## Name

modbus_get_query_function- retrieve the function code of a query

## Synopsis

```c
int modbus_get_header_length(modbus_t *ctx, const uint8_t * query);
```

## Description

The *modbus_get_query_function()* function shall retrieve the function code
of a query. It can be useful for modbus server development.

## Return value

The function code a the query.
