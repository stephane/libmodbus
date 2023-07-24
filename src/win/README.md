# Instructions to compile on Windows

## Create a new Visual Studio project with the library included

This is solution is build and tested in Visual Studio 2022 CE.

Clone the latest version of libmodbus source code from Github.

1. Open libmodbus solution in VS (`libmodbus.sln`).
2. Use Powershell terminal in VS and run `cscript configure.js` (You should already be in the `src/win` directory).
3. Build solution ("CTRL+SHIFT+B").
