# modbus_rtu_set_slave_filter

## Name

modbus_rtu_set_slave_filter - set the unit identifiers accepted in RTU slave mode

## Synopsis

```c
int modbus_rtu_set_slave_filter(modbus_t *ctx, const uint8_t *slaves, unsigned int count);
```

## Description

The *modbus_rtu_set_slave_filter()* function shall configure the set of Modbus
unit identifiers accepted by an RTU context in slave mode. By default an RTU
context only handles indications addressed to the single slave set with
[modbus_set_slave](modbus_set_slave.md); a filter lets one context serve several
slaves on the same serial link, each typically backed by its own data mapping.

*slaves* points to an array of *count* unit identifiers to accept. The broadcast
address is always accepted regardless of the filter. Passing a NULL *slaves* or
a *count* of 0 clears the filter and restores the default single-slave
behaviour.

When a filter is set, an indication addressed to a unit identifier outside the
set is ignored (*modbus_receive()* returns 0) and the context keeps listening
for the next indication.

## Return value

The *modbus_rtu_set_slave_filter()* function shall return 0 if successful.
Otherwise it shall return -1 and set errno to EINVAL if *ctx* is not an RTU
context or if a unit identifier is out of range.

## Example

```c
ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);

/* Serve unit identifiers 5, 6 and 7 from this context. */
uint8_t slaves[] = {5, 6, 7};
modbus_rtu_set_slave_filter(ctx, slaves, 3);

modbus_connect(ctx);

for (;;) {
    uint8_t req[MODBUS_RTU_MAX_ADU_LENGTH];
    int rc = modbus_receive(ctx, req);
    if (rc > 0) {
        modbus_reply(ctx, req, rc, mapping_for(req[0]));
    }
}
```

## See also

- [modbus_set_slave](modbus_set_slave.md)
- [modbus_new_rtu](modbus_new_rtu.md)
- [modbus_receive](modbus_receive.md)
- [modbus_reply](modbus_reply.md)
