#ifndef MODBUS_LOG_H
#define MODBUS_LOG_H

#include "modbus.h"

#include <stdarg.h>

MODBUS_API void modbus_set_out_user_data(void* out_user_data);
MODBUS_API void modbus_set_error_user_data(void* error_user_data);

typedef int (*modbus_stream_handler_t)(void *user, const char *format, va_list ap);
MODBUS_API void modbus_set_trace_handler(modbus_stream_handler_t handler);

MODBUS_API int modbus_trace(const char* format, ...);
MODBUS_API int modbus_vtrace(const char* format, va_list ap);
MODBUS_API int modbus_trace_error(const char* format, ...);
MODBUS_API int modbus_vtrace_error(const char* format, va_list ap);

#endif /* MODBUS_LOG_H */
