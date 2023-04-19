# Instructions to compile on Windows
To use this library on Windows, we have to generate `.dll` file out of this code. On successful build we will get a set of 3 files :
- modbus.dll
- libmodbus.a
- libmodbus.def

You use `libmodbus.a` when you write your code and compile. While running your executable you use `modbus.dll` either by copying it in the same folder where executable is or by adding `path-to-modbus.dll` in the `ENVIRONMENT VARIABLES - PATH` of Windows.


Currently, to compile and build this code you have 3 major options.

1. You can use `Visual Studio` and create a project from `modbus.vcproject` in `src/win32` folder
2. Use some other IDE and import this as a `Visual Studio Project` in that IDE
3. Create a `dll project` in any IDE and build the code from scratch (this option comes if import is not available or successful) 


## First do this

In all three cases you have to do the following steps :

- Download the latest version of libmodbus source code from Github
`https://github.com/stephane/libmodbus` -> Code -> Download ZIP.

- Once the archive is decompressed, launch a Windows terminal (`cmd`) in
`src/win32` directory and run `cscript configure.js`.

- Copy the generated file `config.h` from `src/win32` to `src`.

- Proceed with one of the 3 options below


## Option 1: Create a new Visual Studio project with the library included

1. create a new 'Console App' project under Visual Studio.
2. create a new directory called `libmodbus` inside your VS project (same level
   as the `.vcxproj` file).
3. copy all `*.c` and `*.h` from libmodbus `src` in the new `libmodbus` folder
   of your VS project.
4. copy `modbus.rc` in your VS project (same level as the `.vcxproj` file).
5. drag and drop `libmodbus/*.c` files (4) in *Solution Explorer -> Source Files*.
6. drag and drop `libmodbus/*.h` files (8) in *Solution Explorer -> Header Files*.
7. drag and drop `modbus.rc` file in *Solution Explorer -> Resource Files*.
8. check path is `#include "modbus-version.h"` in `modbus.rc`.
9. in the **Property Pages** of the project *Configuration Properties -> C/C++
    -> General -> Additional Include Directories*, add `libmodbus` folder.
10. in the **Property Pages** of the project *Configuration Properties ->
    Resources -> Additional Include Directories*, add `libmodbus` folder.
11. in the **Property Pages** of the project *Configuration Properties -> Linker
    -> Input*, define `ws2_32.lib`.
12. if required, add `_CRT_SECURE_NO_WARNINGS` to *C/C++ -> Preprocessor ->
    Preprocessor Definitions*.


## Option 2: Import as Visual Studio project in any other IDE

After you have generated `config.h` file using cscript and moved it to `src` folder, check in your IDE if there is an option to `import Visual Studio project` (CodeBlocks has it).

1. Select `modbus.vcproject` as file to import from (this file is in `src/win32` folder). You might get some errors while importing but check the severity of the error before proceeding.
2. Build the project and you will get set of 3 files in `src/win32`(`modbus.dll`, `libmodbus.a`, `libmodbus.def`).


## Option 3: Build the library as DLL project

After you have generated `config.h` file using `cscript` and copied it to `src` folder,

1. Create a new project as DLL (Dynamic Link Library) and select `src` as the project directory
2. Delete generated `.c` `.cpp` and `.h` files (Codeblocks generated `main.cpp` and `main.h` which have to be deleted).
3. Add all the .c and .h files of libmodbus to this project as source and header files.
4. Add `src` as the `include directory` of the project (we still don't have separate `include directory`). 
5. Add `ws2_32` library to the linker library
6. On successful build, set of 3 files (`modbus.dll`, `libmodbus.a`, `libmodbus.def`) will be generated in `bin/Debug` or somewhere similar.


# Using generated library

1. To use the generated library create a separate `Console project`.
2. Add the `src` folder of libmodbus library as the `Include directory` for your project or use the `libmodbus` directory that has all the header files.
3. Link `libmodbus.a` in linker setting before generating the `.exe` file for your project.
4. Build the project to generate an `.exe` file for this project.
5. For running the `.exe` file of your test project, place the `modbus.dll` in the same folder as the `.exe` file. Or, alternatively add the path to `modbus.dll` in the `ENVIRONMENT VARIABLES - PATH` in Windows.
6. Run your `.exe` file
