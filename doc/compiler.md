# Using the compiler

## Contents

* [Basic operation](#basic-operation)
* [Multi-file projects](#multi-file-projects)
* [Installation](#installation)
	* [Windows](#windows)
	* [Linux/MacOS](#linuxmacos)
	* [SymbOS](#symbos)
* [Compiler options](#compiler-options)
	* [SymbOS executable options](#symbos-executable-options)
	* [Other options](#other-options)
* [Interfacing with assembly](#interfacing-with-assembly)
* [Some advice on workflow](#some-advice-on-workflow)

## Basic operation

To compile a single source file into a single SymbOS executable, just run it through `cc` on the command line:

```bash
cc windemo.c
```

That's it! Enjoy your executable.

## Multi-file projects

For more complex projects, it is helpful to understand how the C build chain works. Under the hood, SCC consists of a multi-stage toolchain typical for C compilers: a preprocessor (`cpp`), multiple compiler stages (`cc0`, `cc1`, and `cc2`), an assembler (`as`), linker (`ld`), optimizer (`copt`), and a relocation table builder (`reloc`). Generally speaking, after preprocessing, source files (`.c`) are compiled to assembly files (`.s`), which are then assembled into object files (`.o`). The linker (`ld`) then links these object files together with each other (and with the relevant functions from the standard library) to produce a single executable.

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

## Installation

### Windows

To install on Windows, just download the latest [binary release](https://github.com/danielgaskell/scc/releases/) for Windows and unzip it to a convenient folder. SCC has been tested to work on Windows 98SE through Windows 11.

### Linux/MacOS

Using SCC on Linux is possible, but currently requires building from source:

1. Download the latest [source release](https://github.com/danielgaskell/scc/releases/) and unzip it to a convenient folder.
2. Ensure you have `gcc` installed.
3. On the command line, navigate to the `src` subfolder and run `./makelinux.sh`. (It may be necessary to run `chmod +x makelinux.sh` first.)

This process currently builds SCC in-place (in the source repository's `bin` subfolder) rather than installing to some universally accessible directory like `/usr/bin`. Once built, `cc` expects to be run directly from this `bin` subfolder, or it will not be able to find its own libraries.

### SymbOS

SCC can run natively on SymbOS! This is cool, but much too slow for serious development---expect multiple minutes to compile a simple "hello world" program. For this reason, it's currently recommended to [set up a good cross-compilation environment](#some-advice-on-workflow) rather than trying to do all your development on SymbOS. But if you insist, see [below](#native-compilation) for some suggestions.

To install on SymbOS, just download the latest [binary release](https://github.com/danielgaskell/scc/releases/) for SymbOS, unzip it, and copy the entire directory tree onto a sufficiently large FAT-formatted drive (such as a mass storage device). Once installed, you can just run `cc` from SymShell like you would on other platforms.

**Warning:** The directory structure must remain exactly as it is in the archive! SCC will not run if all the files are placed in a single folder.

**Warning:** SCC **must** be run from a FAT-formatted drive! The filesytem used on CP/M and Amstrad floppies internally pads files to the nearest 128 bytes, which will cause problems for utilities (like `ld`) that need to know the length of binary files to the exact byte. This also means that transferring the object and library files in the `lib` subfolder (`.o`, `.a`) via an AMSDOS floppy or floppy image **will corrupt them**. If there is no other way to transfer these files to their final FAT-formatted drive, a good workaround is to compress them into a `.zip` file and decompress them at their final destination using the SymbOS `unzip.com` utility.

It is recommended to install SCC to a relatively "root" subfolder (such as `C:\SCC`) because `cc.com` will use this absolute path repeatedly in long SymShell commands, which may overflow if the base path is very long. To allow CC to be run directly in SymShell without specifying its full path, the base SCC folder can be the SymbOS system folder (e.g., `C:\SYMBOS`, so the path of `cc` is `C:\SYMBOS\CC.COM` and SCC's libraries are in subfolders like `C:\SYMBOS\LIB`).

## Compiler options

### SymbOS executable options

SymbOS executables include several special resources used by the desktop. These can be specified using command-line arguments passed to `cc`:

* `-N "appname"` - specifies the application name shown in the task manager.
* `-G iconfile` - specifies the 4-color application icon (see below).
* `-g iconfile` - specifies the 16-color application icon (see below).

For example:

```bash
cc file.c -N "Application Name" -G icon4.sgx -g icon16.sgx
```

Icons are 24x24 images in `.sgx` format. The default icon resembles the SymShell icon and is suitable for console apps, but we can create our own using software such as [MSX Viewer 5](https://marmsx.msxall.com/msxvw/msxvw5/index_en.php) (classic version) or the `gfx2sgx` tool included with the [graphics library](graphics.md). For example, to convert a 24x24-pixel image called `icon4.png` into a 4-color `.sgx` icon:

```bash
gfx2sgx icon4.png -4
```

A library of generic icons can also be found on the [SymbOS website](http://symbos.org).

**Note**: MSX Viewer 5 generates an incorrect header for 16-color icons. This can be fixed easily with a hex editor by deleting the first eight bytes of the file and replacing them with the ten bytes: `0C 18 18 00 00 00 00 20 01 05`. The resulting file should be exactly 298 bytes long. 4-color icons generated with MSX Viewer 5 will work unmodified.

### Other options

* `-c` - compile to object modules only (`.o` files) without linking; see [Multi-File Projects](#multi-file-projects) for details.
* `-D x` - define the macro `x` for the preprocessor; e.g., `-D TESTBUILD` will act like the preprocessor directive `#define TESTBUILD`.
* `-E` - preprocess only, do not compile. Output will be saved to the temporary file `$stream0.c`.
* `-I x` - adds a directory to the include path; e.g., `-I C:\MYPROJ` will add `C:\MYPROJ` to the list of directories searched when resolving an `#include` directive.
* `-h x` - set the `malloc()` heap size to `x` bytes (default 4096); see [here](special.md#the-malloc-heap) for discussion.
* `-l x` - link against the library `x` (physically stored as the file `libx.a` in SCC's `lib` subfolder); e.g., `-lnet` links against `libnet.a`, giving us access to functions in the [network library](network.md).
* `-L x` - adds a directory to the library path; e.g., `-L C:\MYPROJ` will add `C:\MYPROJ` to the list of directories searched when resolving a `-l` command-line option.
* `-M x` - create a map file showing the relative memory addresses of each exported symbol (variable or function); e.g., `-M mapfile.txt` will create a map file called `mapfile.txt`. For more detailed information, the utility `sortmap.exe` in SCC's `bin` subfolder can be used to sort this map and show the amount of memory used by each symbol; e.g., `sortmap mapfile.txt`.
* `-o x` - set the output filename of the SymbOS executable to `x`; e.g., `-o file.com` would name the output file `file.com`.
* `-Ox` - set the optimization level, where `x` is one of `0`, `1`, `2`, or `s`. This controls whether various short operations will be inlined (for speed) or delegated to subroutines (for size). Not all optimization levels are currently stable, so only `-O1` (the default) or `-Os` (optimize for size) are currently recommended.
* `-s` - build standalone, without including libc or system libraries. For experts only.
* `-S` - compile to Z80 assembly source only (`.s` files); do not assemble.
* `-T x` - set the assembler name of the **code** segment to `x`. For experts only.
* `-V` - verbosely print the full command being run for each compiler pass.
* `-X` - do not erase temporary files (for example, generated Z80 assembly files). Mainly useful for low-level debugging.

## Interfacing with assembly

SCC does not currently support inline assembly. However, Z80 assembly files can be passed as arguments to `cc` to be linked into the main executable. For example:

```bash
cc cfile.c asmfile.s
```

The usual system of `.export` and `extern` applies for sharing symbols between assembly and C objects. Assembly files must export any shared symbols with the `.export` directive:

```
.code                ; emit to code segment
.export _asmadd      ; export symbol _asmadd
_asmadd:
	pop de           ; pop return address
	pop hl           ; pop first argument (8-bit, so in L)
	ld a,(_asmbuf+0)
	add l
	ld l,a           ; 8-bit return value goes in L
	push hl          ; restore stack (caller cleans up)
	push de          ; restore return address
	ret

.symtrans            ; emit to transfer segment
.export _asmbuf      ; export symbol _asmbuf
_asmbuf:
    .ds 256          ; 256 bytes of 0x00
```

The exported symbols can then be declared in C with the `extern` keyword:

```c
extern char asmbuf[256];
extern char asmadd(char v);

int main(int argc, char* argv[]) {
    asmbuf[0] = 1;
    asmbuf[1] = asmadd(2); // asmbuf[1] will contain 2 + 1 = 3
}
```

The assembled version of shared symbols is assumed to start with an underscore, so the C symbol `main` assembles to `_main` and the assembly symbol `_asmfunc` is referenced in C as `asmfunc`. If a symbol is not exported, it remains local to its own assembly file.

SCC uses an approximately cdecl calling convention: all arguments are passed on the stack, right to left, before the function is called with `call`. On function entry, the top word of the stack will be the return address, followed by the leftmost argument, the next leftmost argument, and so on. The caller cleans up the stack, so the stack should be returned to this state (at least in quantity, if not in content) before calling `ret`. 8-bit values are passed as the low byte of the 16-bit value pushed to stack. 32-bit values are passed as two successive 16-bit values on the stack. 8-bit values are returned in the L register. 16-bit values are returned in the HL register pair. 32-bit values are returned with the low word in HL and the high word in `(__hireg)`, a globally defined symbol in `libz80.a` that will be linked into all SymbOS executables.

**Note that assembly routines should preserve the values of the IX, IY, and (ideally) BC registers on entry/exit.** SCC uses IX and/or IY to track the local variables of the calling function, and may use BC as a `register` variable. (If no `register` variables are used, it is safe to destroy BC.)

`as` supports standard Z80 opcodes, with notable directives including:

* `.code`: output subsequent code to the SymbOS **code** segment
* `.symdata`: output subsequent code to the SymbOS **data** segment
* `.symtrans`: output subsequent code to the SymbOS **transfer** segment
* `.abs`: output subsequent code at the absolute address specified by `.org` (this may not link correctly into a SymbOS executable)
* `.org ____`: set absolute address of subsequent code to `____`. **Must be preceded by `.abs`!**
* `.export ____`: export symbol `____` for linking
* `.byte ____`: emit the raw byte `____` (also `.db` or `db` or `defb`). Multiple values can be separated by commas.
* `.word ____`: emit the raw 2-byte word `____` (also `.dw` or `dw` or `defw`). Multiple values can be separated by commas.
* `.ds ____`: emit `____` bytes filled with 0x00 (also `ds` or `defs` or `.blkb`)
* `.ascii "____"`: emit the text string `"____"` as raw ASCII text (also `defm`). Single quotes `'` can also be used to delimit the string if the string includes double quotes.

In assembly generated during compilation, you may see additional segment directives like `.data` and `.bss` (the internal "data" and "bss" segments, actually linked as part of the SymbOS **code** segment) or `.literal` (the internal "literal" segment, actually linked as part of the SymbOS **data** segment). **Do not confuse `.data` with `.symdata`!** To put data in the SymbOS **data** segment, use `.symdata`, not `.data.`.

Note that `as` uses assembly syntax similar to---but simpler and not 100% identical with---the Maxam-style syntax supported by the WinApe assembler (the most common assembler for SymbOS programming). If something isn't working as expected, try examining some of the assembler files in the SCC source repository to see what syntax they use. To see what assembly code `cc` is producing, run it with the `-X` option to preserve intermediate files.

## Some advice on workflow

Development will be much, **much** easier if you cross-compile on a PC and test on an emulator. Working natively on SymbOS sounds cool---and improving the native experience is a long-term goal---but for now, the slow compilation speeds and lack of a good IDE quickly get frustrating. Setting up an efficient cross-compilation workflow will dramatically improve your experience with SCC.

### Testing on an emulator

Some example workflows using [WinApe](http://www.winape.net/) on Windows:

1. Easy: Run SymbOS in WinApe (e.g., from a floppy or hard disk image). Insert a blank, formatted disk image into one of WinApe's virtual floppy drives, and keep the "Edit Disk" window open. Whenever you compile a new version of your app, just drag-and-drop it into the Edit Disk window and run it from SymbOS.

2. Advanced: Create a FAT16 or FAT32 partition on your hard drive and mount it as a hard drive within WinApe (Settings > Other > Logical Drive). Unpack the SymbOS installation packages to this partition and install the SymbOS CPC ROM images to slots Upper 1/2/3/4 in WinApe. You should now be able to run SymbOS directly off of your hard drive, with both Windows and SymbOS seeing the same files simultaneously. Just compile on Windows and run the resulting executable in-place on SymbOS!

Similar workflows are possible with other emulators, depending on setup. For developing [network](network.md) apps, note especially that [CPCEMU](https://www.cpc-emu.org/) can emulate the M4 Board's WiFi capabilities.

### Editors

SCC code tends to use a lot of SymbOS-specific system calls, so it's worth using a good modern IDE with code-completion---it will drastically reduce the number of times you have to look up syntax in the manual. ([Code::Blocks](https://www.codeblocks.org/) and Visual Studio Code are both popular.) In Code::Blocks, you should add SCC's `lib/include` directory to the search path to enable code-completion for SCC-specific functions (Settings > Compiler > Search directories > Compiler > Add), or just keep a copy of `symbos.h` open.

### Native compilation

If you insist on running SCC natively on SymbOS, a few tips to improve the experience:

**Run `cc` with the `-V` option.** (e.g.: `cc -V test.c`) This shows the commands used for each compilation stage as they are executed, giving some sense of progress. (Otherwise, `cc` will just appear to do nothing for multiple minutes---pretty boring.)

**Break large programs into small modules and only recompile the ones that have changed.** See [above](#multi-file-projects) for discussion of how to do this.

**Only include the minimum number of necessary header files in each module** (e.g., `symbos/shell.h` instead of the entire `symbos.h`). This may also be necessary to prevent SCC from running out of memory.

**Run SCC in an accelerated platform/emulator.** Compilation is much faster on SymbOSVM, although there are currently still bottlenecks relating to disk access that should be improved in future versions. Emulators like WinApe also allow ramping the CPU speed from 100% (Shift+F4) to 1000% (Shift+F5). (Whether this defeats the whole purpose of running SCC natively on SymbOS is, of course, a matter of taste.)

**Create batch files to automate the build.** SCC currently lacks a native "make" utility, but SymShell does support simple batch files. For example, we can create a file called `make.bat` in our project's folder that contains the commands necessary to build the app, one per line. Typing `make` in the project's folder will then run the commands in order from the batch file.
