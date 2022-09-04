# modbus_mapping_new

## Name

modbus_mapping_new - allocate four arrays of bits and registers

## Synopsis

```c
modbus_mapping_t* modbus_mapping_new(int nb_bits, int nb_input_bits, int nb_registers, int nb_input_registers);
```

## Description

The *modbus_mapping_new()* function shall allocate four arrays to store bits,
input bits, registers and inputs registers. The pointers are stored in
modbus_mapping_t structure. All values of the arrays are initialized to zero.

This function is equivalent to a call of the
[modbus_mapping_new_start_address](modbus_mapping_new_start_address.md) function
with all start addresses to `0`.

If it isn't necessary to allocate an array for a specific type of data, you can
pass the zero value in argument, the associated pointer will be NULL.

This function is convenient to handle requests in a Modbus server/slave.

## Return value

The function shall return the new allocated structure if successful. Otherwise
it shall return NULL and set errno.

## Errors

- *ENOMEM*, not enough memory.

## Example

```c
/* The first value of each array is accessible from the 0 address. */
mb_mapping = modbus_mapping_new(
    BITS_ADDRESS + BITS_NB,
    INPUT_BITS_ADDRESS + INPUT_BITS_NB,
    REGISTERS_ADDRESS + REGISTERS_NB,
    INPUT_REGISTERS_ADDRESS + INPUT_REGISTERS_NB
);
if (mb_mapping == NULL) {
    fprintf(
        stderr, "Failed to allocate the mapping: %s\n",
        modbus_strerror(errno)
    );
    modbus_free(ctx);
    return -1;
}
```

## See also

- [modbus_mapping_free](modbus_mapping_free.md)
- [modbus_mapping_new_start_address](modbus_mapping_new_start_address.md)
