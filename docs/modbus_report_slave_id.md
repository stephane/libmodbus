# modbus_report_slave_id

## Name

modbus_report_slave_id - returns a description of the controller

## Synopsis

```c
int modbus_report_slave_id(modbus_t *ctx, int max_dest, uint8_t *dest);
```

## Description

The *modbus_report_slave_id()* function shall send a request to the controller
to obtain a description of the controller.

The response stored in `dest` contains:

- the slave ID, this unique ID is in reality not unique at all so it's not
  possible to depend on it to know how the information are packed in the
  response.
- the run indicator status (0x00 = OFF, 0xFF = ON)
- additional data specific to each controller. For example, libmodbus returns
  the version of the library as a string.

The function writes at most `max_dest` bytes from the response to `dest` so
you must ensure that `dest` is large enough.

## Return value

The function shall return the number of read data if successful.

If the output was truncated due to the `max_dest` limit then the return value is
the number of bytes which would have been written to `dest` if enough space had
been available. Thus, a return value greater than `max_dest` means that the
response data was truncated.

Otherwise it shall return -1 and set errno.

## Example

```c
uint8_t tab_bytes[MODBUS_MAX_PDU_LENGTH];

...

rc = modbus_report_slave_id(ctx, MODBUS_MAX_PDU_LENGTH, tab_bytes);
if (rc > 1) {
    printf("Run Status Indicator: %s\n", tab_bytes[1] ? "ON" : "OFF");
}
```
