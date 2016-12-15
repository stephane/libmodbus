/*
 * Simple logging framework.
 * 
 * License: Unlicense(http://unlicense.org)
 *
 * Find the latest version at https://github.com/rm5248/simplelogger
 *
 * To use: Copy this file into your source directory and define storage for
 * simplelogger_global_log_function in a source file.  Then set your 
 * simplelogger_global_log_function to be a function pointer to a function
 * that you control.
 */
#ifndef SIMPLELOGGER_H
#define SIMPLELOGGER_H

#include "simplelogger_defs.h"

#ifndef SIMPLELOGGER_FUNCTION
  #ifdef _MSC_VER
    #define SIMPLELOGGER_FUNCTION __FUNCSIG__
  #else
    #define SIMPLELOGGER_FUNCTION __func__
  #endif
#endif

/**
 * Global log pointer
 */
#ifndef SIMPLELOGGER_LOG_FUNCTION_NAME
#define SIMPLELOGGER_LOG_FUNCTION_NAME simplelogger_global_log_function
#endif
extern simplelogger_log_function SIMPLELOGGER_LOG_FUNCTION_NAME;

#define SIMPLELOGGER_LOG_CSTR( logger, message, level ) do{\
    if( !SIMPLELOGGER_LOG_FUNCTION_NAME ) break;\
    struct SL_LogLocation location;\
    location.line_number = __LINE__;\
    location.file = __FILE__;\
    location.function = SIMPLELOGGER_FUNCTION;\
    SIMPLELOGGER_LOG_FUNCTION_NAME( logger, &location, level, message );\
    } while(0)

#define SIMPLELOGGER_TRACE_CSTR( logger, message ) do{\
    SIMPLELOGGER_LOG_CSTR( logger, message, SL_TRACE);\
    } while(0)
#define SIMPLELOGGER_DEBUG_CSTR( logger, message ) do{\
    SIMPLELOGGER_LOG_CSTR( logger, message, SL_DEBUG);\
    } while(0)
#define SIMPLELOGGER_INFO_CSTR( logger, message ) do{\
    SIMPLELOGGER_LOG_CSTR( logger, message, SL_INFO);\
    } while(0)
#define SIMPLELOGGER_WARN_CSTR( logger, message ) do{\
    SIMPLELOGGER_LOG_CSTR( logger, message, SL_WARN);\
    } while(0)
#define SIMPLELOGGER_ERROR_CSTR( logger, message ) do{\
    SIMPLELOGGER_LOG_CSTR( logger, message, SL_ERROR);\
    } while(0)
#define SIMPLELOGGER_FATAL_CSTR( logger, message ) do{\
    SIMPLELOGGER_LOG_CSTR( logger, message, SL_FATAL);\
    } while(0)

#ifdef __cplusplus
#include <string>
#include <sstream>

#define SIMPLELOGGER_TRACE_STDSTR( logger, message ) do{\
    std::stringstream stream;\
    stream << message;\
    SIMPLELOGGER_LOG_CSTR( logger, stream.str().c_str(), SL_TRACE);\
    } while(0)
#define SIMPLELOGGER_DEBUG_STDSTR( logger, message ) do{\
    std::stringstream stream;\
    stream << message;\
    SIMPLELOGGER_LOG_CSTR( logger, stream.str().c_str(), SL_DEBUG);\
    } while(0)
#define SIMPLELOGGER_INFO_STDSTR( logger, message ) do{\
    std::stringstream stream;\
    stream << message;\
    SIMPLELOGGER_LOG_CSTR( logger, stream.str().c_str(), SL_INFO);\
    } while(0)
#define SIMPLELOGGER_WARN_STDSTR( logger, message ) do{\
    std::stringstream stream;\
    stream << message;\
    SIMPLELOGGER_LOG_CSTR( logger, stream.str().c_str(), SL_WARN);\
    } while(0)
#define SIMPLELOGGER_ERROR_STDSTR( logger, message ) do{\
    std::stringstream stream;\
    stream << message;\
    SIMPLELOGGER_LOG_CSTR( logger, stream.str().c_str(), SL_ERROR);\
    } while(0)
#define SIMPLELOGGER_FATAL_STDSTR( logger, message ) do{\
    std::stringstream stream;\
    stream << message;\
    SIMPLELOGGER_LOG_CSTR( logger, stream.str().c_str(), SL_FATAL);\
    } while(0)

#ifdef SIMPLELOGGER_ENABLE_AUTO_MACROS
  #define SIMPLELOGGER_TRACE( logger, message )\
    SIMPLELOGGER_TRACE_STDSTR(logger, message )
  #define SIMPLELOGGER_DEBUG( logger, message )\
    SIMPLELOGGER_DEBUG_STDSTR(logger, message )
  #define SIMPLELOGGER_INFO( logger, message )\
    SIMPLELOGGER_INFO_STDSTR(logger, message )
  #define SIMPLELOGGER_WARN( logger, message )\
    SIMPLELOGGER_WARN_STDSTR(logger, message )
  #define SIMPLELOGGER_ERROR( logger, message )\
    SIMPLELOGGER_ERROR_STDSTR(logger, message )
  #define SIMPLELOGGER_FATAL( logger, message )\
    SIMPLELOGGER_FATAL_STDSTR(logger, message )
#endif /* SIMPLELOGGER_ENABLE_AUTO_MACROS */

#ifdef SIMPLELOGGER_ENABLE_SMALL_MACROS
  #define LOG_TRACE( logger, message )\
    SIMPLELOGGER_TRACE_STDSTR( logger, message )
  #define LOG_DEBUG( logger, message )\
    SIMPLELOGGER_DEBUG_STDSTR( logger, message )
  #define LOG_INFO( logger, message )\
    SIMPLELOGGER_INFO_STDSTR( logger, message )
  #define LOG_WARN( logger, message )\
    SIMPLELOGGER_WARN_STDSTR( logger, message )
  #define LOG_ERROR( logger, message )\
    SIMPLELOGGER_ERROR_STDSTR( logger, message )
  #define LOG_FATAL( logger, message )\
    SIMPLELOGGER_FATAL_STDSTR( logger, message )
#endif /* SIMPLELOGGER_ENABLE_SMALL_MACROS */

#else
/* C macros */

#ifdef SIMPLELOGGER_ENABLE_AUTO_MACROS
  #define SIMPLELOGGER_TRACE( logger, message )\
    SIMPLELOGGER_TRACE_CSTR(logger, message )
  #define SIMPLELOGGER_DEBUG( logger, message )\
    SIMPLELOGGER_DEBUG_CSTR(logger, message )
  #define SIMPLELOGGER_INFO( logger, message )\
    SIMPLELOGGER_INFO_CSTR(logger, message )
  #define SIMPLELOGGER_WARN( logger, message )\
    SIMPLELOGGER_WARN_CSTR(logger, message )
  #define SIMPLELOGGER_ERROR( logger, message )\
    SIMPLELOGGER_ERROR_CSTR(logger, message )
  #define SIMPLELOGGER_FATAL( logger, message )\
    SIMPLELOGGER_FATAL_CSTR(logger, message )
#endif /* SIMPLELOGGER_ENABLE_AUTO_MACROS */

#ifdef SIMPLELOGGER_ENABLE_SMALL_MACROS
  #define LOG_TRACE( logger, message )\
    SIMPLELOGGER_TRACE_CSTR( logger, message )
  #define LOG_DEBUG( logger, message )\
    SIMPLELOGGER_DEBUG_CSTR( logger, message )
  #define LOG_INFO( logger, message )\
    SIMPLELOGGER_INFO_CSTR( logger, message )
  #define LOG_WARN( logger, message )\
    SIMPLELOGGER_WARN_CSTR( logger, message )
  #define LOG_ERROR( logger, message )\
    SIMPLELOGGER_ERROR_CSTR( logger, message )
  #define LOG_FATAL( logger, message )\
    SIMPLELOGGER_FATAL_CSTR( logger, message )
#endif /* SIMPLELOGGER_ENABLE_SMALL_MACROS */

#endif /* __cplusplus */


#endif /* SIMPLELOGGER_H */
