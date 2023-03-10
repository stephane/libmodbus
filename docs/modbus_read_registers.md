# modbus_read_registers

## Name

modbus_read_registers - read many registers

## Synopsis

```c
int modbus_read_registers(modbus_t *ctx, int addr, int nb, uint16_t *dest);
```

## Description

The *modbus_read_registers()* function shall read the content of the `nb`
holding registers to the address `addr` of the remote device. The result of
reading is stored in `dest` array as word values (16 bits).

You must take care to allocate enough memory to store the results in `dest`
(at least `nb * sizeof(uint16_t)`).

The function uses the Modbus function code 0x03 (read holding registers).

## Return value

The function shall return the number of read registers
if successful. Otherwise it shall return -1 and set errno.

## Errors

- *EMBMDATA*, too many registers requested.

## Example

```c
modbus_t *ctx;
uint16_t tab_reg[64];
int rc;
int i;

ctx = modbus_new_tcp("127.0.0.1", 1502);
if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
}

rc = modbus_read_registers(ctx, 0, 10, tab_reg);
if (rc == -1) {
    fprintf(stderr, "%s\n", modbus_strerror(errno));
    return -1;
}

for (i=0; i < rc; i++) {
    printf("reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
}

modbus_close(ctx);
modbus_free(ctx);
```

## See also

- [modbus_write_register](modbus_write_register.md)
- [modbus_write_registers](modbus_write_registers.md)
