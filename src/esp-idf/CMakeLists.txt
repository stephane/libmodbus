# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
set(srcs
    "src/modbus-data.c"
    "src/modbus-rtu.c"
    "src/modbus-tcp.c"
    "src/modbus.c")

set(include_dirs src)

set(priv_include_dirs ../)

list(APPEND priv_include_dirs)

add_prefix(srcs "${CMAKE_CURRENT_LIST_DIR}/libmodbus/" ${srcs})
add_prefix(include_dirs "${CMAKE_CURRENT_LIST_DIR}/libmodbus/" ${include_dirs})
add_prefix(priv_include_dirs "${CMAKE_CURRENT_LIST_DIR}/libmodbus/" ${priv_include_dirs})

message(STATUS "DEBUG: Using libmodbus component folder: ${CMAKE_CURRENT_LIST_DIR}.")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "${include_dirs}"
                    PRIV_INCLUDE_DIRS "${priv_include_dirs}"
                    PRIV_REQUIRES driver vfs)

