# modbus_set_trace_handler

## Name

modbus_set_trace_handler - call method when log data is written

## Synopsis

```c
typedef int (*modbus_stream_handler_t)(void *user, const char *format, va_list ap);
void modbus_set_trace_handler(modbus_t *ctx, modbus_stream_handler_t handler);
```

## Description

The *modbus_set_trace_handler()* sets a callback. When log data is written, the 
callback is called. A log message is finalized with a '\n' as last character. 
Defaults to vfprintf when not set.


## Example

```c++
class Test {
  public:
    static int log_callback(void *user, const char *format, va_list ap)
    {
        auto *inst = reinterpret_cast<Test*>(user);
        //call methods
    }
    void setup()
    {
        modbus_set_out_user_data(reinterpret_cast<void*>(this));
        modbus_set_error_user_data(reinterpret_cast<void*>(this));
        modbus_set_trace_handler(log_callback);
        modbus_set_debug(true);
    }
}
```

## See also

- [modbus_set_out_user_data](modbus_set_out_user_data.md)
- [modbus_set_error_user_data](modbus_set_error_user_data.md)
