# modbus_disable_quirks

## Name

modbus_disable_quirks - disable a list of quirks according to a mask

## Synopsis

```c
int modbus_disable_quirks(modbus_t *ctx, unsigned int quirks_mask);
```

## Description

The function shall disable the quirks according to the provided mask. It's
useful to revert changes applied by a previous call to
[modbus_enable_quirks](modbus_enable_quirks.md)

To reset all quirks, you can use the specific value `MODBUS_QUIRK_ALL`.

```c
modbus_enable_quirks(ctx, MODBUS_QUIRK_MAX_SLAVE | MODBUS_QUIRK_REPLY_TO_BROADCAST);

...

// Reset all quirks
modbus_disable_quirks(ctx, MODBUS_QUIRK_ALL);
```

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno.

## See also

- [modbus_enable_quirks](modbus_enable_quirks.md)
