#include "modbus-log.h"

#include <stdio.h>
#include <stddef.h>

static void* modbus_out_user_data = NULL;
static void* modbus_error_user_data = NULL;
static modbus_stream_handler_t modbus_stream_handler = (modbus_stream_handler_t) vfprintf;

MODBUS_API void modbus_set_out_user_data(void* out_user_data)
{
    modbus_out_user_data = out_user_data;
}

MODBUS_API void modbus_set_error_user_data(void* error_user_data)
{
    modbus_error_user_data = error_user_data;
}

MODBUS_API void modbus_set_trace_handler(modbus_stream_handler_t handler)
{
    modbus_stream_handler = handler;
}

MODBUS_API int modbus_trace(const char* format, ...)
{
    int result;
    va_list argp;
    va_start(argp, format);

    result = modbus_vtrace(format, argp);

    va_end(argp);
    return result;
}

MODBUS_API int modbus_vtrace(const char* format, va_list ap)
{
    if (!modbus_out_user_data) {
        modbus_out_user_data = stdout;
    }
    return modbus_stream_handler(modbus_out_user_data, format, ap);
}

MODBUS_API int modbus_trace_error(const char* format, ...)
{
    int result;
    va_list argp;
    va_start(argp, format);

    result = modbus_vtrace_error(format, argp);

    va_end(argp);
    return result;
}

MODBUS_API int modbus_vtrace_error(const char* format, va_list ap)
{
    if (!modbus_error_user_data) {
        modbus_error_user_data = stderr;
    }
    return modbus_stream_handler(modbus_error_user_data, format, ap);
}
