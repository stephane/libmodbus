#include "modbus-log.h"
#include "modbus-private.h"
#include "modbus.h"

#include <errno.h>
#include <stdio.h>
#include <stddef.h>

MODBUS_API void modbus_set_out_user_data(modbus_t *ctx, void* out_user_data)
{
    if (ctx == NULL) {
        errno = EINVAL;
        return;
    }
    ctx->out_user_data = out_user_data;
}

MODBUS_API void modbus_set_error_user_data(modbus_t *ctx, void* error_user_data)
{
    if (ctx == NULL) {
        errno = EINVAL;
        return;
    }
    ctx->error_user_data = error_user_data;
}

MODBUS_API void modbus_set_trace_handler(modbus_t *ctx, modbus_stream_handler_t handler)
{
    if (ctx == NULL) {
        errno = EINVAL;
        return;
    }
    ctx->stream_handler = handler;
}

MODBUS_API int modbus_trace(modbus_t *ctx, const char* format, ...)
{
    int result;
    va_list argp;
    va_start(argp, format);

    result = modbus_vtrace(ctx, format, argp);

    va_end(argp);
    return result;
}

MODBUS_API int modbus_vtrace(modbus_t *ctx, const char* format, va_list ap)
{
    if (ctx == NULL || ctx->stream_handler == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (!ctx->out_user_data) {
        ctx->out_user_data = stdout;
    }
    return ctx->stream_handler(ctx->out_user_data, format, ap);
}

MODBUS_API int modbus_trace_error(modbus_t *ctx, const char* format, ...)
{
    int result;
    va_list argp;
    va_start(argp, format);

    result = modbus_vtrace_error(ctx, format, argp);

    va_end(argp);
    return result;
}

MODBUS_API int modbus_vtrace_error(modbus_t *ctx, const char* format, va_list ap)
{
    if (ctx == NULL || ctx->stream_handler == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (!ctx->error_user_data) {
        ctx->error_user_data = stderr;
    }
    return ctx->stream_handler(ctx->error_user_data, format, ap);
}
