# modbus_flush

## Name

modbus_flush - flush non-transmitted data

## Synopsis

```c
int modbus_flush(modbus_t *ctx);
```

## Description

The *modbus_flush()* function shall discard data received but not read to the
socket or file descriptor associated to the context 'ctx'.

## Return value

The function shall return 0 or the number of flushed bytes if
successful. Otherwise it shall return -1 and set errno.
