MAN3 = \
        modbus_close.3 \
        modbus_connect.3 \
        modbus_flush.3 \
        modbus_free.3 \
        modbus_get_byte_from_bits.3 \
        modbus_get_byte_timeout.3 \
        modbus_get_float.3 \
        modbus_get_header_length.3 \
        modbus_get_response_timeout.3 \
        modbus_get_socket.3 \
        modbus_mapping_free.3 \
        modbus_mapping_new.3 \
        modbus_new_rtu.3 \
        modbus_new_tcp_pi.3 \
        modbus_new_tcp.3 \
        modbus_read_bits.3 \
        modbus_read_input_bits.3 \
        modbus_read_input_registers.3 \
        modbus_read_registers.3 \
        modbus_receive_confirmation.3 \
        modbus_receive_from.3 \
        modbus_receive.3 \
        modbus_reply_exception.3 \
        modbus_reply.3 \
        modbus_report_slave_id.3 \
        modbus_rtu_get_serial_mode.3 \
        modbus_rtu_set_serial_mode.3 \
        modbus_send_raw_request.3 \
        modbus_set_bits_from_bytes.3 \
        modbus_set_bits_from_byte.3 \
        modbus_set_byte_timeout.3 \
        modbus_set_debug.3 \
        modbus_set_error_recovery.3 \
        modbus_set_float.3 \
        modbus_set_response_timeout.3 \
        modbus_set_slave.3 \
        modbus_set_socket.3 \
        modbus_strerror.3 \
        modbus_tcp_listen.3 \
        modbus_write_and_read_registers.3 \
        modbus_write_bits.3 \
        modbus_write_bit.3 \
        modbus_write_registers.3 \
        modbus_write_register.3
MAN7 = libmodbus.7

MAN_DOC = $(MAN3) $(MAN7)

MAN_TXT = $(MAN3:%.3=%.txt)
MAN_TXT += $(MAN7:%.7=%.txt)
MAN_HTML = $(MAN_TXT:%.txt=%.html)

if INSTALL_MAN
dist_man_MANS = $(MAN_DOC)
doc: $(MAN_DOC)
endif

EXTRA_DIST = asciidoc.conf $(MAN_TXT)
if BUILD_DOC
EXTRA_DIST += $(MAN_HTML)
html: $(MAN_HTML)
endif

MAINTAINERCLEANFILES = $(MAN_DOC) $(MAN_HTML)

dist-hook: $(MAN_DOC) $(MAN_HTML)

if BUILD_DOC
SUFFIXES=.html .txt .xml .1 .3 .7

.txt.html:
	asciidoc -d manpage -b xhtml11 -f asciidoc.conf \
		-alibmodbus_version=@LIBMODBUS_VERSION@ $<
.txt.xml:
	asciidoc -d manpage -b docbook -f asciidoc.conf \
		-alibmodbus_version=@LIBMODBUS_VERSION@ $<
.xml.1:
	xmlto man $<
.xml.3:
	xmlto man $<
.xml.7:
	xmlto man $<

clean:
	rm -f *.1 *.3 *.7
	rm -f *.html

endif
