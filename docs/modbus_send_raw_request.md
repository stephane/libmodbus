# modbus_send_raw_request

## Name

modbus_send_raw_request - send a raw request

## Synopsis

```c
int modbus_send_raw_request(modbus_t *ctx, const uint8_t *raw_req, int raw_req_length);
```

## Description

The *modbus_send_raw_request()* function shall send a request via the socket of
the context `ctx`. This function must be used for debugging purposes because you
have to take care to make a valid request by hand. The function only adds to the
message, the header or CRC of the selected backend, so `raw_req` must start and
contain at least a slave/unit identifier and a function code. This function can
be used to send request not handled by the library.

The public header of libmodbus provides a list of supported Modbus functions
codes, prefixed by `MODBUS_FC_` (eg. `MODBUS_FC_READ_HOLDING_REGISTERS`), to help
build of raw requests.

## Return value

The function shall return the full message length, counting the extra data
relating to the backend, if successful. Otherwise it shall return -1 and set
errno.

## Example

```c
modbus_t *ctx;
/* Read 5 holding registers from address 1 */
uint8_t raw_req[] = { 0xFF, MODBUS_FC_READ_HOLDING_REGISTERS, 0x00, 0x01, 0x0, 0x05 };
int req_length;
uint8_t rsp[MODBUS_TCP_MAX_ADU_LENGTH];

ctx = modbus_new_tcp("127.0.0.1", 1502);
if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

req_length = modbus_send_raw_request(ctx, raw_req, 6 * sizeof(uint8_t));
modbus_receive_confirmation(ctx, rsp);

modbus_close(ctx);
modbus_free(ctx);
```

## See also

- [modbus_receive_confirmation](modbus_receive_confirmation.md)
