# SymbOS C Compiler

A C compiler for SymbOS based on Alan Cox's Fuzix Compiler Kit (Z80 version).
Currently it runs on Windows and cross-compiles binaries for SymbOS. (Eventually it
should be possible to cross-compile itself to run natively on SymbOS.)

Build with mingw32 using the included batch files. To compile a single file:

```cc source.c```

The libc port is a work in progress; most platform-independent functions (strcpy,
asin, etc.) should work, but system-specific functions will not work. In particular,
most of stdio (fwrite, printf, etc.) will not work. Use the equivalent SymbOS system
functions (File_Write, Shell_StringOut, etc.) instead; see the SymbOS developer's
documentation and the system header file:

```#include <symbos.h>```

# License

Compiler is GPLv3. Libraries are LGPL except where indicated. copt is from Z88DK under
the Clarified Artistic License. cpp is from MCPP under the BSD license. Sample code is
public domain.

