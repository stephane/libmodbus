# modbus_strerror

## Name

modbus_strerror - return the error message

## Synopsis

```c
const char *modbus_strerror(int errnum);
```

## Description

The *modbus_strerror()* function shall return a pointer to an error message
string corresponding to the error number specified by the `errnum` argument.  As
libmodbus defines additional error numbers over and above those defined by the
operating system, applications should use *modbus_strerror()* in preference to
the standard *strerror()* function.

## Return value

The *modbus_strerror()* function shall return a pointer to an error message
string.

## Errors

No errors are defined.

## Example

Display an error message when a Modbus connection cannot be established

```c
if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    abort();
}
```
