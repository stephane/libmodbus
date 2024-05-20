# Instructions to use with Espressif IoT Development Framework (ESP-IDF)

## Adding libmodbus as a component

- Create a subdirectory at the top level of your ESP-IDF project where you will
place this and create a component, ie. `libmodbus`. This directory will be
referred as *component directory* further down in this text.

- Download the latest version of libmodbus with the method of your choice and
unpack it under component directory in a subdirectory `libmodbus`

- Copy the files supplied in this documentation directory to the component directory,
namely:
  - `CMakeLists.txt`: the CMake script that enumerates files and directories used
     in the build as well as defines needed dependencies
  - `component.mk`: the component build definition
  - `config.h`: the library configuration, especially tailored for ESP-IDF. This is 
    usually generated with the autoconf tool, but this is not present in ESP-IDF and
    is therefore manually prepared and customized
  - `idf_component.yml`: the component description file

- Add a reference from your main project in the project top level `CMakeLists.txt` to
  the newly added module with something like: `set(EXTRA_COMPONENT_DIRS libmodbus/)`. 
  If you already have other components you may just add the reference to the newly
  added component.

- As the ESP-IDF does not provide a `nanosleep` function in its SDK, you should add
  this in your project so you will be able to find it at linking time, for example:

```
int nanosleep(const struct timespec *req, struct timespec *_Nullable rem) {
    return usleep(req->tv_sec*1000 + req->tv_nsec / 1000);
}
```

Now you are almost ready to use libmodbus in your project!

If you desire to use the library for serial communication, you will need to do a few
more hardware configuration steps before using the `modbus_new_rtu` call, namely:

- Configure, if needed, any pins for the used uart via `uart_set_pin`

- Install the uart driver via the `uart_driver_install`

- Configure, if needed, the uart mode (ie. set it to half duplex) via `uart_set_mode`

These configurations are not included in libmodbus as they are highly hardware specific
and would require a heavy change in the library interface.

## Other details using libmodbus with ESP-IDF

- The serial driver is implemented using the `vfs` virtual filesystem component. This
  makes the changes needed for the library minimal, but may not be the most performant
  solution. Be aware that if you are not using the UART0 as console (ie. you are
  disabling console or you are using the USB Serial/JTAG Controller) you may need to
  explicitly initialize UART VFS in your main program as well just by calling
  `uart_vfs_dev_register()` (from `driver/uart_vfs.h`).

- The serial name (first parameter to `modbus_new_rtu`) should be a string containing
  only the serial index (ie. `"1"` or `"2"`).

- When using the TCP version be aware of the maximum number of sockets that can be
  open on the platform: this is by default 10 and can be possibly raised to 16 in
  a standard configuration. Please check the `LWIP_MAX_SOCKETS` configuration
  variable.

