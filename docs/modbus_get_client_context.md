# modbus_get_client_context


## Name

`modbus_get_client_context` - returns the client context value from the libmodbus
context


## Synopsis

```c
const void *modbus_get_client_context(modbus_t *'ctx');
```

## Description

The `modbus_get_client_context()` function shall return the client context value
from the `libmodbus` context.

Do not be confused by the overloaded use of “client” and “context” in these
functions. Here, “client” refers to the code that makes use of `libmodbus`,
“client context” is the data set and get by the client code, and “libmodbus context”
is the `modbus_t*` context pointer.

## Return Value

The function shall return the client context or NULL.


## Errors

This function does not return an error.


## See also

- [`modbus_set_client_context()`](modbus_set_client_context)
