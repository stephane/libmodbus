TXT3 = \
        modbus_close.txt \
        modbus_connect.txt \
        modbus_flush.txt \
        modbus_free.txt \
        modbus_get_byte_from_bits.txt \
        modbus_get_byte_timeout.txt \
        modbus_get_float.txt \
        modbus_get_float_abcd.txt \
        modbus_get_float_badc.txt \
        modbus_get_float_cdab.txt \
        modbus_get_float_dcba.txt \
        modbus_get_header_length.txt \
        modbus_get_response_timeout.txt \
        modbus_get_socket.txt \
        modbus_mapping_free.txt \
        modbus_mapping_new.txt \
        modbus_mapping_new_start_address.txt \
        modbus_mask_write_register.txt \
        modbus_new_rtu.txt \
        modbus_new_tcp_pi.txt \
        modbus_new_tcp.txt \
        modbus_read_bits.txt \
        modbus_read_input_bits.txt \
        modbus_read_input_registers.txt \
        modbus_read_registers.txt \
        modbus_receive_confirmation.txt \
        modbus_receive.txt \
        modbus_reply_exception.txt \
        modbus_reply.txt \
        modbus_report_slave_id.txt \
        modbus_rtu_get_serial_mode.txt \
        modbus_rtu_set_serial_mode.txt \
        modbus_rtu_get_rts.txt \
        modbus_rtu_set_rts.txt \
        modbus_rtu_set_custom_rts.txt \
        modbus_rtu_get_rts_delay.txt \
        modbus_rtu_set_rts_delay.txt \
        modbus_send_raw_request.txt \
        modbus_set_bits_from_bytes.txt \
        modbus_set_bits_from_byte.txt \
        modbus_set_byte_timeout.txt \
        modbus_set_debug.txt \
        modbus_set_error_recovery.txt \
        modbus_set_float.txt \
        modbus_set_float_abcd.txt \
        modbus_set_float_badc.txt \
        modbus_set_float_cdab.txt \
        modbus_set_float_dcba.txt \
        modbus_set_response_timeout.txt \
        modbus_set_slave.txt \
        modbus_set_socket.txt \
        modbus_strerror.txt \
        modbus_tcp_accept.txt \
        modbus_tcp_pi_accept.txt \
        modbus_tcp_listen.txt \
        modbus_tcp_pi_listen.txt \
        modbus_write_and_read_registers.txt \
        modbus_write_bits.txt \
        modbus_write_bit.txt \
        modbus_write_registers.txt \
        modbus_write_register.txt
TXT7 = libmodbus.txt

EXTRA_DIST = asciidoc.conf $(TXT3) $(TXT7)

MAN3 = $(TXT3:%.txt=%.3)
MAN7 = $(TXT7:%.txt=%.7)

if BUILD_DOC
man3_MANS = $(MAN3)
man7_MANS = $(MAN7)
endif

HTML = $(TXT3:%.txt=%.html) $(TXT7:%.txt=%.html)

htmldoc: $(HTML)

.txt.html:
	asciidoc -d manpage -b xhtml11 -f asciidoc.conf -alibmodbus_version=@LIBMODBUS_VERSION@ $<

.txt.3 .txt.7:
	a2x --doctype manpage --format manpage -alibmodbus_version=@LIBMODBUS_VERSION@ $<

CLEANFILES = *.3 *.7 *.html
