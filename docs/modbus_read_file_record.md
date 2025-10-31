# modbus_read_file_record

## Name

modbus_read_file_record - Read records from a file

## Synopsis

```c
int modbus_read_file_record(modbus_t *'ctx', int 'addr', int 'sub_addr', int 'nb', uint16_t * 'dest');
```

## Description

The *modbus_read_file_record()* function reads `nb` records from file
number `addr`, starting from record position `sub_addr` in the file.
The result of reading is stored in `dest` array as word values (16 bits). You
must take care to allocate enough memory to store the results in `dest`
- (at least `nb * sizeof(uint16_t)`).

A file is an array of records, each of 16 bits.

* A ModBus device may have up to 65535 files, addressed 1 to 65535 decimal.
* Each file contains 10000 records, addressed 0000 to 9999 decimal.

A maximum of 124 records (`nb`) may be retrieved in a single request.

This function uses the ModBus function code 0x14 (Read File Record). Note that
although the ModBus Specification allows for multiple non-contiguous reads in
the same file to be made in a single request, this function only supports a
single contiguous read request.


## Return value

The function shall return the number of records read (i.e. the value of `nb`) if successful.
Otherwise it shall return -1 and set errno.


## See also

- [modbus_read_register](modbus_read_register.md)
- [modbus_write_file_record](modbus_write_file_record.md)
