# modbus_mapping_new_start_address

## Name

modbus_mapping_new_start_address - allocate four arrays of bits and registers accessible from their starting addresses

## Synopsis

```c
modbus_mapping_t* modbus_mapping_new_start_address(
    int start_bits, int nb_bits,
    int start_input_bits, int nb_input_bits,
    int start_registers, int nb_registers,
    int start_input_registers, int nb_input_registers);
```

## Description

The `modbus_mapping_new_start_address()` function shall allocate four arrays to
store bits, input bits, registers and inputs registers. The pointers are stored
in modbus_mapping_t structure. All values of the arrays are initialized to zero.

The different starting addresses make it possible to place the mapping at any
address in each address space. This way, you can give access to the clients at
values stored at high addresses without allocating memory from the address zero,
for eg. to make available registers from 340 to 349, you can use:

```c
mb_mapping = modbus_mapping_new_start_address(0, 0, 0, 0, 340, 10, 0, 0);
```

The newly created `mb_mapping` will have the following arrays:

- `tab_bits` set to NULL
- `tab_input_bits` set to NULL
- `tab_input_registers` allocated to store 10 registers (`uint16_t`)
- `tab_registers` set to NULL.

The clients can read the first register by using the address 340 in its request.
On the server side, you should use the first index of the array to set the value
at this client address:

```c
mb_mapping->tab_registers[0] = 42;
```

If it isn't necessary to allocate an array for a specific type of data, you can
pass the zero value in argument, the associated pointer will be NULL.

This function is convenient to handle requests in a Modbus server/slave.

## Return value

The `modbus_mapping_new_start_address()` function shall return the new allocated structure if
successful. Otherwise it shall return NULL and set errno.

## Errors

- *ENOMEM*, not enough memory.

## Example

```c
/* The first value of each array is accessible at the defined address.
The end address is ADDRESS + NB - 1. */
mb_mapping = modbus_mapping_new_start_address(
    BITS_ADDRESS, BITS_NB,
    INPUT_BITS_ADDRESS, INPUT_BITS_NB,
    REGISTERS_ADDRESS, REGISTERS_NB,
    INPUT_REGISTERS_ADDRESS, INPUT_REGISTERS_NB
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

- [modbus_mapping_new](modbus_mapping_new.md)
- [modbus_mapping_free](modbus_mapping_free.md)
