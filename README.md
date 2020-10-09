# Raspberry Pi fork of libmodbus with GPIO rx-tx functionality with RS485 chips or similar

## When it should be used :

If you're using an UART to RS485 conversion chip like 75176 or anything similar and using one of the RPi GPIO
to switch rx - tx functionality of the chip.

In addition to standard libmodbus API, addtional APIs are included to support GPIO based rx-tx switching

## Additional APIs :

- `modbus_enable_rpi`                 : Sets an enable Rpi integer in ctx data structure.
- `modbus_configure_rpi_bcm_pin`      : Configures GPIO BCM pin to be used for rx-tx switching (if the RE and DE are the same)
- `modbus_configure_rpi_bcm_pins`     : Configures GPIO BCM pins (RE & DE) to be used for rx-tx switching (if the RE and DE are not on the same pin)
- `modbus_rpi_pin_export_direction`   : export configured GPIO, configures it as an output.
- `modbus_rpi_pin_unexport_direction` : unexport configured GPIO, configures it as an input.

## Usage :

### While initializing :

- `modbus_enable_rpi(ctx);`
- `modbus_configure_rpi_bcm_pin(ctx,18);   // configures BCM pin # 18 for DE & RE used for switching`
- `modbus_configure_rpi_bcm_pins(ctx,17,18);   // configures BCM pin # 17 for DE and # 18 for RE used for switching`
- `modbus_rpi_pin_export_direction(ctx);   // exports configured BCM pin #`

### While clean up :

- `modbus_rpi_pin_unexport_direction(ctx); //unexport configured BCM pin #`

### Examples

See test.c in rpi-test folder for template.
