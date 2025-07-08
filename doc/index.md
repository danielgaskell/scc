# Documentation contents

* [Introduction](#introduction) (this page)
* [SymbOS programming guide](symbos.md#symbos-programming)
	* **Start here** for a quickstart guide to writing your first SymbOS app (both console apps to run in SymShell and windowed apps to run on the desktop).
* [System call reference](syscall1.md#system-call-reference)
* [Graphics library](graphics.md#graphics-library)
* [Network library](network.md#network-library)
* [Special considerations](special.md#special-considerations) (compiler quirks, etc.)
* [Porting tips](porting.md#porting-tips)

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

For single files, this is all we need to know. For more complex projects, it is helpful to understand how the C build chain works. Under the hood, SCC consists of a multi-stage toolchain typical for C compilers: a preprocessor (`cpp`), multiple compiler stages (`cc0`, `cc1`, and `cc2`), an assembler (`as`), linker (`ld`), optimizer (`copt`), and a relocation table builder (`reloc`). Generally speaking, after preprocessing, source files (`.c`) are compiled to assembly files (`.s`), which are then assembled into object files (`.o`). The linker (`ld`) then links these object files together with each other (and with the relevant functions from the standard library) to produce a single executable.

For organization and to improve compilation speed (particularly when compiling natively on SymbOS), projects can be broken up into multiple source files (`.c` or `.s` for assembly-language source files). If we pass multiple source files to `cc`, it will compile or assemble them into object files, then link the resulting object files into an executable:

```bash
cc file1.c file2.c asmfile.s
```

However, we can also skip the linking step by using the `-c` command-line option. This lets us only recompile the specific modules we have changed, saving a lot of time when building large projects natively on SymbOS. For example, to compile just the single source file `file1.c` into the object file `file1.o`:

```bash
cc -c file1.c
```

Then, to link all the relevant object files into a single executable:

```bash
cc -o file.exe file1.o file2.o asmfile.o
```

As seen in the above example, the `-o` option can be used to manually specify what the resulting executable should be named (e.g., `file.exe`).

In the C world this type of modular build is usually done with a Makefile. SCC does not currently have its own `make` utility, but we can use the one from MinGW (not documented here). In practice SymbOS projects are usually small enough that, for normal desktop cross-compilation, we can just maintain a single main source file (potentially with `#include` directives to merge in subsidiary files) and compile it directly with `cc`.

The `cc` app is usually the most convenient way to organize modular builds, but we can also run the stages separately if we know what we are doing. (A good way to determine what `cc` is doing under the hood is to run it with the `-V` option, which outputs each subcommand as it is run.)

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

* Runs natively on Windows, Linux, and SymbOS.
* Full build chain with preprocessor, object files, linker, etc.
* Standard ANSI C syntax, with good support for most typical usage.
* Standard data types and structures: `char`, `short`, `int`, `long`, `float`, `double`, `signed`, `unsigned`, `struct`, `union`, `enum`, `auto`, `static`, `register`, `extern`, `typedef`.
* A proper libc port so existing code can be compiled unmodified.
* Headers, typedefs, and support functions for most SymbOS system calls (`symbos.h`).
* Clean handling of SymbOS segments, with data, buffers, and literals located correctly in the executable without duplication (other compilers struggle with this). Keywords `_data` and `_transfer` allow specifying the segment of globals.
* Multithreading (yes, [really](syscall2.md#multithreading)!)

## Limitations

* **Not all libc functions are available, well-tested, and/or correctly implemented yet**. The libc implementation is robust enough for SCC to compile itself, but you may encounter subtle incompatibilities and any code requiring less-common system functions should be tested carefully.
* Symbol matching only considers the first 15 characters (to conserve memory on native builds).
* No high-level optimizations like subexpression merging. (Just write efficiently.)
* The libc implementation is not very fast and favors portability over speed.
* For the usual Z80 reasons, 8-bit arithmetic (`char`) will always be much faster than 16-bit (`int`) and particularly floating-point (`float`, `double`) arithmetic. Declaring variables `unsigned` may also improve efficiency where applicable.
* `double` is currently treated as synonymous with `float` (32-bit IEEE 754 floating-point).
* The usual SymbOS limits apply, such as 64KB and 16KB limits on particular segments (see the [SymbOS programming guide](symbos.md#symbos-programming)).
* No named static struct initializers like `{.x = 1, .y = 2}`---use `{1, 2}` instead and just be careful about member order.
* No K&R-style function declarations.
* Not all C99 and C11 additions are supported.
* `const` and `volatile` are accepted but don't actually do anything.
* Local variables are technically scoped to their function, not their block.
* If you run into type-mismatch problems with unusual pointer data types (like struct members that are typed as pointers to another typedef'd struct---this can happen when working with complex data structures for windows), try `void*` instead.

## Some advice on workflow

Development will be much, **much** easier if you cross-compile on a PC and test on an emulator. Working natively on SymbOS sounds cool---and improving the native experience is a long-term goal---but for now, the slow compilation speeds and lack of a good IDE quickly get frustrating. Setting up an efficient cross-compilation workflow will dramatically improve your experience with SCC.

### Testing on an emulator

Some example workflows using [WinApe](http://www.winape.net/) on Windows:

1. Easy: Run SymbOS in WinApe (e.g., from a floppy or hard disk image). Insert a blank, formatted disk image into one of WinApe's virtual floppy drives, and keep the "Edit Disk" window open. Whenever you compile a new version of your app, just drag-and-drop it into the Edit Disk window and run it from SymbOS.

2. Advanced: Create a FAT16 or FAT32 partition on your hard drive and mount it as a hard drive within WinApe (Settings > Other > Logical Drive). Unpack the SymbOS installation packages to this partition and install the SymbOS CPC ROM images to slots Upper 1/2/3/4 in WinApe. You should now be able to run SymbOS directly off of your hard drive, with both Windows and SymbOS seeing the same files simultaneously. Just compile on Windows and run the resulting executable in-place on SymbOS!

Similar workflows are possible with other emulators, depending on setup. For developing [network](network.md) apps, note especially that [CPCEMU](https://www.cpc-emu.org/) can emulate the M4 Board network expansion.

### Editors

SCC code tends to use a lot of SymbOS-specific system calls, so it's worth using a good modern IDE with code-completion---it will drastically reduce the number of times you have to look up syntax in the manual. ([Code::Blocks](https://www.codeblocks.org/) and Visual Studio Code are both popular.) In Code::Blocks, you should add SCC's `lib/include` directory to the search path to enable code-completion for SCC-specific functions (Settings > Compiler > Search directories > Compiler > Add).
