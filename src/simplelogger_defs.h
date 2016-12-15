/*
 * Simple logging framework.
 * 
 * License: Unlicense(http://unlicense.org)
 *
 * Find the latest version at https://github.com/rm5248/simplelogger
 *
 * Definitions for library usage.  This is the file that must be distributed
 * if you are using the Simplelogger in a library.
 */
#ifndef SIMPLELOGGER_DEFS_H
#define SIMPLELOGGER_DEFS_H

/**
 * Where the log message was generated from.
 */
struct SL_LogLocation{
    int line_number;
    const char* function;
    const char* file;
};

/**
 * Level of the log message
 */
enum SL_LogLevel{
    SL_TRACE,
    SL_DEBUG,
    SL_INFO,
    SL_WARN,
    SL_ERROR,
    SL_FATAL
};

/**
 * Pointer to a function that does the actual log operation.
 */
typedef void (*simplelogger_log_function)(const char* logger_name, 
    const struct SL_LogLocation* location,
    const enum SL_LogLevel level,
    const char* log_string );


#define SL_LOGLEVEL_TO_STRING( stringPtr, level ) do{\
  switch( level ){ \
  case SL_TRACE: stringPtr = "TRACE"; break;\
  case SL_DEBUG: stringPtr = "DEBUG"; break;\
  case SL_INFO: stringPtr = "INFO"; break;\
  case SL_WARN: stringPtr = "WARN"; break;\
  case SL_ERROR: stringPtr = "ERROR"; break;\
  case SL_FATAL: stringPtr = "FATAL"; break;\
  default: stringPtr = "UNKN"; break;\
  }}while(0)


#endif /* SIMPLELOGGER_DEFS_H */
