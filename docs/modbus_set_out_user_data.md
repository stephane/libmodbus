# modbus_set_out_user_data

## Name

modbus_set_out_user_data - set file stream to write log output

## Synopsis

```c
void modbus_set_out_user_data(modbus_t *ctx, void* out_user_data);
```

## Description

The *modbus_set_out_user_data()* changes where log output is written
to when enabled with [modbus_set_debug](modbus_set_debug.md). Defaults
to stdout when not set.


## Example

```c
FILE *fp;
fp = fopen("output.txt", "w");
modbus_set_out_user_data(ctx, fp);
modbus_set_debug(ctx, 1)
```

## See also

- [modbus_set_error_user_data](modbus_set_error_user_data.md)
- [modbus_set_trace_handler](modbus_set_trace_handler.md)
