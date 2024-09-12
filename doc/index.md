# Documentation contents

* [Introduction](#introduction) (this page)
* [SymbOS programming guide](symbos.md)
	* **Start here** for a quickstart guide to writing your first SymbOS app (both console apps to run in SymShell and windowed apps to run on the desktop).
* [System call reference](syscalls.md)
* [Special considerations](special.md) (compiler quirks, etc.)

# Introduction

SCC is an ANSI C compiler that produces executables for [SymbOS](https://symbos.org).

SCC is not the only way to write code for SymbOS; for a more Visual Basic-like experience with a GUI form editor and event-driven programming (albeit with sparse documentation and a lot of quirks), check out the [Quigs IDE](https://symbos.org/quigs.htm). There is also [extensive documentation and example code](https://symbos.org/download.htm) for writing SymbOS applications in pure Z80 assembler. But if you already know C and want to write SymbOS software, or you want to port existing C code to SymbOS, or you just want something more powerful than Quigs but less mind-numbing than assembly, read on.

(This documentation assumes you are familiar with standard C syntax, particularly structs, pointers, and typecasting, as well as the various weird ways you can combine them.)

## Why C?

Writing C will rot your brain. Only a true C programmer would look at the following code and think it was an "elegant" solution to anything:

```c
addr = ((_MemStruct)membank).ptr->addr & (ver > 4 ? 7 : attr);
*((char*)(++addr)) += 'A';
```

C is an awful language that is nonetheless ideal for many kinds of 8-bit programming, where we want to express ideas in a relatively high-level way while retaining the knowledge of where every byte is going and why. C is particularly good at dealing with byte-level structured data and pointers, which makes it a good fit for SymbOS. For example, SymbOS GUI controls are defined by exact blocks of bytes laid out in the correct order. These can be conveniently represented in C as structs, allowing us to reference the control's properties by name:

```c
typedef struct {
    char* text;               // address of text buffer
    unsigned short scroll;    // scroll position, in bytes
    unsigned short cursor;    // cursor position, in bytes
    signed short selection;   // number of selected characters relative to cursor
    unsigned short len;       // current text length, in bytes
    unsigned short maxlen;    // maximum text length, in bytes
    unsigned char flags;      // flags
    unsigned char textcolor;  // text color
    unsigned char linecolor;  // line color
} Ctrl_Input;
```

With this type of intrastructure in place, we can express complex ideas in just a few lines of code:

```c
// declare a 256-byte static buffer in the SymbOS "data" segment
_data char buffer[256];

// subroutine: convert the text in the specified GUI textbox (passed by reference)
// to a number, then AND it with every byte in buffer[].
void filter_buffer(Ctrl_Input* textbox) {
	int i = 0;
	char mask = atoi(textbox->text);
	while (i < sizeof(buffer)) {
		buffer[i] &= mask;
		i++;
	}
}
```

C is also a mature language with a vast amount of open-source code written in it, so there is ample material for porting or adapting and it is possible to find code examples for just about every algorithm imaginable.

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

### Compilation on SymbOS

Using SCC natively on SymbOS is similar to other platforms; just invoke `cc` from SymShell. See `install.txt` in the SymbOS release folder for some important details on how to install SCC on your machine.

Compilation on an original 4 MHz machine (CPC, MSX, etc.) currently runs at "take a coffee break" speeds, although this will hopefully continue to improve in future releases. Compilation times can be improved by breaking programs into small modules and only recompiling the modules that have changed (see the example in the prior section), as well as by only including the minimum number of necessary header files (e.g., `symbos/shell.h` instead of the entire `symbos.h`). This may also be necessary to prevent SCC from running out of memory.

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

* Runs natively on Windows and SymbOS.
* Full build chain with preprocessor, object files, linker, etc.
* Standard ANSI C syntax, with good support for most typical usage.
* Standard data types and structures: `char`, `short`, `int`, `long`, `float`, `double`, `signed`, `unsigned`, `struct`, `union`, `enum`, `auto`, `static`, `register`, `extern`, `typedef`.
* A proper libc port [work in progress] so existing code can be compiled unmodified.
* Headers, typedefs, and support functions for most SymbOS system calls (`symbos.h`).
* Clean handling of SymbOS segments, with data, buffers, and literals located correctly in the executable without duplication (other compilers struggle with this). Keywords `_data` and `_transfer` allow specifying the segment of globals.
* Multithreading (yes, [really](syscalls.md#multithreading)!)

## Limitations

* **`float` is currently somewhat broken**; use with care. (The current implementation is horrible anyway, and should eventually be replaced with a proper Z80 floating-point library.)
* **Not all libc functions are available, well-tested, and/or correctly implemented yet**. The libc implementation is robust enough for SCC to compile itself, but you may encounter subtle incompatibilities and any code requiring less-common system functions should be tested carefully.
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

