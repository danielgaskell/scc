# Special considerations

## The `malloc()` heap

The libc function `malloc()` dynamically allocates a new block of memory for storing data. On a modern system, this is the usual way of requesting additional memory, and a lot of existing C code assumes that it can `malloc()` more memory indefinitely---often without even checking for out-of-memory errors. This is a reasonable assumption on modern platforms, but definitely not true on the Z80 (which has a limited 64KB address space), and even less true on SymbOS (where multiple multitasking applications may be packed into the same 64KB bank). In practice, a SymbOS executable must either 1) use [banked memory](syscalls.h#memory-management) system calls to access memory in other banks, or 2) declare upfront how much memory it needs in its main 64KB bank.

SCC currently implements an imperfect compromise, allocating a static 4KB heap for `malloc()` to expand into. This is sufficient for everyday usage (such as allocating `FILE*` records and other small data structures), but not for allocating large data buffers. If larger buffers are needed, we have three options:

* Rewrite the code to use static buffers.
* Rewrite the code to use [banked memory](syscalls.h#memory-management) system calls.
* Increase the static heap size with the `cc` command-line option `-h`:

```bash
cc -h 16384 source.c
```

## Native preprocessor

For desktop cross-compilation, SCC uses MCPP as its preprocessor, which should support all standard C preprocessor syntax. For improved speed when running natively on SymbOS, SCC uses a stripped-down preprocessor. The native processor supports the most common directives used in 99% of cases (`#include`, `#define`, `#undef`, `#ifdef`, `#ifndef`, `#else`, `#endif`), but not `#if (condition)` (which is much more complicated to implement, and only rarely used). Use combinations of `#ifdef` or `#ifndef` instead of `#if (condition)`. Function-style definitions (e.g., `#define add(x,y) ((x)+(y))`) are supported, but only with single-character argument names.

A more advanced (but slower) native preprocessor is available in the `bin/symbos/alternate` folder of the SCC source repository. This preprocessor is adapted from the Fuzix Compiler Kit, and can be substituted directly for the default native preprocessor if desired.

## Quirks of `stdio.h`

### File sizes

Due to a limitation of the filesystem, files stored on AMSDOS filesystems (e.g., CPC floppy disks) will often be terminated with an EOF character 0x1A and then some garbage padding (see [File Access](syscall2.md#file-access)). To improve compatibility, most `stdio.h` functions treat character 0x1A as EOF. If we need to read a binary file that includes legitimate 0x1A characters, the file should be opened in binary (`b`) mode, e.g.:

```c
f = fopen("data.dat", "rb");
```

...with the tradeoff being that we now need to pay attention to the fact that there may be garbage data at the end of the file. (This problem does not apply to the FAT filesystems used by most mass storage devices.)

### `fseek()`

`fseek()` and `lseek()` past the existing end of a file will generally follow the POSIX behavior of filling in the intervening space with zeros (to create a "sparse file"). However, for multiple internal reasons, this will occur at the time of the seek, rather than if/when the file is written to.

### `printf()`

Because of how SCC handles variable argument lists, SCC's implementation of `printf()` and its relatives (`vsprintf()`, etc.) are pickier than some others about the data types of passed arguments matching the data types indicated in the format string. In particular, 32-bit values should be cast to `(int)` before being printed with `%i`, and 8-bit and 16-bit values should be case to `(long)` before being printed with `%l`. (Since 8-bit values are passed internally as 16-bit values on the stack, it is not necessary to cast 16-bit values to 8-bit or vice versa.)

## Interfacing with assembly code

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

## Using `as` as a standalone assembler

`as` can be used in conjunction with `ld` as a standalone assembler (including natively on SymbOS). First, use `as` to assembly an assembly (`.s`) file to an object (`.o`) file:

```bash
as asmfile.s
```

Then, use `ld` to link the `.o` file into a binary. To output a raw binary file without `ld` trying to patch the start of the file (see below), we should run `ld` with the `-b` option:

```bash
ld asmfile.o -b -o asmfile.out
```

The `-o` option allows us to specify the output file name, in this case `asmfile.out`. Multiple `.o` files can be linked into a single binary by listing them in the command line (e.g., `ld asmfile1.o asmfile2.o`...), so we can split up large projects into multiple `.s` files, assemble them separately into object files, and link all the object files together. Symbols can be shared between source files using the `.export` directive, as described above.

The behavior without the `-b` option is slightly different. `ld` assumes that we want to arrange any defined segments (`.code`, `.symdata`, `.symtrans`) into a SymbOS executable and that the code at the start of the `.code` segment defines a valid SymbOS executable header. After linking, the appropriate bytes in the header will be updated with the actual segment lengths. If our assembly files do in fact define a valid SymbOS executable header, this allows us to conveniently define segments using the `.symdata`, etc. directives rather than manually tracking their locations with symbols.

## Building SCC

The current primary build target for SCC is Windows. Install MinGW and Python 3, ensure that their `bin` folders are in the system path, and then run the `make.bat` batch files found throughout the SCC source tree to compile the relevant parts of the codebase. (This really ought to transition to proper Makefiles, but whatever.)

## SCC vs. SDCC

SCC is being developed as a replacement for Nerlaska's SymbosMake SDK for SDCC. The original version of this SDK is now hard to find, but a patched version for SDCC 4.1.12+ can be found in the [CPvM source repository](https://github.com/danielgaskell/cpvm).

While an incomplete early effort (SymbosMake reports the version number 0.0.1), this SDK has served admirably, being used to develop major applications including CPvM, Zym, and Star Chart. The key advantage of SymbosMake is more efficient code generation. Since it is based on the SDCC cross-compiler, it is able to take advantage of SDCC's modern memory- and CPU-hungry optimization techniques and more efficient calling convention. SDCC also supports several helpful C features missing in SCC, most notably inline assembly and named struct initializers. Thus, for performance-critical applications, SymbosMake remains a viable tool.

However, SCC aims to improve on SymbosMake in several important ways:

* It's open-source, so bugs can actually be fixed
* A full build chain with object files and a linker, rather than single-file compilation
* A native stdio port, so `printf()` etc. can be used in SymShell
* More comprehensive headers for system calls
* More flexible segment management; it is much easier to place globals in the correct segment, large buffers are not forced into the **data** segment unnecessarily, and static initializers do not waste space by being forcibly duplicated in the **transfer** segment
* Static initializers can contain direct pointers to other data structures, rather than having to initialize this at runtime (very useful when defining windows and controls)
* Much faster compilation times
* Much more comprehensive documentation
* Faster floating-point library
