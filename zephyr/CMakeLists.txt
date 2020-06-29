if(CONFIG_LIBMODBUS)

zephyr_include_directories(../src)
zephyr_library()

zephyr_library_sources(
	../src/modbus.c
	../src/modbus-data.c
)
zephyr_library_sources_ifdef(
	CONFIG_LIBMODBUS_TCP ../src/modbus-tcp.c)

endif()
