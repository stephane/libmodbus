# Instructions to compile on Windows

## Create a new Visual Studio project with the library included

Download the latest version of libmodbus source code from Github
`https://github.com/stephane/libmodbus` -> Code -> Download ZIP.

Once the archive is decompressed, launch a Windows terminal (`cmd`) in
`src/win32` directory and run `cscript configure.js`.

1. copy the file `config.h` from `src/win32` to `src`.
2. create a new 'Console App' project under Visual Studio.
3. create a new directory called `libmodbus` inside your VS project (same level
   as the `.vcxproj` file).
4. copy all `*.c` and `*.h` from libmodbus `src` in the new `libmodbus` folder
   of your VS project.
5. copy `modbus.rc` in your VS project (same level as the `.vcxproj` file).
6. drag and drop `libmodbus/*.c` files (4) in *Solution Explorer -> Source Files*.
7. drag and drop `libmodbus/*.h` files (8) in *Solution Explorer -> Header Files*.
8. drag and drop `modbus.rc` file in *Solution Explorer -> Resource Files*.
9. check path is `#include "modbus-version.h"` in `modbus.rc`.
10. in the **Property Pages** of the project *Configuration Properties -> C/C++
    -> General -> Additional Include Directories*, add `libmodbus` folder.
11. in the **Property Pages** of the project *Configuration Properties ->
    Resources -> Additional Include Directories*, add `libmodbus` folder.
12. in the **Property Pages** of the project *Configuration Properties -> Linker
    -> Input*, define `ws2_32.lib`.
13. if required, add `_CRT_SECURE_NO_WARNINGS` to *C/C++ -> Preprocessor ->
    Preprocessor Definitions*.

## Create a libmodbus DLL

This directory contains the project file for Visual Studio to build `modbus.dll`
and import library `modbus.lib`.

In the Windows terminal, run `cscript configure.js` to generate:

- `config.h`
- `modbus-version.h` are generated using configure.js.

To write...
