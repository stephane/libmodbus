# modbus_strerror_r

## Name

modbus_strerror_r - return the error message

## Synopsis

```c
const char *modbus_strerro_r(int errnum, char buf[.buflen], size_t buflen);
```

## Description

The *modbus_strerror_r()* function is a reentrant replacement for
*modbus_strerror()*.

The *modbus_strerror_r()* function returns a pointer to a string containing
the error message. This may either be a pointer to a string that the
function stores in *buf*, or a pointer to some (immutable) static string
(in which case *buf* is unused). If the function stores a string in *buf*,
then at most *buflen* bytes are stored. The string may be truncated if
*buflen* is tool small and errnum is unknown. The string always includes a
terminating null byte ('\0').

As libmodbus defines additional error numbers over and above those defined
by the operating system, applications should use *modbus_strerror_r()* in
preference to the standard *strerror_r()* function.

## Return value

The *modbus_strerror()* function shall return a pointer to an error message
string.

## Errors

No errors are defined.

## Example

Display an error message when a Modbus connection cannot be established

```c
if (modbus_connect(ctx) == -1) {
    char buf[128]
    fprintf(stderr, "Connection failed: %s\n",
        modbus_strerror_r(errno, buf, sizeof(buf)));
    abort();
}
```
