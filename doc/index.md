# Documentation contents

* [Introduction](#introduction) (this page)
* [SymbOS programming guide](symbos.md)
	* **Start here** for a quickstart guide to writing your first SymbOS app (both console apps to run in SymShell and windowed apps to run on the desktop).
* [System call reference](syscalls.md)
* [Special considerations](special.md) (compiler quirks, etc.)

# Introduction

Writing C will rot your brain. Only a true C programmer would look at the following code fragment and think it was an "elegant" solution to anything:

```c
addr = (((_MemStruct)membank).ptr->addr & (ver > 4 ? 7 : attr));
*((char*)(++addr)) += 'A';
```

C is a terrible, very bad, no-good language that provides you with an army of ways to shoot yourself in the foot. It is also obviously the best language for many kinds of systems programming, where we want to express ideas in a relatively readable, high-level way while retaining the knowledge of where every byte is going and why:

```c
_data char buffer[256]; // declare a 256-byte static buffer in the SymbOS "data" segment

// subroutine: AND every byte in buffer[] with the byte 'mask'.
void and_buffer(char mask) {
	int i = 0;
	while (i < sizeof(buffer)) {
		buffer[i] &= mask;
		i++;
	}
}
```

This middle-level flexibility---in addition to the vast amount of open-source code already written in C---makes C an extremely useful language for programming on 8-bit systems.

SCC is an ANSI C compiler that produces executables for [SymbOS](https://symbos.org). If you want a nice Visual Basic-style experience with a GUI form editor and event-driven programming, you should probably check out the [Quigs IDE](https://symbos.org/quigs.htm) instead. But if you already know C and want to write SymbOS software, or you want to port existing C code to SymbOS, or you just want to do something more low-level and complicated than is possible in Quigs, read on.

(This documentation assumes you are familiar with standard C syntax, particularly structs, pointers, and typecasting, as well as the various weird ways you can combine them.)

## Using SCC

### Compilation

To compile a single source file into a single SymbOS executable, just run it through `cc` on the command line:

```bash
cc file.c
```

Under the hood, SCC consists of a multi-stage toolchain typical for C compilers: a preprocessor (`cpp`), multiple compiler stages (`cc0`, `cc1`, and `cc2`), an assembler (`as`), linker (`ld`), optimizer (`copt`), and relocation tool (`reloc`), which performs the final stage of building a SymbOS executable by patching in the relocation table produced by the linker. For complex projects, these can be run independently to built multiple files and objects into a single executable. For example, to compile two source files (`file1.c` and `file2.c`) into object files (`file1.o`and `file2.o`) and link them with the system libraries into a single executable:

```bash
cc -c file1.c
cc -c file2.c
ld -o file.exe -R file.rel ..\lib\crt0.o file1.o file2.o ..\lib\libc.a ..\lib\libsym.a ..\lib\libz80.a
reloc file.exe file.rel
```

In the C world this type of modular build is usually done with a Makefile. SCC does not currently have its own `make` utility, but we can use the one from MinGW (not documented here). In practice SymbOS projects are usually small enough that we can just maintain a single main source file (potentially with `#include` directives to merge in subsidiary files) and compile it directly with `cc`.

A good way to determine what `cc` is doing under the hood (particularly for linking) is to run it with the `-V` option, which outputs each command as it is run.

### SymbOS executable options

SymbOS executables include several special resources used by the desktop. The first is the application name, which is displayed in the task manager and used as the default name when creating a shortcut. The default is just "App", but we can set this using the `-N` command line option in `cc`:

```bash
cc -N "Application Name" file.c
```

The second resource is the application icon, a 24x24 image in SGX format. The default icon resembles the SymShell icon and is suitable for console apps, but to specify our own, we can use the uppercase `-G` command line option (for 4-color icons) or lowercase `-g` option (for 16-color icons):

```bash
cc file.c -G icon.sgx -g icon16.sgx
```

Images can be converted to SGX format using software such as [MSX Viewer 5](https://marmsx.msxall.com/msxvw/msxvw5/index_en.php) (classic version). A library of generic icons can also be found on the [SymbOS website](http://symbos.org). (For 16-color icons, note that MSX Viewer 5 will generate an incorrect header; this can be fixed easily with a hex editor by deleting the first eight bytes of the file and replacing them with the ten bytes: `0C 18 18 00 00 00 00 20 01 05`. The resulting file should be exactly 298 bytes long. 4-color icons generated with MSX Viewer 5 will work unmodified.)

## Features

* Full build chain with preprocessor, object files, linker, etc.
* Standard ANSI C syntax, with good support for most typical usage.
* Standard data types and structures: `char`, `short`, `int`, `long`, `float`, `double`, `signed`, `unsigned`, `struct`, `union`, `enum`, `auto`, `static`, `register`, `extern`, `typedef`.
* A proper libc port [work in progress] so existing code can be compiled unmodified.
* Headers, typedefs, and support functions for most SymbOS system calls (`symbos.h`).
* Clean handling of SymbOS segments, with data, buffers, and literals located correctly in the executable without duplication (other compilers struggle with this). Keywords `_data` and `_transfer` allow specifying the segment of globals.
* The underlying code is 8-bit-friendly, so it should (eventually!) be possible to build SCC to run natively on SymbOS.

## Limitations

* **Not all libc functions are available, well-tested, and/or correctly implemented yet**. Platform-independent ones should all work, but any code requiring I/O or calls to the operating system should be tested carefully.
* **`float` is currently somewhat broken**; use with care. (The current implementation is horrible anyway, and should eventually be replaced with a proper Z80 floating-point library.)
* No high-level optimizations like subexpression merging. (Just write efficiently.)
* The libc implementation is not very fast and favors portability over speed.
* For the usual Z80 reasons, 8-bit arithmetic (`char`) will always be much faster than 16-bit (`int`) and particularly floating-point (`float`, `double`) arithmetic. Declaring variables `unsigned` may also improve efficiency where applicable.
* `double` is currently treated as synonymous with `float` (IEEE 754 floating-point).
* The usual SymbOS limits apply, such as 64KB and 16KB limits on particular segments (see the [SymbOS programming guide](symbos.md)).
* No named static struct initializers like `{.x = 1, .y = 2}`---use `{1, 2}` instead and just be careful about member order.
* No K&R-style function declarations.
* Not all C99 and C11 additions are supported.
* `const` and `volatile` are accepted but don't actually do anything.
* Local variables are technically scoped to their function, not their block.
* If you run into type-mismatch problems with unusual pointer data types (like struct members that are typed as pointers to another typedef'd struct---this can happen when working with complex data structures for windows), try `void*` instead.

