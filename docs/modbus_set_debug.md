# modbus_set_debug

## Name

modbus_set_debug - set debug flag of the context

## Synopsis

```c
int modbus_set_debug(modbus_t *ctx, int flag);
```

## Description

The *modbus_set_debug()* function shall set the debug flag of the *modbus_t*
context by using the argument `flag`. By default, the boolean flag is set to
`FALSE`. When the `flag` value is set to `TRUE`, many verbose messages are
displayed on stdout and stderr. For example, this flag is useful to display the
bytes of the Modbus messages.

```text
[00][14][00][00][00][06][12][03][00][6B][00][03]
Waiting for a confirmation...
<00><14><00><00><00><09><12><03><06><02><2B><00><00><00><00>
```

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set errno.
