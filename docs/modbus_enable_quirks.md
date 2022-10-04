# modbus_enable_quirks

## Name

modbus_enable_quirks - enable a list of quirks according to a mask

## Synopsis

```c
int modbus_enable_quirks(modbus_t *ctx, unsigned int quirks_mask);
```

## Description

The function is only useful when you are confronted with equipment which does
not respect the protocol, which behaves strangely or when you wish to move away
from the standard.

In that case, you can enable a specific quirk to workaround the issue, libmodbus
offers the following flags:

- `MODBUS_QUIRK_MAX_SLAVE` allows slave adresses between 247 and 255.
- `MODBUS_QUIRK_REPLY_TO_BROADCAST` force a reply to a broacast request when the
  device is a slave in RTU mode (should be enabled on the slave device).

You can combine the flags by using the bitwise OR operator.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno.

## See also

- [modbus_disable_quirks](modbus_disable_quirks.md)
