modbus_write_file_record

## Name

modbus_write_file_record - Write records to a file

## Synopsis

```c
int modbus_write_file_record(modbus_t *'ctx', int 'addr', int 'sub_addr', int 'nb', const uint16_t * 'src');
```

## Description

The *modbus_write_file_record()* function writes the content of `nb`
records from the `src` array to file number `addr`, starting from position
`sub_addr` in the file.

A file is an array of records, each of 16 bits.

* A ModBus device may have up to 65535 files, addressed 1 to 65535 decimal.
* Each file contains 10000 records, addressed 0000 to 9999 decimal.

A maximum of 124 records (`nb`) may be written in a single request.

This function uses the ModBus function code 0x15 (Write File Record). Note that
although the ModBus Specification allows for multiple non-contiguous writes in
the same file to be made in a single request, this function only supports a
single contiguous write request.

## Return value

The function shall return the number of records written (i.e. the value of `nb`) if successful.
Otherwise it shall return -1 and set errno.

## See also

- [modbus_write_register](modbus_write_register.md)
- [modbus_read_file_record](modbus_read_file_record.md)
