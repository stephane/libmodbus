#### Input Files ####
set(MAN7_SRCS libmodbus.txt)
set(MAN3_SRCS
    modbus_write_bit.txt
    modbus_write_and_read_registers.txt
    modbus_rtu_set_rts.txt
    modbus_reply_exception.txt
    modbus_rtu_get_rts_delay.txt
    modbus_set_bits_from_bytes.txt
    modbus_mapping_new_start_address.txt
    modbus_report_slave_id.txt
    modbus_read_input_bits.txt
    modbus_rtu_set_rts_delay.txt
    modbus_set_socket.txt
    modbus_set_float_abcd.txt
    modbus_free.txt
    modbus_new_tcp.txt
    modbus_rtu_set_custom_rts.txt
    modbus_set_debug.txt
    modbus_set_slave.txt
    modbus_read_input_registers.txt
    modbus_tcp_accept.txt
    modbus_set_response_timeout.txt
    modbus_receive.txt
    modbus_get_float_cdab.txt
    modbus_get_byte_from_bits.txt
    modbus_new_tcp_pi.txt
    modbus_close.txt
    modbus_mapping_new.txt
    modbus_read_registers.txt
    modbus_flush.txt
    modbus_get_float_badc.txt
    modbus_send_raw_request.txt
    modbus_set_float_cdab.txt
    modbus_mask_write_register.txt
    modbus_rtu_get_rts.txt
    modbus_write_registers.txt
    modbus_rtu_get_serial_mode.txt
    modbus_strerror.txt
    modbus_receive_confirmation.txt
    modbus_tcp_pi_listen.txt
    modbus_set_error_recovery.txt
    modbus_set_float_dcba.txt
    modbus_write_bits.txt
    modbus_write_register.txt
    modbus_set_bits_from_byte.txt
    modbus_get_float_dcba.txt
    modbus_tcp_pi_accept.txt
    modbus_get_byte_timeout.txt
    modbus_get_header_length.txt
    modbus_reply.txt
    modbus_get_socket.txt
    modbus_connect.txt
    modbus_tcp_listen.txt
    modbus_set_float_badc.txt
    modbus_mapping_free.txt
    modbus_get_float.txt
    modbus_new_rtu.txt
    modbus_set_byte_timeout.txt
    modbus_read_bits.txt
    modbus_get_float_abcd.txt
    modbus_set_float.txt
    modbus_rtu_set_serial_mode.txt
    modbus_get_response_timeout.txt)

#### Build Configuration ####
option(BUILD_DOCS_HTML "Build HTML documentation" ON)
option(BUILD_DOCS_MAN "Build manpage documentation" ON)

set(ASCIIDOC_DIR "" CACHE PATH "The dir containing asciidoc executable")
set(A2X_DIR "" CACHE PATH "The dir containing a2x executable")

find_program(ASCIIDOC_EXECUTABLE asciidoc
            HINTS ${ASCIIDOC_DIR})

find_program(A2X_EXECUTABLE NAMES a2x a2x.py
            HINTS ${A2X_DIR})

mark_as_advanced(ASCIIDOC_EXECUTABLE)
mark_as_advanced(A2X_EXECUTABLE)

if(ASCIIDOC_EXECUTABLE)
    set(ASCIIDOC_ARGS
        "-d manpage -b xhtml11 -f \"${CMAKE_CURRENT_SOURCE_DIR}/asciidoc.conf\" -alibmodbus_version=${PROJECT_VERSION}"
        CACHE STRING "Command line arguments for asciidoc")
        mark_as_advanced(ASCIIDOC_ARGS)

        separate_arguments(ASCIIDOC_ARG_LIST UNIX_COMMAND ${ASCIIDOC_ARGS})
else(ASCIIDOC_EXECUTABLE)
    message(WARNING "Cannot find asciidoc executable, building HTML documentation is disabled."
                    "You can set ASCIIDOC_DIR and/or ASCIIDOC_EXECUTABLE to enable it.")
    set(BUILD_DOCS_HTML OFF CACHE BOOL "Build HTML documentation" FORCE)
endif(ASCIIDOC_EXECUTABLE)

if(A2X_EXECUTABLE)
    set(A2X_ARGS
        "--doctype manpage --format manpage -alibmodbus_version=${PROJECT_VERSION}"
        CACHE STRING "Command line arguments for a2x")
    mark_as_advanced(A2X_ARGS)

    separate_arguments(A2X_ARG_LIST UNIX_COMMAND ${A2X_ARGS})
else(A2X_EXECUTABLE)
    message(WARNING "Cannot find asciidoc executable, building manpage documentation is disabled."
                    "You can set A2X_DIR and/or A2X_EXECUTABLE to enable it.")
    set(BUILD_DOCS_MAN OFF CACHE BOOL "Build manpage documentation" FORCE)
endif(A2X_EXECUTABLE)

if((NOT BUILD_DOCS_HTML) AND (NOT BUILD_DOCS_MAN))
    message(WARNING "Neither asciidoc nor a2x executable has been found."
                    "Building documentation is disabled")
    set(BUILD_DOCS OFF CACHE BOOL "Build libmodbus docs" FORCE)
endif((NOT BUILD_DOCS_HTML) AND (NOT BUILD_DOCS_MAN))

#### Build ####
set(MAN3_OUTPUTS "")
set(MAN7_OUTPUTS "")
set(HTML_OUTPUTS "")

foreach(input_file ${MAN3_SRCS})
    if(BUILD_DOCS_MAN)
        string(REGEX REPLACE ".txt" ".3" output_file ${input_file})
        list(APPEND MAN3_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/${output_file}")
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_file}"
                           COMMAND ${A2X_EXECUTABLE} ${A2X_ARG_LIST} "-D" "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${input_file}"
                           DEPENDS ${input_file})
    endif(BUILD_DOCS_MAN)

    if(BUILD_DOCS_HTML)
        string(REGEX REPLACE ".txt" ".html" output_file ${input_file})
        list(APPEND HTML_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/${output_file}")
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_file}"
                           COMMAND ${ASCIIDOC_EXECUTABLE} ${ASCIIDOC_ARG_LIST} "-o" "${CMAKE_CURRENT_BINARY_DIR}/${output_file}" "${CMAKE_CURRENT_SOURCE_DIR}/${input_file}"
                           DEPENDS ${input_file})
    endif(BUILD_DOCS_HTML)
endforeach(input_file ${MAN3_SRCS})

foreach(input_file ${MAN7_SRCS})
    if(BUILD_DOCS_MAN)
        string(REGEX REPLACE ".txt" ".7" output_file ${input_file})
        list(APPEND MAN7_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/${output_file}")
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_file}"
                           COMMAND ${A2X_EXECUTABLE} ${A2X_ARG_LIST} "-D" "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${input_file}"
                           DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${input_file}")
    endif(BUILD_DOCS_MAN)

    if(BUILD_DOCS_HTML)
        string(REGEX REPLACE ".txt" ".html" output_file ${input_file})
        list(APPEND HTML_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/${output_file}")
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${output_file}"
                           COMMAND ${ASCIIDOC_EXECUTABLE} ${ASCIIDOC_ARG_LIST} "-o" "${CMAKE_CURRENT_BINARY_DIR}/${output_file}" "${CMAKE_CURRENT_SOURCE_DIR}/${input_file}"
                           DEPENDS ${input_file})
    endif(BUILD_DOCS_HTML)
endforeach(input_file ${MAN3_SRCS})

add_custom_target(docs)

if(BUILD_DOCS_HTML)
    add_custom_target(docs_html
                      SOURCES "${MAN7_SRCS}" "${MAN3_SRCS}"
                      DEPENDS "${HTML_OUTPUTS}")
    add_dependencies(docs docs_html)

    install(FILES ${HTML_OUTPUTS}
            DESTINATION ${CMAKE_INSTALL_DOCDIR} OPTIONAL)
endif(BUILD_DOCS_HTML)

if(BUILD_DOCS_MAN)
    add_custom_target(docs_man3
                      SOURCES "${MAN3_SRCS}"
                      DEPENDS "${MAN3_OUTPUTS}")

    add_custom_target(docs_man7
                      SOURCES "${MAN7_SRCS}"
                      DEPENDS "${MAN7_OUTPUTS}")

    add_custom_target(docs_man)
    add_dependencies(docs_man docs_man3 docs_man7)

    add_dependencies(docs docs_man)

    install(FILES ${MAN3_OUTPUTS}
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man3 OPTIONAL)

    install(FILES ${MAN7_OUTPUTS}
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man7 OPTIONAL)
endif(BUILD_DOCS_MAN)
